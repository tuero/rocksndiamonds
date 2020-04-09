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

# Numerical
import pandas as pd
from sklearn.model_selection import KFold
from sklearn.model_selection import ParameterGrid

# Pytorch
import torch
import torch.nn as nn

from data_handler import getDataLoader
from model import distnetConfigToStr, DisNetCNN
from train import trainingConfigToStr, train
from loss_functions import getNumberOfParameters


def kfoldValidation(model: nn.Module, features: torch.Tensor, observations: torch.Tensor,
                    training_config: dict, num_folds: int = 5) -> pd.DataFrame:
    """
    Runs K-fold validation on the given model and data

    Args:
        model (nn.Module): The model structure to train
        features (torch.tensor) : Tensor of features already preprocessed
        observations (torch.tensor) : Tensor of observations already preprocessed
        training_config (dict): Configuration for training
        num_folds (int): Number of folds to perform validation

    Returns:
        Dataframe of validation loss data
    """
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    kf = KFold(n_splits=num_folds, shuffle=True, random_state=0)

    loss_data = []
    for fold_count, (training_idx, test_idx) in enumerate(kf.split(features)):
        print('Fold {}'.format(fold_count + 1))

        # Split data
        train_features, test_features = features[training_idx, :], features[test_idx, :]
        train_observations, test_observations = observations[training_idx, :], observations[test_idx, :]

        # Create dataloaders
        batch_size = training_config['batch_size']
        train_loader = getDataLoader(train_features, train_observations, batch_size)
        test_loader = getDataLoader(test_features, test_observations, batch_size)

        # Train model
        df_run, trained_model = train(model, device, train_loader, test_loader, training_config)
        loss_data.append(df_run)

    return pd.concat(loss_data)


def gridSearch(model_base_config: dict, training_base_config: dict, param_grid: dict, features: torch.Tensor,
               observations: torch.Tensor, num_folds: int = 5, start_msg: str = ""):
    """
    Runs gridsearch on the input parameter grid and populating the base model and training config

    Args:
        model_base_config (dict): The base model configuration
        training_base_config (dict) : The base training configuration
        param_grid (dict) : A dictionary of parameters and their value lists to search over
        features (torch.Tensor): The preprocessed features
        observations (torch.Tensor): The preprocessed observations
        num_folds (int): Number of folds to perform validation
        start_msg (str): The 
        
    Returns:
        Dataframe of validation loss data
    """
    SEP = "------------------"
    # Run gridsearch
    print(start_msg)
    output_file = open("../logs/kfold_run_{}.txt".format(start_msg), 'w')
    output_file.write('{}\n'.format(start_msg))

    # Count number of params combinations
    total_combs = reduce(mul, [len(i) for i in param_grid.values()], 1)

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

        print(SEP)
        output_file.write('{}\n'.format(SEP))
        print('Step {:3} of {:3}\t{}'.format(i + 1, total_combs, start_msg))
        output_file.write('Step {:3} of {:3}\n'.format(i + 1, total_combs))
        print(distnetConfigToStr(current_model_config))
        output_file.write('{}\n'.format(distnetConfigToStr(current_model_config)))
        print(trainingConfigToStr(current_training_config))
        output_file.write('{}\n'.format(trainingConfigToStr(current_training_config)))
        output_file.flush()

        model = DisNetCNN(current_model_config, str(i + 1))

        # Run K-fold
        kfold_df_ = kfoldValidation(model, features, observations, current_training_config, num_folds)
        kfold_df_.to_pickle('../data_out/df_{}_{}.pkl'.format(start_msg, i + 1))

    output_file.close()
