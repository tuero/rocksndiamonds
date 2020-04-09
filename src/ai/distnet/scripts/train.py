"""
File: train.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Model training functionality
"""

# Library
import logging
from typing import Tuple

# Numerical
import numpy as np
import pandas as pd

# Pytorch
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader

# Module
from data_handler import DataMode, PreprocessMode
from model import weights_init
from loss_functions import LossFunctionTypes, getLossFunction


def getTrainingConfig(n_epochs: int = 100, batch_size: int = 32, loss_fn: LossFunctionTypes = LossFunctionTypes.MSE,
                      start_rate: float = 1e-3, end_rate: float = 1e-5, clip_gradient_norm: float = 1e-2,
                      split_ration: float = 0.8, data_mode: DataMode = DataMode.NOKEY_SOLUTIONS,
                      preprocess_mode: PreprocessMode = PreprocessMode.NORMALIZE, seed : int = -1) -> dict:
    """
    Get the config dictionary which defines the specs for training

    Args:
        n_epochs (int): The number of epochs to train for
        batch_size (int): The number of samples in each batch
        loss_fn (LossFunctionTypes): The loss function type to use
        start_rate (float): The initial learning rate
        end_rate (float): The end learning rate
        clip_gradient_norm (float): The size of gradient before clipping takes place
        split_ration (float): Percentage of data to use for training
        data_mode (DataMode): The data type to train on
        preprocess_mode (PreprocessMode): Type of preprocessing to perform
        seed (int): Seed to use, or -1 if no seed.

    Returns:
        Dictionary representing configuration for the training
    """
    return {
        'n_epochs'             : n_epochs,
        'batch_size'           : batch_size,
        'loss_fn'              : loss_fn,
        'start_rate'           : start_rate,
        'end_rate'             : end_rate,
        'clip_gradient_norm'   : clip_gradient_norm,
        'split_ration'         : split_ration,
        'data_mode'            : data_mode,
        'preprocess_mode'      : preprocess_mode,
        'seed'                 : seed
    }


def trainingConfigToStr(config: dict) -> str:
    """
    Convert a given training confiruation into a readable string

    Args:
        config (dict): A dictionary representing the configuration for the training

    Returns:
        A string representing the configuration settings
    """
    return "Training Config:\n" + \
           "\tNumber of Epochs: {}\n".format(config['n_epochs']) + \
           "\tBatch size: {}\n".format(config['batch_size']) + \
           "\tLoss function: {}\n".format(config['loss_fn'].__name__) + \
           "\tStart rate: {}\n".format(config['start_rate']) + \
           "\tEnd rate: {}\n".format(config['end_rate']) + \
           "\tGradient Clipping: {}\n".format(config['clip_gradient_norm']) + \
           "\tSplit ratio: {}\n".format(config['split_ration']) + \
           "\tData Mode: {}\n".format(config['data_mode'].name) + \
           "\tPreprocess Mode:{}\n".format(config['preprocess_mode'].name) + \
           "\tSeed: {}\n".format(config['seed'])


def getEmptyDataFrame() -> pd.DataFrame:
    """
    Gets an empty dataframe used to represent the training information
    """
    DF_COLUMNS = ['Epoch', 'Loss', 'Run Type', 'Model']
    return pd.DataFrame(columns=DF_COLUMNS)


# Optimizers
# - learning rate start 1e-3 to end 1e-5 [done]
# - gradient clipping [done]
# - tanh activation function
# - SGD [done]
# - batch normalization (as a layer)
# - L2 regularization of 1e-4
def train(model: nn.Module, device: torch.device, train_loader: DataLoader, validation_loader: DataLoader,
          training_config: dict = getTrainingConfig()) -> Tuple[pd.DataFrame, nn.Module]:
    """
    Train the model using the given training configuration
    Args:
        model: Model structure to train
        device: Where to send data/model for training
        train_loader (torch.DataLoader) : Dataloader for training data
        validation_loader (torch.DataLoader) : Dataloader for validation data
        training_config (dict): Configuration for training

    Returns:
        DataFrame of loss data during training with testing every epoch
    """
    logger = logging.getLogger()

    # Reset model and send model to device
    model.apply(weights_init)
    model.to(device)

    # Training config
    start_rate = training_config['start_rate']
    end_rate = training_config['end_rate']
    n_epochs = training_config['n_epochs']
    loss_fn = getLossFunction(training_config['loss_fn'])
    clip_gradient_norm = training_config['clip_gradient_norm']

    # Optimizer and scheduler
    optimizer = optim.SGD(model.parameters(), lr=start_rate, weight_decay=0.01)
    decay_rate = np.exp(np.log(end_rate / start_rate) / n_epochs)
    lr_scheduler = optim.lr_scheduler.ExponentialLR(optimizer=optimizer, gamma=decay_rate)

    # Output loss information during training
    df_losses = getEmptyDataFrame()

    for epoch in range(n_epochs):
        # Rest into training mode
        training_loss = 0.0
        model.train()

        for i, data in enumerate(train_loader):
            # get the inputs; data is a list of [features, runtime]
            inputs, rts = data
            inputs, rts = inputs.to(device), rts.to(device)

            # zero the parameter gradients
            optimizer.zero_grad()

            # Propogate input to model and calculate loss
            outputs = model(inputs)
            loss = loss_fn(outputs, rts)

            # Add l2 loss

            # Propogate loss backwards and step optimizer
            loss.backward()

            # Clip the gradients
            if clip_gradient_norm is not None:
                torch.nn.utils.clip_grad_norm_(model.parameters(), clip_gradient_norm)

            # Step optimizer
            optimizer.step()

            training_loss += loss.item()

        # Get validation loss
        validation_loss = 0.0
        model.eval()
        with torch.no_grad():
            for i, data in enumerate(validation_loader):
                inputs, rts = data
                inputs, rts = inputs.to(device), rts.to(device)
                outputs = model(inputs)
                loss = loss_fn(outputs, rts)
                validation_loss += loss.item()

        # Store training and validation loss
        train_row = (epoch, training_loss / len(train_loader), 'Train', model.toStr())
        valid_row = (epoch, validation_loss / len(validation_loader), 'Test', model.toStr())
        df_losses = df_losses.append(pd.Series(train_row, index=df_losses.columns), ignore_index=True)
        df_losses = df_losses.append(pd.Series(valid_row, index=df_losses.columns), ignore_index=True)
        if epoch % 10 == 9:
            output_msg = "Epoch: {:>4d}, Training Loss {:>18,.4f}, Validation Loss {:>18,.4f}"
            logger.info(output_msg.format(epoch + 1, float(train_row[1]), float(valid_row[1])))

        # Update learning rate
        lr_scheduler.step()

    return df_losses, model
