"""
File: loss_functions.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Implemented custom loss functions for log-likelihood
"""

# Library
import sys
from enum import IntEnum
import logging
from typing import Callable
import unittest
import random

# Numeric
import numpy as np
from scipy import stats

# Pytorch
import torch
import torch.nn as nn
# from torch.distributions.exponential import Exponential
from torch.distributions.log_normal import LogNormal


# Constants
EPSILON = 1e-5
# TWO_PI = 6.2831853
TWO_PI = 2 * np.pi
LOG_2PI = 1.837877066
LOG_2PI = np.log(TWO_PI)
SQRT_TWO = np.sqrt(2.0)
HALF = 0.5


# Loss function types
class LossFunctionTypes(IntEnum):
    MSE = 0
    EXPONENTIAL = 1
    INVERSE_GAUSSIAN = 2
    LOG_NORMAL = 3


def getNumberOfParameters(loss_function_type: LossFunctionTypes) -> int:
    """
    Get the number of distribution parameters for the given loss function

    Args:
        loss_function_type (LossFunctionTypes): The loss function distribution type

    Returns
        Number of parameters representing the output layer for the network
    """
    logger = logging.getLogger()
    if loss_function_type == LossFunctionTypes.MSE:
        return 1
    if loss_function_type == LossFunctionTypes.EXPONENTIAL:
        return 1
    if loss_function_type == LossFunctionTypes.INVERSE_GAUSSIAN:
        return 2
    if loss_function_type == LossFunctionTypes.LOG_NORMAL:
        return 2

    # Othewise, we have an unknown loss function
    logger.error("Unknown loss function: {}".format(loss_function_type))
    sys.exit()


def getLossFunction(loss_function_type: LossFunctionTypes) -> Callable:
    """
    Get the callable function which will be used as the loss function during training

    Args:
        loss_function_type (LossFunctionTypes): The loss function distribution type

    Returns
        The callable function for the loss
    """
    logger = logging.getLogger()
    if loss_function_type == LossFunctionTypes.MSE:
        return nn.MSELoss()
    if loss_function_type == LossFunctionTypes.EXPONENTIAL:
        return expo_loss
    if loss_function_type == LossFunctionTypes.INVERSE_GAUSSIAN:
        return lognormal_loss
    if loss_function_type == LossFunctionTypes.LOG_NORMAL:
        return invgauss_loss

    # Othewise, we have an unknown loss function
    logger.error("Unknown loss function: {}".format(loss_function_type))
    sys.exit()


def expo_loss(prediction: torch.Tensor, target: torch.Tensor) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Exponential distribution
    for the given sample

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        target (torch.Tensor): The target as an input to the distribution

    Returns:
        A single dim tensor representing the mean log-likelihood of the target
    """
    scale = prediction + EPSILON
    scale = 1.0 / scale

    log_scale = torch.log(scale)
    ll = log_scale - (scale * target)

    return torch.mean(-ll)


def lognormal_loss(prediction: torch.Tensor, observation: torch.Tensor) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Lognormal distribution
    for the given sample

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        observation (torch.Tensor): The observation as an input to the distribution

    Returns:
        A single dim tensor representing the mean log-likelihood of the observation
    """
    mu = prediction[:, 0:1] + EPSILON
    sigma_squared = prediction[:, 1:2] + EPSILON
    target = observation[:, 0:1] + EPSILON

    log_mu = torch.log(mu)
    log_sigma_squared = torch.log(sigma_squared)
    log_target = torch.log(target)

    help1 = ((log_target - log_mu)**2) / (2 * sigma_squared)
    lh = -log_target - (HALF * log_sigma_squared) - (HALF * LOG_2PI) - help1

    # erf_helper = log_target - 

    return torch.mean(-lh)


