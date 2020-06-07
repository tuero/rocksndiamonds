"""
File: model_assessment.py
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
from torch.utils.data import Dataset

# Module
from data.data_handler import getTrainTestSubsetSampler, getDataLoader, divideAndClipData
from models.model_util import FilterMode
from train import train, validate_model
from export.export import exportModel


def getParamGrid(mode: FilterMode) -> dict:
    """
    Get the grid of parameters to search over for the different types of models

    Args:
        mode (FilterMode): The type of filter mode being used (determines number of filters to try in convolutional layers)

    Returns:
        Dictionary of value lists to gridsearch over.
    """

    if mode == FilterMode.DOUBLE:
        param_grid = {'start_rate' : [1e-1, 1e-2, 1e-3],
                      'n_convdepth' : [8, 16, 32],
                      'drop_value' : [0.15, 0.25, 0.35],
                      'clip_gradient_norm' : [1e-1, 1e-2]
                      }
    elif mode == FilterMode.CONSTANT:
        param_grid = {'start_rate' : [1e-1, 1e-2, 1e-3],
                      'n_convdepth' : [8, 16, 32],
                      'drop_value' : [0.15, 0.25, 0.35],
                      'clip_gradient_norm' : [1e-1, 1e-2]
                      }
    elif mode == FilterMode.HALF:
        param_grid = {'start_rate' : [1e-1, 1e-2, 1e-3],
                      'n_convdepth' : [64, 32, 16],
                      'drop_value' : [0.15, 0.25, 0.35],
                      'clip_gradient_norm' : [1e-1, 1e-2]
                      }

    return param_grid


def kfoldValidation(net_type: str, dataset: Dataset, training_config: dict, model_config: dict,
                    num_folds: int = 5, data_divider: int = 1, max_samples: int = 0, save_name: str = None) -> pd.DataFrame:
    """
    Runs K-fold validation on the given model and data

    Args:
        net_type (str): Model string
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

    # Kfold testing
    loss_data = []
    # total_testloss, total_ks, total_t = (0.0, 0.0, 0.0)
    all_levels = dataset.getLevelSamples()
    for fold_count, (train_lvls, test_lvls) in enumerate(kf.split(all_levels)):
        training_idx = dataset.getSamplesFromLevelIndices(train_lvls)
        test_idx = dataset.getSamplesFromLevelIndices(test_lvls, True)

        # Split data
        train_indices = divideAndClipData(training_idx, data_divider, max_samples)
        test_indices = divideAndClipData(test_idx)
        train_sampler, test_sampler = getTrainTestSubsetSampler(train_indices, test_indices)

        # Log current fold
        logger.info('Fold {} of {}'.format(fold_count + 1, num_folds))
        logger.info('Number of training samples: {}'.format(len(train_indices)))
        logger.info('Number of testing samples: {}'.format(len(test_indices)))

        # Create dataloaders
        batch_size = training_config['batch_size']
        train_loader = getDataLoader(dataset, batch_size, train_sampler)
        test_loader = getDataLoader(dataset, batch_size, test_sampler)

        # Train model
        # val_len = len(test_loader) * training_config['batch_size']
        # total_len = len(train_loader) * training_config['batch_size']
        df_run, trained_model = train(net_type, device, train_loader, test_loader, training_config, model_config)
        loss_data.append(df_run)
        # avg_testloss, avg_ks, avg_t, _, _ = validate_model(trained_model, device, test_loader, training_config, total_len)
        # total_testloss += avg_testloss
        # total_ks += avg_ks
        # total_t += avg_t

        # If we going to validate by using model in engine, need to save model
        # along with the level split being used (so we can test on levels not trained on)
        if save_name is not None:
            exportModel(trained_model, dataset, test_lvls, '{}_{}'.format(save_name, fold_count + 1))

    # return pd.concat(loss_data), total_testloss / num_folds, total_ks / num_folds, total_t / num_folds
    return pd.concat(loss_data)


def gridSearch(net_type: str, model_base_config: dict, training_base_config: dict, param_grid: dict, dataset: Dataset,
               num_folds: int = 5, start_msg: str = ""):
    """
    Runs gridsearch on the input parameter grid and populating the base model and training config

    Args:
        net_type (str): String net name
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

        current_training_config['end_rate'] = current_training_config['start_rate'] / 100

        logger.info(SEP)
        logger.info('Step {:3} of {:3}\t{}'.format(i + 1, total_combs, start_msg))

        # Run K-fold
        kfold_df_, _, _, _ = kfoldValidation(net_type, dataset, current_training_config, current_model_config, num_folds)
        kfold_df_ = kfoldValidation(net_type, dataset, current_training_config, current_model_config, num_folds)
        kfold_df_.to_pickle('../gridsearch_dfs/df_{}_{}.pkl'.format(start_msg, i + 1))
