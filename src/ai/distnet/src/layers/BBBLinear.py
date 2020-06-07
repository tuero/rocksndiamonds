"""
File: BBBLinear.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Bayesian CNN Linear layer Implementation
Source: Taken from https://github.com/kumar-shridhar/PyTorch-BayesianCNN
"""

import torch
import torch.nn.functional as F
from torch.nn import Parameter

from metrics import calculate_kl as KL_DIV
from layers.misc import ModuleWrapper

import sys
sys.path.append("..")


class BBBLinear(ModuleWrapper):
    
    def __init__(self, in_features, out_features, bias=True, name='BBBLinear'):
        super(BBBLinear, self).__init__()
        self.in_features = in_features
        self.out_features = out_features
        self.use_bias = bias
        self.device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
        self.name = name

        self.prior_mu = 0.0
        self.prior_sigma = 0.1

        self.W_mu = Parameter(torch.Tensor(out_features, in_features))
        self.W_rho = Parameter(torch.Tensor(out_features, in_features))

        self.bias_mu = Parameter(torch.Tensor(out_features))
        self.bias_rho = Parameter(torch.Tensor(out_features))
        # if self.use_bias:
        #     self.bias_mu = Parameter(torch.Tensor(out_features))
        #     self.bias_rho = Parameter(torch.Tensor(out_features))
        # else:
        #     self.register_parameter('bias_mu', None)
        #     self.register_parameter('bias_rho', None)

        self.reset_parameters()

    def reset_parameters(self):
        self.W_mu.data.normal_(0.0, 0.1)
        self.W_rho.data.normal_(-3, 0.1)

        self.bias_mu.data.normal_(0.0, 0.1)
        self.bias_rho.data.normal_(-3.0, 0.1)
        # if self.use_bias:
        #     self.bias_mu.data.normal_(0, 0.1)
        #     self.bias_rho.data.normal_(-3, 0.1)

    def forward(self, x):

        self.W_sigma = torch.log1p(torch.exp(self.W_rho))

        self.bias_sigma = torch.log1p(torch.exp(self.bias_rho))
        bias_var = self.bias_sigma ** 2
        # if self.use_bias:
        #     self.bias_sigma = torch.log1p(torch.exp(self.bias_rho))
        #     bias_var = self.bias_sigma ** 2
        # else:
        #     self.bias_sigma = bias_var = None

        act_mu = F.linear(x, self.W_mu, self.bias_mu)
        act_var = 1e-16 + F.linear(x ** 2, self.W_sigma ** 2, bias_var)
        act_std = torch.sqrt(act_var)

        eps = torch.empty(act_mu.size()).normal_(0.0, 1.0).to(self.device)
        return act_mu + act_std * eps
        # if self.training or sample:
        #     eps = torch.empty(act_mu.size()).normal_(0, 1).to(self.device)
        #     return act_mu + act_std * eps
        # else:
        #     return act_mu

    def kl_loss(self):
        kl = KL_DIV(self.prior_mu, self.prior_sigma, self.W_mu, self.W_sigma)

        kl += KL_DIV(self.prior_mu, self.prior_sigma, self.bias_mu, self.bias_sigma)
        # if self.use_bias:
        #     kl += KL_DIV(self.prior_mu, self.prior_sigma, self.bias_mu, self.bias_sigma)
        return kl
