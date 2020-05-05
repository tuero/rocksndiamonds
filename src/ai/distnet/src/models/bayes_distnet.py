"""
File: bayes_distnet.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Bayesian version Distnet CNN model definition
"""

# PyTorch
import torch.nn as nn

from layers.misc import ModuleWrapper, FlattenLayer
from layers.BBBConv import BBBConv2d
from layers.BBBLinear import BBBLinear
from models.model_util import getChannelTransform
from config.config_handler import getModelConfig


class BayesDistNetCNN(ModuleWrapper):
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

    def __init__(self, config: dict = getModelConfig(), name_suffix: str = ""):
        super().__init__()

        self.name_suffix = name_suffix

        # Model config
        self.n_convdepth       = config['n_convdepth']
        self.kernel_size       = config['kernel_size']
        self.n_fcdepth         = config['n_fcdepth']
        self.output_size       = 1
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
            BBBConv2d(in_channel, out_channel, kernel_size=self.kernel_size, alpha_shape=(1, 1), stride=1, padding=1, name='conv1'),
            # nn.ReLU(),
            nn.Softplus(),
            nn.AvgPool2d(kernel_size=2, stride=2),
            nn.BatchNorm2d(out_channel)
        )
        current_dim = current_dim - self.kernel_size + (2 * 1) + 1
        current_dim = int((current_dim - 0) / 2)

        # Layer 2: Convolutional, ReLU Activation, with Batch Normalization
        in_channel = out_channel
        out_channel = getChannelTransform(out_channel, self.filter_mode)
        self.layer2 = nn.Sequential(
            BBBConv2d(in_channel, out_channel, kernel_size=self.kernel_size, alpha_shape=(1, 1), stride=1, padding=1, name='conv2'),
            # nn.ReLU(),
            nn.Softplus(),
            # nn.AvgPool2d(kernel_size=2, stride=2),
            nn.BatchNorm2d(out_channel)
        )
        current_dim = current_dim - self.kernel_size + (2 * 1) + 1
        # current_dim = int((current_dim - 0) / 2)

        # Layer 3: Convolutional, ReLU Activation, with Batch Normalization
        in_channel = out_channel
        out_channel = getChannelTransform(out_channel, self.filter_mode)
        ks_last = 1 if self.single_out_filter else self.kernel_size
        padding_last = 0 if self.single_out_filter else 1
        self.layer3 = nn.Sequential(
            BBBConv2d(in_channel, out_channel, kernel_size=ks_last, stride=1, alpha_shape=(1, 1), padding=padding_last, name='conv3'),
            # nn.ReLU(),
            nn.Softplus(),
            nn.BatchNorm2d(out_channel)
        )
        current_dim = current_dim - ks_last + (2 * padding_last) + 1

        # ---------- FC Layers ----------
        # Layer 4: Previous convolutional layer to the fully connected layer, ReLU Activation, and Dropout
        in_channel = out_channel * current_dim * current_dim
        out_channel = self.n_fcdepth
        self.flatten = FlattenLayer(in_channel)
        self.layer4 = nn.Sequential(
            BBBLinear(in_channel, out_channel, alpha_shape=(1, 1), bias=False, name='fc1'),
            # nn.ReLU(),
            nn.Softplus()
        )

        # Layer 5: Fully connected, ReLU Activation, with Dropout
        in_channel = out_channel
        out_channel = out_channel // 2
        self.layer5 = nn.Sequential(
            BBBLinear(in_channel, out_channel, alpha_shape=(1, 1), bias=False, name='fc2'),
            # nn.ReLU(),
            nn.Softplus(),
        )

        # Layer 6: Final output from model
        in_channel = out_channel
        self.layer_end = nn.Sequential(
            BBBLinear(in_channel, self.output_size, alpha_shape=(1, 1), bias=False, name='fc3'),
            # nn.ReLU(),
            nn.Softplus()
            # nn.Linear()
        )

    # Helper function for debugging
    def toStr(self) -> str:
        return "Bayes_Distnet" + "_" + self.name_suffix
