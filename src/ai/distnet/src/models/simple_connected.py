"""
File: simple_connected.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Simple fully connected base model
"""

# Pytorch
import torch
import torch.nn as nn


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
            nn.Softplus(),
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
