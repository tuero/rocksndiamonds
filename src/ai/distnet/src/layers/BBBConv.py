"""
File: BBBConv.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Bayesian CNN Conv2D Implementation
Source: Taken from https://github.com/kumar-shridhar/PyTorch-BayesianCNN
"""

# Library
import math

# PyTorch
import torch
import torch.nn.functional as F
from torch.nn import Parameter

# Modules
import metrics
from layers.misc import ModuleWrapper
# import config_bayesian as cfg
# import utils


class BBBConv2d(ModuleWrapper):

    def __init__(self, in_channels, out_channels, kernel_size, alpha_shape, stride=1,
                 padding=0, dilation=1, bias=True, name='BBBConv2d'):
        super(BBBConv2d, self).__init__()
        self.in_channels = in_channels
        self.out_channels = out_channels
        self.kernel_size = (kernel_size, kernel_size)
        self.stride = stride
        self.padding = padding
        self.dilation = dilation
        self.alpha_shape = alpha_shape
        self.groups = 1
        self.weight = Parameter(torch.Tensor(out_channels, in_channels, *self.kernel_size))
        self.bias = Parameter(torch.Tensor(out_channels))
        # if bias:
        #     # self.bias = Parameter(torch.Tensor(1, out_channels, 1, 1))
        #     self.bias = Parameter(torch.Tensor(out_channels))
        # else:
        #     self.register_parameter('bias', None)
        # self.out_bias = lambda input, kernel: F.conv2d(input, kernel, self.bias, self.stride, self.padding, self.dilation, self.groups)
        # self.out_nobias = lambda input, kernel: F.conv2d(input, kernel, None, self.stride, self.padding, self.dilation, self.groups)
        self.log_alpha = Parameter(torch.Tensor(*alpha_shape))
        self.reset_parameters()
        self.name = name
        # self.device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
        # if cfg.record_mean_var:
        #     self.mean_var_path = cfg.mean_var_dir + f"{self.name}.txt"

    def reset_parameters(self):
        n = self.in_channels
        for k in self.kernel_size:
            n *= k
        stdv = 1. / math.sqrt(n)
        self.weight.data.uniform_(-stdv, stdv)
        self.bias.data.uniform_(-stdv, stdv)
        # if self.bias is not None:
        #     self.bias.data.uniform_(-stdv, stdv)
        self.log_alpha.data.fill_(-5.0)

    def forward(self, x):

        # mean = self.out_bias(x, self.weight)
        mean = F.conv2d(x, self.weight, self.bias, self.stride, self.padding, self.dilation, self.groups)

        sigma = torch.exp(self.log_alpha) * self.weight * self.weight
        
        # std = torch.sqrt(1e-16 + self.out_nobias(x * x, sigma))
        temp = F.conv2d(x * x, sigma, None, self.stride, self.padding, self.dilation, self.groups)
        std = torch.sqrt(1e-16 + temp)
        epsilon = std.new_empty(std.size()).normal_()
        # if self.training:
        #     epsilon = std.data.new(std.size()).normal_()
        # else:
        #     epsilon = 0.0

        # Local reparameterization trick
        out = mean + std * epsilon

        # if cfg.record_mean_var and cfg.record_now and self.training and self.name in cfg.record_layers:
        #     utils.save_array_to_file(mean.cpu().detach().numpy(), self.mean_var_path, "mean")
        #     utils.save_array_to_file(std.cpu().detach().numpy(), self.mean_var_path, "std")

        return out

    def kl_loss(self):
        return self.weight.nelement() / self.log_alpha.nelement() * metrics.calculate_kl(self.log_alpha)
