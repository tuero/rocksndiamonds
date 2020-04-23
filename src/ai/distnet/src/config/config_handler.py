"""
File: config_handler.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Handle model and training configurations
"""

# Library
import sys
import logging
import configparser
import traceback

# Module
from models.model_util import FilterMode
from data.data_handler import PreprocessMode
from loss_functions import LossFunctionTypes


def getTrainingConfig(n_epochs: int = 100, batch_size: int = 32, loss_fn: LossFunctionTypes = LossFunctionTypes.MSE,
                      start_rate: float = 1e-3, end_rate: float = 1e-5, clip_gradient_norm: float = 1e-2,
                      split_ration: float = 0.8, preprocess_mode: PreprocessMode = PreprocessMode.NORMALIZE,
                      seed : int = -1, n_ens: int = 20) -> dict:
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
        preprocess_mode (PreprocessMode): Type of preprocessing to perform
        seed (int): Seed to use, or -1 if no seed
        n_ens (int): Number of samples for variational inference approximation

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
        'preprocess_mode'      : preprocess_mode,
        'seed'                 : seed,
        'n_ens'                : n_ens
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
           "\tLoss function: {}\n".format(config['loss_fn'].name) + \
           "\tStart rate: {}\n".format(config['start_rate']) + \
           "\tEnd rate: {}\n".format(config['end_rate']) + \
           "\tGradient Clipping: {}\n".format(config['clip_gradient_norm']) + \
           "\tSplit ratio: {}\n".format(config['split_ration']) + \
           "\tPreprocess Mode: {}\n".format(config['preprocess_mode'].name) + \
           "\tSeed: {}\n".format(config['seed']) + \
           "\tSamples for Variational Inference: {}\n".format(config['n_ens'])


def getModelConfig(n_convdepth: int = 32, kernel_size: int = 3, n_fcdepth: int = 64,
                   output_size: int = 1, drop_value: float = 0.15,
                   filter_mode: FilterMode = FilterMode.CONSTANT, single_out_filter: bool = True) -> str:
    """
    Get the config dictionary which defines the specs of the modles

    Args:
        n_convdepth (int): The number of filters for the first convolutional layer
        kernel_size (int): The kernel size
        n_fcdepth (int): The number of nodes in the first fully connected layer
        output_size (int): The number of outputs nodes for the network
        drop_value (float): Probability for dropout events
        filter_mode (FilterMode): Progression of number of filters for CNN (constant, double, half)
        single_out_filter (bool): Flag for using single filter on last convolutional layer

    Returns:
        Dictionary representing configuration for the DistNet model
    """
    return {
        'n_convdepth'       : n_convdepth,
        'kernel_size'       : kernel_size,
        'n_fcdepth'         : n_fcdepth,
        'output_size'       : output_size,
        'drop_value'        : drop_value,
        'filter_mode'       : filter_mode,
        'single_out_filter' : single_out_filter
    }


def modelConfigToStr(config: dict) -> str:
    """
    Convert a given DistNet model confiruation into a readable string

    Args:
        config (dict): A dictionary representing the configuration for the DistNet model

    Returns:
        A string representing the configuration settings
    """
    return "Disnet Config:\n" + \
           "\tFirst conv size: {}\n".format(config['n_convdepth']) + \
           "\tkernel size: {}\n".format(config['kernel_size']) + \
           "\tfilter mode: {}\n".format(config['filter_mode'].name) + \
           "\tsingle output filter: {}\n".format(config['single_out_filter']) + \
           "\tFirst FC layer size: {}\n".format(config['n_fcdepth']) + \
           "\tDropout p = {}\n".format(config['drop_value']) + \
           "\tOutput size: {}".format(config['output_size'])


def parseConfig(config_section):
    # Get logger/config
    logger = logging.getLogger()
    config = configparser.ConfigParser()
    config.read('config/training_config.ini')

    if config_section not in config.sections() and config_section != "DEFAULT":
        logger.error("Training config {} section was not found. See training_config.ini".format(config_section))
        sys.exit()

    # Start with default configs
    training_config = getTrainingConfig()
    model_config = getModelConfig()

    # Get items in the config option and update our training/model config
    try:
        # Training config items
        if config.has_option(config_section, 'n_epochs'):
            training_config['n_epochs'] = int(config.get(config_section, 'n_epochs'))
        if config.has_option(config_section, 'batch_size'):
            training_config['batch_size'] = int(config.get(config_section, 'batch_size'))
        if config.has_option(config_section, 'loss_fn'):
            training_config['loss_fn'] = LossFunctionTypes[config.get(config_section, 'loss_fn')]
        if config.has_option(config_section, 'start_rate'):
            training_config['start_rate'] = float(config.get(config_section, 'start_rate'))
        if config.has_option(config_section, 'end_rate'):
            training_config['end_rate'] = float(config.get(config_section, 'end_rate'))
        if config.has_option(config_section, 'clip_gradient_norm'):
            training_config['clip_gradient_norm'] = float(config.get(config_section, 'clip_gradient_norm'))
        if config.has_option(config_section, 'split_ratio'):
            training_config['split_ratio'] = float(config.get(config_section, 'split_ratio'))
        if config.has_option(config_section, 'preprocess_mode'):
            training_config['preprocess_mode'] = PreprocessMode[config.get(config_section, 'preprocess_mode')]
        if config.has_option(config_section, 'seed'):
            training_config['seed'] = int(config.get(config_section, 'seed'))
        if config.has_option(config_section, 'n_ens'):
            training_config['n_ens'] = int(config.get(config_section, 'n_ens'))

        # Model config items
        if config.has_option(config_section, 'n_convdepth'):
            model_config['n_convdepth'] = int(config.get(config_section, 'n_convdepth'))
        if config.has_option(config_section, 'kernel_size'):
            model_config['kernel_size'] = int(config.get(config_section, 'kernel_size'))
        if config.has_option(config_section, 'n_fcdepth'):
            model_config['n_fcdepth'] = int(config.get(config_section, 'n_fcdepth'))
        if config.has_option(config_section, 'output_size'):
            model_config['output_size'] = int(config.get(config_section, 'output_size'))
        if config.has_option(config_section, 'drop_value'):
            model_config['drop_value'] = float(config.get(config_section, 'drop_value'))
        if config.has_option(config_section, 'filter_mode'):
            model_config['filter_mode'] = FilterMode[config.get(config_section, 'filter_mode')]
        if config.has_option(config_section, 'single_out_filter'):
            model_config['single_out_filter'] = bool(config.get(config_section, 'single_out_filter'))
    except Exception:
        logger.error("Unknown error parsing configuration.")
        logger.error(traceback.format_exc())
        sys.exit()

    return training_config, model_config
