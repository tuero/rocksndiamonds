"""
File: model_util.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Uility functions for model use
"""

# Library
from enum import IntEnum

# Pytorch
import torch
import torch.nn as nn


# Enum for describing progression of number of filters for CNN
class FilterMode(IntEnum):
    CONSTANT = 1
    HALF = 2
    DOUBLE = 3


def getModel(net_type: str, model_config: dict) -> nn.Module:
    """
    Get the model object

    Args:
        net_type (str): String name of model type
        model_config (dict): Dictionary representing model configuration

    Returns:
        Torch module object
    """
    from models.distnet import DistNetCNN
    from models.bayes_distnet import BayesDistNetCNN

    if net_type == 'distnet':
        return DistNetCNN(model_config)
    elif net_type == 'bayes_distnet':
        return BayesDistNetCNN(model_config)
    else:
        raise ValueError('Unknown net type.')


def weights_init(m: torch.nn.Module):
    """
    Initializes the weights for the given module using the Xavier Uniform method

    Args:
        m (torch.nn.Module): The module to initalize the weights
    """
    if isinstance(m, nn.Conv2d) or isinstance(m, nn.Linear):
        torch.nn.init.xavier_uniform_(m.weight, gain=nn.init.calculate_gain('relu'))
        m.bias.data.zero_()
        m.bias.data.fill_(0.01)


def getChannelTransform(n_filters: int, filter_mode: FilterMode) -> int:
    """
    Get the new number of channels based on filter mode

    Args:
        n_filters (int): Current number of filters
        filter_mode (FilterMode): Filter mode which dictates progression of filter count

    Returns:
        The new number of filters
    """
    multiplier = {FilterMode.CONSTANT : 1,
                  FilterMode.HALF : 0.5,
                  FilterMode.DOUBLE : 2
                  }
    return int(multiplier[filter_mode] * n_filters)


def count_parameters(model: nn.Module) -> int:
    """
    Counts the number of parameters for the given model

    Args:
        model (torch.nn.): The model to examine

    Returns:
        The count of trainable parameters for the model
    """
    return sum(p.numel() for p in model.parameters() if p.requires_grad)
