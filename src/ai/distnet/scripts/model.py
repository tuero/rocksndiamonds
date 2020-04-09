"""
File: model.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Base model definitions
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


def getDisnetConfig(n_convdepth: int = 32, kernel_size: int = 3, n_fcdepth: int = 64,
                    output_size: int = 1, drop_value: float = 0.15,
                    filter_mode: FilterMode = FilterMode.CONSTANT, single_out_filter: bool = True) -> str:
    """
    Get the config dictionary which defines the specs of the DistNet model

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


def distnetConfigToStr(config: dict) -> str:
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


class DisNetCNN(nn.Module):
    """
    Implementation of the modified DistNet model (Eggensperger et al, 2018)

    Rather than a fully connected network, we utilize a CNN structure
        - Inputs are a collection of 2-channel 'images'
        - Channel 1 is a representation of the game level
        - Channel 2 is a representation of the path taken over the given level
        - Output is the parameter for a given distribution
        - There are 3 convolutional layers, followed by 2 fully connected layers,
          to the number of outputs

    Goal of the network is to predict the runtime distribution of the given level and
    the path the agent plans to take. The choice of distribution is given as input
    by the configuration in the form of the loss function, being the log-likelihood.
    """

    def __init__(self, config: dict = getDisnetConfig(), name_suffix: str = ""):
        super().__init__()

        self.name_suffix = name_suffix

        # Model config
        self.n_convdepth       = config['n_convdepth']
        self.kernel_size       = config['kernel_size']
        self.n_fcdepth         = config['n_fcdepth']
        self.output_size       = config['output_size']
        self.drop_value        = config['drop_value']
        self.filter_mode       = config['filter_mode']
        self.single_out_filter = config['single_out_filter']

        n_input_channels  = 2
        current_dim       = 40      # Used to calculate dimension of final convolution layer -> fully connected layer

        # ---------- Conv Layers ----------
        # Layer 1: Convolutional, ReLU Activation, with Batch Normalization
        in_channel = n_input_channels
        out_channel = self.n_convdepth
        self.layer1 = nn.Sequential(
            nn.Conv2d(in_channel, out_channel, kernel_size=self.kernel_size, stride=1, padding=1),
            nn.ReLU(),
            nn.BatchNorm2d(out_channel)
        )
        current_dim = current_dim - self.kernel_size + (2 * 1) + 1

        # Layer 2: Convolutional, ReLU Activation, with Batch Normalization
        in_channel = out_channel
        out_channel = self.get_channel_transform(out_channel)
        self.layer2 = nn.Sequential(
            nn.Conv2d(in_channel, out_channel, kernel_size=self.kernel_size, stride=1, padding=1),
            nn.ReLU(),
            nn.BatchNorm2d(out_channel)
        )
        current_dim = current_dim - self.kernel_size + (2 * 1) + 1

        # Layer 3: Convolutional, ReLU Activation, with Batch Normalization
        in_channel = out_channel
        out_channel = self.get_channel_transform(out_channel)
        ks_last = 1 if self.single_out_filter else self.kernel_size
        padding_last = 0 if self.single_out_filter else 1
        self.layer3 = nn.Sequential(
            nn.Conv2d(in_channel, out_channel, kernel_size=ks_last, stride=1, padding=padding_last),
            nn.ReLU(),
            nn.BatchNorm2d(out_channel)
        )
        current_dim = current_dim - ks_last + (2 * padding_last) + 1

        # ---------- FC Layers ----------
        # Layer 4: Previous convolutional layer to the fully connected layer, ReLU Activation, and Dropout
        in_channel = out_channel * current_dim * current_dim
        out_channel = self.n_fcdepth
        self.layer4 = nn.Sequential(
            nn.Linear(in_channel, out_channel),
            nn.ReLU(),
            nn.Dropout2d(self.drop_value)
        )

        # Layer 5: Fully connected, ReLU Activation, with Dropout
        in_channel = out_channel
        out_channel = out_channel // 2
        self.layer5 = nn.Sequential(
            nn.Linear(in_channel, out_channel),
            nn.ReLU(),
            nn.Dropout2d(self.drop_value)
        )

        # Layer 6: Final output from model
        in_channel = out_channel
        self.layer_end = nn.Sequential(
            nn.Linear(in_channel, self.output_size),
            nn.ReLU()
        )

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        x = self.layer1(x)
        x = self.layer2(x)
        x = self.layer3(x)
        x = x.view(-1, self.num_flat_features(x))
        x = self.layer4(x)
        x = self.layer5(x)
        x = self.layer_end(x)
        return x

    # Helper function to get the next output number of convolution filters based on the configuration mode
    def get_channel_transform(self, n_channels: int) -> int:
        multiplier = {FilterMode.CONSTANT : 1,
                      FilterMode.HALF : 0.5,
                      FilterMode.DOUBLE : 2
                      }
        return int(multiplier[self.filter_mode] * n_channels)

    # Helper function to get the shape of the input when flattened
    def num_flat_features(self, x: torch.Tensor) -> int:
        size = x.size()[1:]  # all dimensions except the batch dimension
        num_features = 1
        for s in size:
            num_features *= s
        return num_features

    # Helper function for debugging
    def toStr(self) -> str:
        return "DisNet" + " " + self.name_suffix


class FCN(nn.Module):
    """
    Simple fully connected model to compare results against
    """

    def __init__(self, fc1_size: int = 64, fc2_size: int = 32, output_size: int = 1, drop_value: float = 0.0):
        super().__init__()

        n_input_channels  = 2
        current_dim       = 40      # Input is (40, 40) with 2 channels

        # Layer 1
        in_channel = n_input_channels * current_dim * current_dim
        out_channel = fc1_size
        self.layer1 = nn.Sequential(
            nn.Linear(in_channel, out_channel),
            nn.ReLU(),
            nn.Dropout2d(drop_value)
        )

        # Layer 2
        in_channel = out_channel
        out_channel = fc2_size
        self.layer2 = nn.Sequential(
            nn.Linear(in_channel, out_channel),
            nn.ReLU(),
            nn.Dropout2d(drop_value)
        )

        # Layer 3: Final output from model
        in_channel = out_channel
        out_channel = output_size
        self.layer_end = nn.Sequential(
            nn.Linear(in_channel, out_channel),
            nn.ReLU()
        )

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        x = x.view(-1, self.num_flat_features(x))
        x = self.layer1(x)
        x = self.layer2(x)
        x = self.layer_end(x)
        return x

    # Helper function to get the shape of the input when flattened
    def num_flat_features(self, x: torch.Tensor) -> int:
        size = x.size()[1:]  # all dimensions except the batch dimension
        num_features = 1
        for s in size:
            num_features *= s
        return num_features

    # Helper function for debugging
    def toStr(self) -> str:
        return "FCN"


def weights_init(m: torch.nn.Module):
    """
    Initializes the weights for the given module using the Xavier Uniform method

    Args:
        m (torch.nn.Module): The module to initalize the weights
    """
    if isinstance(m, nn.Conv2d) or isinstance(m, nn.Linear):
        torch.nn.init.xavier_uniform_(m.weight.data, gain=nn.init.calculate_gain('relu'))
        m.bias.data.zero_()


def count_parameters(model: nn.Module) -> int:
    """
    Counts the number of parameters for the given model

    Args:
        model (torch.nn.): The model to examine

    Returns:
        The count of trainable parameters for the model
    """
    return sum(p.numel() for p in model.parameters() if p.requires_grad)