def lognormal_scipy_loss(prediction: torch.Tensor, observation: torch.Tensor) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Lognormal distribution
    for the given sample

    Note:
        Calculates the log-likelihood using the scipy distribution

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        observation (torch.Tensor): The observation as an input to the distribution

    Returns:
        A single dim tensor representing the mean log-likelihood of the observation
    """
    with torch.no_grad():
        mu = prediction[:, 0] + EPSILON
        sigma_squared = prediction[:, 1] + EPSILON
        target = observation[:, 0] + EPSILON

        nll = []
        for i, (m, s, t) in enumerate(zip(mu, sigma_squared, target)):
            nll.append(-(stats.lognorm.logpdf(t.item(), np.sqrt(s.item()), loc=0, scale=m.item())))

    return torch.mean(torch.tensor([nll], dtype=torch.float32, requires_grad=True))


def lognormal_torch_loss(prediction: torch.Tensor, observation: torch.Tensor) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Lognormal distribution
    for the given sample

    Note:
        Calculates the log-likelihood using the builtin torch distribution function

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        observation (torch.Tensor): The observation as an input to the distribution

    Returns:
        A single dim tensor representing the mean log-likelihood of the observation
    """
    mu = prediction[:, 0] + EPSILON
    sigma_squared = prediction[:, 1] + EPSILON
    target = observation[:, 0] + EPSILON
    sol_flag = observation[:, 1]

    nll = torch.zeros([prediction.shape[0]], dtype=torch.float32)
    for i, (m, s, t) in enumerate(zip(mu, sigma_squared, target)):
        pdf = LogNormal(torch.log(m), torch.sqrt(s))
        nll[i] = -pdf.log_prob(t) if sol_flag[i] else -torch.log((1.0 - pdf.cdf(t)))

    return torch.mean(nll)


def invgauss_loss(prediction: torch.Tensor, target: torch.Tensor) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Inverse Gaussian distribution
    for the given sample

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        target (torch.Tensor): The target as an input to the distribution

    Returns:
        A single dim tensor representing the mean log-likelihood of the target
    """
    mu = prediction[:, 0] + EPSILON
    lambda_ = prediction[:, 1] + EPSILON
    target = target[:, 0] + EPSILON

    # help1 = lambda_ * ((target - mu)**2) / (2 * lambda_ * (mu**2))
    help1 = lambda_ * ((target - mu)**2) / (2 * target * (mu**2))
    lh = (HALF * torch.log(lambda_)) - (HALF * torch.log(TWO_PI * (target**3))) - help1

    return torch.mean(-lh)


def invgauss_scipy_loss(prediction: torch.Tensor, target: torch.Tensor) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Inverse Gaussian distribution
    for the given sample

    Note:
        Calculates the log-likelihood using the scipy distribution

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        target (torch.Tensor): The target as an input to the distribution

    Returns:
        A single dim tensor representing the mean log-likelihood of the target
    """
    with torch.no_grad():
        mu = prediction[:, 0] + EPSILON
        lambda_ = prediction[:, 1] + EPSILON
        target = target[:, 0] + EPSILON

        nll = []
        for i, (m, s, t) in enumerate(zip(mu, lambda_, target)):
            nll.append(-(stats.invgauss.logpdf(t.item(), m.item() / lambda_, loc=0, scale=s.item())))

    return torch.mean(torch.tensor([nll], dtype=torch.float32, requires_grad=True))


class TestLossFunctions(unittest.TestCase):

    def testLognormalLoss(self):
        DELTA = 1e-5
        for _ in range(100):
            mu = random.uniform(2.0, 3.0)
            theta = random.uniform(1.0, 2.0)
            target = random.uniform(0.0, 20.0)
            l1 = lognormal_loss(torch.Tensor([[mu, theta]]), torch.Tensor([[target]])).data.cpu().numpy()
            l2 = lognormal_scipy_loss(torch.Tensor([[mu, theta]]), torch.Tensor([[target]])).data.cpu().numpy()
            l3 = lognormal_torch_loss(torch.Tensor([[mu, theta]]), torch.Tensor([[target]])).data.cpu().numpy()
            self.assertTrue(abs(l1 - l2) < DELTA)
            self.assertTrue(abs(l1 - l3) < DELTA)
            self.assertTrue(abs(l2 - l3) < DELTA)

    def testInverseGaussianLoss(self):
        DELTA = 1e-5
        for _ in range(100):
            mu = random.uniform(2.0, 3.0)
            lambda_ = random.uniform(1.0, 2.0)
            target = random.uniform(0.0, 20.0)
            l1 = invgauss_loss(torch.Tensor([[mu, lambda_]]), torch.Tensor([[target]])).data.cpu().numpy()
            l2 = invgauss_scipy_loss(torch.Tensor([[mu, lambda_]]), torch.Tensor([[target]])).data.cpu().numpy()
            self.assertTrue(abs(l1 - l2) < DELTA)


if __name__ == "__main__":
    unittest.main()
