"""
File: validation.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Validation methods to compare models
"""

# Library
import copy
from functools import reduce
from operator import mul
import logging

# Numerical
import pandas as pd
from sklearn.model_selection import KFold
from sklearn.model_selection import ParameterGrid

# Pytorch
import torch
import torch.nn as nn
from torch.utils.data import Dataset

# import wandb

from data_handler import getTrainTestSubsetSampler, getDataLoader
from model import distnetConfigToStr, DisNetCNN, FilterMode
from train import trainingConfigToStr, train
from loss_functions import getNumberOfParameters, getLossFunction, runKSTest


def getParamGrid(mode: FilterMode) -> dict:
    """
    Get the grid of parameters to search over for the different types of models

    Args:
        mode (FilterMode): The type of filter mode being used (determines number of filters to try in convolutional layers)

    Returns:
        Dictionary of value lists to gridsearch over.
    """

    if mode == FilterMode.DOUBLE:
        param_grid = {'start_rate' : [1e-2, 1e-3],
                      'n_convdepth' : [8, 16, 32],
                      'clip_gradient_norm' : [1e-1, 1e-2, 1e-3]
                      }
    elif mode == FilterMode.CONSTANT:
        param_grid = {'start_rate' : [1e-2, 1e-3],
                      'n_convdepth' : [8, 16, 32, 64],
                      'clip_gradient_norm' : [1e-1, 1e-2, 1e-3]
                      }
    elif mode == FilterMode.HALF:
        param_grid = {'start_rate' : [1e-2, 1e-3],
                      'n_convdepth' : [64, 32, 16],
                      'clip_gradient_norm' : [1e-1, 1e-2, 1e-3]
                      }

    return param_grid


def kfoldValidation(model: nn.Module, dataset: Dataset, training_config: dict, model_config: dict, combined_config,
                    num_folds: int = 5, data_divider: int = 1, max_samples: int = -1) -> pd.DataFrame:
    """
    Runs K-fold validation on the given model and data

    Args:
        model (nn.Module): The model structure to train
        dataset (torch.tensor) : The complete dateset to use for training/testing
        training_config (dict): Configuration for training
        model_config (dict): Configuration for the model
        num_folds (int): Number of folds to perform validation
        data_divider (int): Number to divide training/test sets to compare # samples vs % lower bound data

    Returns:
        Dataframe of validation loss data
        Average loss between folds
        Percentage of test samples that fail the KS hypothesis under 0.01 confidence
    """
    logger = logging.getLogger()
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    kf = KFold(n_splits=num_folds, shuffle=True, random_state=0)

    loss_data = []
    dataset_indices = list(range(len(dataset)))
    ks_counter = 0.0
    ks_total = 0.0
    test_loss = 0.0
    loss_fn = getLossFunction(training_config['loss_fn'])

    for fold_count, (training_idx, test_idx) in enumerate(kf.split(dataset_indices)):
        # wandb.init(project="rnd_compare_lb_lognormal_kfold", config=combined_config, reinit=True)
        logger.info('Fold {}'.format(fold_count + 1))
        logger.info('Number of training samples: {}'.format(len(training_idx)))
        logger.info('Number of testing samples: {}'.format(len(test_idx)))

        # Split data
        ratiod_training_idx = training_idx[:int(len(training_idx) / data_divider)]
        ratiod_test_idx = training_idx[:int(len(test_idx) / data_divider)]
        if max_samples > 0 and len(ratiod_training_idx) > max_samples:
            ratiod_training_idx = ratiod_training_idx[:max_samples]
        train_sampler, test_sampler = getTrainTestSubsetSampler(ratiod_training_idx, ratiod_test_idx)

        # Create dataloaders
        batch_size = training_config['batch_size']
        train_loader = getDataLoader(dataset, batch_size, train_sampler)
        test_loader = getDataLoader(dataset, batch_size, test_sampler)

        logger.info(trainingConfigToStr(training_config))
        logger.info(distnetConfigToStr(model_config))

        # Train model
        df_run, trained_model = train(model, device, train_loader, test_loader, training_config)
        loss_data.append(df_run)

        model.eval()
        with torch.no_grad():
            for i, data in enumerate(test_loader):
                inputs, rts = data
                inputs, rts = inputs.to(device), rts.to(device)
                outputs = model(inputs)
                loss = loss_fn(outputs, rts, reduce=False)
                # test_loss += loss.item()
                test_loss += torch.sum(loss)

                for j in range(len(inputs)):
                    ks_total += 1
                    D, p = runKSTest(rts[j], training_config['loss_fn'], outputs[j])
                    if p < 0.01:
                        ks_counter += 1

    # wandb.join()
    return pd.concat(loss_data), test_loss / float(ks_total), ks_counter / float(ks_total) * 100


def gridSearch(model_base_config: dict, training_base_config: dict, param_grid: dict, dataset: Dataset,
               num_folds: int = 5, start_msg: str = ""):
    """
    Runs gridsearch on the input parameter grid and populating the base model and training config

    Args:
        model_base_config (dict): The base model configuration
        training_base_config (dict) : The base training configuration
        param_grid (dict) : A dictionary of parameters and their value lists to search over
        dataset (torch.tensor) : The complete dateset to use for training/testing
        num_folds (int): Number of folds to perform validation
        start_msg (str): The

    Returns:
        Dataframe of validation loss data
    """
    logger = logging.getLogger()
    SEP = "------------------"

    logger.info('{}\n'.format(start_msg))

    # Count number of params combinations
    total_combs = reduce(mul, [len(i) for i in param_grid.values()], 1)

    # Run gridsearch
    for i, params in enumerate(ParameterGrid(param_grid)):
        current_model_config = copy.deepcopy(model_base_config)
        current_training_config = copy.deepcopy(training_base_config)

        # Set current config
        for k, v in params.items():
            if k in current_model_config:
                current_model_config[k] = v
            else:
                current_training_config[k] = v

        output_size = getNumberOfParameters(current_training_config['loss_fn'])
        current_model_config['output_size'] = output_size
        current_training_config['end_rate'] = current_training_config['start_rate'] / 100

        logger.info(SEP)
        logger.info('Step {:3} of {:3}\t{}'.format(i + 1, total_combs, start_msg))

        model = DisNetCNN(current_model_config, str(i + 1))

        # Run K-fold
        kfold_df_, _, _ = kfoldValidation(model, dataset, current_training_config, current_model_config, num_folds)
        kfold_df_.to_pickle('../gridsearch_dfs/df_{}_{}.pkl'.format(start_msg, i + 1))
