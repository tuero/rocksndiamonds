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
TWO_PI = 2 * np.pi
LOG_2PI = 1.837877066
LOG_2PI = np.log(TWO_PI)
SQRT_TWO = np.sqrt(2.0)
HALF = 0.5
E = 2.71828182845904523536028


# Loss function types
class LossFunctionTypes(IntEnum):
    MSE = 0
    EXPONENTIAL = 1
    INVERSE_GAUSSIAN = 2
    LOG_NORMAL = 3
    NORMAL = 4


def runKSTest(target: float, loss_fn: LossFunctionTypes, dist_params: torch.Tensor):
    """
    Run the KS test for the given target, following the distribution

    Args:
        target (float): The observed target runtime
        loss_fn (LossFunctionTypes): The type of distribution (loss function) being used
        dist_params (torch.Tensor): Tensor of distribution parameters

    Returns
        Pair of KS D (distance) and p-value
    """
    param = dist_params.data.cpu().numpy()
    obs_time = target.data.cpu().numpy()[0]
    if loss_fn == LossFunctionTypes.LOG_NORMAL:
        ks = stats.kstest([obs_time], "lognorm", [np.sqrt(param[1] + EPSILON), 0, param[0] + EPSILON])
        return ks[0], ks[1]
    elif loss_fn == LossFunctionTypes.INVERSE_GAUSSIAN:
        ks = stats.kstest([obs_time], "invgauss", [(param[1] + EPSILON) / (param[0] + EPSILON), 0, param[0] + EPSILON])
        return ks[0], ks[1]
    elif loss_fn == LossFunctionTypes.NORMAL:
        ks = stats.kstest([obs_time], "norm", [(param[0] + EPSILON), np.sqrt(param[1]) + EPSILON])
        return ks[0], ks[1]
    elif loss_fn == LossFunctionTypes.EXPONENTIAL:
        ks = stats.kstest([obs_time], "expon", [0, 1.0 / (param[0] + EPSILON)])
        return ks[0], ks[1]


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
    if loss_function_type == LossFunctionTypes.NORMAL:
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
        return invgauss_loss
    if loss_function_type == LossFunctionTypes.LOG_NORMAL:
        return lognormal_loss
    if loss_function_type == LossFunctionTypes.NORMAL:
        return normal_loss

    # Othewise, we have an unknown loss function
    logger.error("Unknown loss function: {}".format(loss_function_type))
    sys.exit()


def _approx_erf(x : torch.Tensor) -> torch.Tensor:
    """
    Approximation function for the Gauss Error Function.
    This uses one of the approximations provided by Abramowitz and Stegun

    Args:
        x (torch.Tensor): The input to the distribution

    Returns:
        The value of the Gauss Error Function at the given input
    """
    p = 0.3275911
    a1 = 0.254829592
    a2 = -0.284496736
    a3 = 1.421413741
    a4 = -1.453152027
    a5 = 1.061405429

    # erf(x) = -erf(-x)
    neg_mask = x < 0
    torch.abs_(x)

    t = 1 / (1 + (p * x))
    polynomial = (a1 * t) + (a2 * (t**2)) + (a3 * (t**3)) + (a4 * (t**4)) + (a5 * (t**5))
    erf = 1 - (polynomial * torch.exp(- x**2))
    erf[neg_mask] *= -1
    return erf


def _standard_gaussian_cdf(x : torch.Tensor) -> torch.Tensor:
    """
    Approximation function for the Standard Guassian

    Args:
        x (torch.Tensor): The input to the distribution

    Returns:
        The value of the Standard Gaussian at the given input
    """
    return HALF * (1 + _approx_erf(x / SQRT_TWO))


def expo_loss(prediction: torch.Tensor, observation: torch.Tensor, reduce: bool = True) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Exponential distribution
    for the given sample

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        observation (torch.Tensor): The observation as an input to the distribution
        reduce (bool): Whether to reduce to mean or not

    Returns:
        A single dim tensor representing the mean log-likelihood of the target
    """
    scale = prediction[:, 0:1] + EPSILON
    scale = 1.0 / scale
    target = observation[:, 0:1] + EPSILON
    sol_flag = observation[:, 1] == 1

    target_1 = target.clone()[sol_flag]
    target_2 = target.clone()[~sol_flag]
    scale_1 = scale.clone()[sol_flag]
    scale_2 = scale.clone()[~sol_flag]
    log_scale_1 = torch.log(scale.clone()[sol_flag])

    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    llh = torch.zeros([prediction.shape[0]], dtype=torch.float32).to(device)

    # Observation seen i.e. use pointwise pdf
    llh[sol_flag] = torch.flatten(log_scale_1 - (scale_1 * target_1))

    # Upperbound i.e. use survival function = 1-CDF
    cdf = 1 - torch.exp(-scale_2 * target_2)
    llh[~sol_flag] = torch.flatten(torch.log(1 - cdf + EPSILON))

    return torch.mean(-llh) if reduce else -llh


def expo_scipy_loss(prediction: torch.Tensor, observation: torch.Tensor, reduce: bool = True) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Exponential distribution
    for the given sample

    Note:
        Calculates the log-likelihood using the scipy distribution

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        observation (torch.Tensor): The observation as an input to the distribution
        reduce (bool): Whether to reduce to mean or not

    Returns:
        A single dim tensor representing the mean log-likelihood of the target
    """
    scale = prediction[:, 0:1] + EPSILON
    scale = 1.0 / scale
    target = observation[:, 0:1] + EPSILON

    with torch.no_grad():
        scale = prediction[:, 0:1] + EPSILON
        scale = 1.0 / scale
        target = observation[:, 0] + EPSILON

        nll = []
        for i, (s, t) in enumerate(zip(scale, target)):
            nll.append((stats.expon.logpdf(t.item(), loc=0, scale=1.0 / s.item())))

    llh = torch.tensor([nll], dtype=torch.float32, requires_grad=True)
    return torch.mean(-llh) if reduce else -llh


def lognormal_loss(prediction: torch.Tensor, observation: torch.Tensor, reduce: bool = True) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Lognormal distribution
    for the given sample

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        observation (torch.Tensor): The observation as an input to the distribution
        reduce (bool): Whether to reduce to mean or not

    Returns:
        A single dim tensor representing the mean log-likelihood of the observation
    """
    mu = prediction[:, 0:1] + EPSILON
    sigma_squared = prediction[:, 1:2] + EPSILON
    target = observation[:, 0:1] + EPSILON
    sol_flag = observation[:, 1] == 1

    log_mu = torch.log(mu)
    log_sigma_squared = torch.log(sigma_squared)
    log_target = torch.log(target)

    log_target_1 = log_target.clone()[sol_flag]
    log_target_2 = log_target.clone()[~sol_flag]
    # log_mu_1 = log_mu.clone()[sol_flag]
    # log_mu_2 = log_mu.clone()[~sol_flag]
    log_mu_1 = mu.clone()[sol_flag]
    log_mu_2 = mu.clone()[~sol_flag]
    sigma_squared_1 = sigma_squared.clone()[sol_flag]
    sigma_squared_2 = sigma_squared.clone()[~sol_flag]
    log_sigma_squared_1 = log_sigma_squared.clone()[sol_flag]

    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    llh = torch.zeros([prediction.shape[0]], dtype=torch.float32).to(device)

    # Observation seen i.e. use pointwise pdf
    pdf_help1 = ((log_target_1 - log_mu_1)**2) / (2 * sigma_squared_1)
    # pdf_help1 = ((log_target[sol_flag] - mu[sol_flag])**2) / (2 * sigma_squared[sol_flag])
    llh[sol_flag] = torch.flatten((-log_target_1 - (HALF * log_sigma_squared_1) - (HALF * LOG_2PI) - pdf_help1))

    # Upperbound i.e. use survival function = 1-CDF
    cdf_help1 = (log_target_2 - log_mu_2) / (SQRT_TWO * torch.sqrt(sigma_squared_2))
    # cdf_help1 = (log_target[~sol_flag] - mu[~sol_flag]) / (SQRT_TWO * torch.sqrt(sigma_squared[~sol_flag]))
    cdf = HALF + (HALF * _approx_erf(cdf_help1))
    llh[~sol_flag] = torch.flatten(torch.log(1 - cdf + EPSILON))
    return torch.mean(-llh) if reduce else -llh


def lognormal_scipy_loss(prediction: torch.Tensor, observation: torch.Tensor, reduce: bool = True) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Lognormal distribution
    for the given sample

    Note:
        Calculates the log-likelihood using the scipy distribution

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        observation (torch.Tensor): The observation as an input to the distribution
        reduce (bool): Whether to reduce to mean or not

    Returns:
        A single dim tensor representing the mean log-likelihood of the observation
    """
    with torch.no_grad():
        mu = prediction[:, 0] + EPSILON
        sigma_squared = prediction[:, 1] + EPSILON
        target = observation[:, 0] + EPSILON

        nll = []
        for i, (m, s, t) in enumerate(zip(mu, sigma_squared, target)):
            # nll.append(-(stats.lognorm.logpdf(t.item(), np.sqrt(s.item()), loc=0, scale=m.item())))
            nll.append((stats.lognorm.logpdf(t.item(), np.sqrt(s.item()), loc=0, scale=np.exp(m.item()))))

    llh = torch.tensor([nll], dtype=torch.float32, requires_grad=True)
    return torch.mean(-llh) if reduce else -llh


def lognormal_torch_loss(prediction: torch.Tensor, observation: torch.Tensor, reduce: bool = True) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Lognormal distribution
    for the given sample

    Note:
        Calculates the log-likelihood using the builtin torch distribution function

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        observation (torch.Tensor): The observation as an input to the distribution
        reduce (bool): Whether to reduce to mean or not

    Returns:
        A single dim tensor representing the mean log-likelihood of the observation
    """
    mu = prediction[:, 0] + EPSILON
    sigma_squared = prediction[:, 1] + EPSILON
    target = observation[:, 0] + EPSILON
    sol_flag = observation[:, 1] == 1

    llh = torch.zeros([prediction.shape[0]], dtype=torch.float32, device='cuda:0')
    for i, (m, s, t) in enumerate(zip(mu, sigma_squared, target)):
        # pdf = LogNormal(torch.log(m), torch.sqrt(s))
        pdf = LogNormal(m, torch.sqrt(s))
        llh[i] = pdf.log_prob(t) if sol_flag[i] else torch.log((1.0 - pdf.cdf(t)))

    return torch.mean(-llh) if reduce else -llh


def _pdf_invgauss(x: torch.Tensor, mu: torch.Tensor, lambda_: torch.Tensor) -> torch.Tensor:
    helper = -(lambda_ * (x - mu)**2) / (2 * x * (mu**2))
    return (torch.sqrt(lambda_) / torch.sqrt(TWO_PI * (x**3))) * torch.exp(helper)


def invgauss_loss(prediction: torch.Tensor, observation: torch.Tensor, reduce: bool = True) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Inverse Gaussian distribution
    for the given sample

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        observation (torch.Tensor): The observation as an input to the distribution
        reduce (bool): Whether to reduce to mean or not

    Returns:
        A single dim tensor representing the mean log-likelihood of the target
    """
    mu = prediction[:, 0] + EPSILON
    lambda_ = prediction[:, 1] + EPSILON
    target = observation[:, 0] + EPSILON
    sol_flag = observation[:, 1] == 1

    lambda_1 = lambda_.clone()[sol_flag]
    lambda_2 = lambda_.clone()[~sol_flag].unsqueeze(1)
    mu_1 = mu.clone()[sol_flag]
    mu_2 = mu.clone()[~sol_flag].unsqueeze(1)
    target_1 = target.clone()[sol_flag]
    target_2 = target.clone()[~sol_flag]

    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    llh = torch.zeros([prediction.shape[0]], dtype=torch.float32).to(device)

    # Observation seen i.e. use pointwise pdf
    pdf_help1 = lambda_1 * ((target_1 - mu_1)**2) / (2 * target_1 * (mu_1**2))
    llh[sol_flag] = torch.flatten(((HALF * torch.log(lambda_1)) - (HALF * torch.log(TWO_PI * (target_1**3))) - pdf_help1))

    # Upperbound i.e. use survival function = 1-CDF
    STEPS = 100
    if target_2.nelement() != 0:
        xs = torch.stack([torch.arange(1, STEPS + 1) / float(STEPS) * i.item() for i in target_2]).to(device)
        pdfs = _pdf_invgauss(xs, mu_2, lambda_2)
        # CDF needs to be clamped within [0,1] or NaNs get propogated
        cdfs = torch.clamp(torch.clamp(torch.trapz(pdfs, xs), max=1), min=0)
        llh[~sol_flag] = torch.flatten(torch.log(1 - cdfs + EPSILON))

    return torch.mean(-llh) if reduce else -llh


def invgauss_scipy_loss(prediction: torch.Tensor, target: torch.Tensor, reduce: bool = True) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Inverse Gaussian distribution
    for the given sample

    Note:
        Calculates the log-likelihood using the scipy distribution

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        target (torch.Tensor): The target as an input to the distribution
        reduce (bool): Whether to reduce to mean or not

    Returns:
        A single dim tensor representing the mean log-likelihood of the target
    """
    with torch.no_grad():
        mu = prediction[:, 0] + EPSILON
        lambda_ = prediction[:, 1] + EPSILON
        target = target[:, 0] + EPSILON

        nll = []
        for i, (m, s, t) in enumerate(zip(mu, lambda_, target)):
            nll.append((stats.invgauss.logpdf(t.item(), m.item() / lambda_, loc=0, scale=s.item())))

    llh = torch.tensor([nll], dtype=torch.float32, requires_grad=True)
    return torch.mean(-llh) if reduce else -llh


def _pdf_normal(x: torch.Tensor, mu: torch.Tensor, sigma: torch.Tensor) -> torch.Tensor:
    helper = HALF * ((x - mu) / sigma)**2
    return 1.0 / (sigma * np.sqrt(TWO_PI)) * torch.exp(-helper)


def normal_loss(prediction: torch.Tensor, observation: torch.Tensor, reduce: bool = True) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Normal distribution
    for the given sample

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        observation (torch.Tensor): The observation as an input to the distribution
        reduce (bool): Whether to reduce to mean or not

    Returns:
        A single dim tensor representing the mean log-likelihood of the observation
    """
    mu = prediction[:, 0:1] + EPSILON
    sigma_squared = prediction[:, 1:2] + EPSILON
    target = observation[:, 0:1] + EPSILON
    sol_flag = observation[:, 1] == 1

    mu_1 = mu.clone()[sol_flag]
    mu_2 = mu.clone()[~sol_flag]
    target_1 = target.clone()[sol_flag]
    target_2 = target.clone()[~sol_flag]
    sigma_squared_1 = sigma_squared.clone()[sol_flag]
    sigma_squared_2 = sigma_squared.clone()[~sol_flag]
    log_sigma_squared = torch.log(sigma_squared)
    log_sigma_squared_1 = log_sigma_squared.clone()[sol_flag]

    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    llh = torch.zeros([prediction.shape[0]], dtype=torch.float32).to(device)

    # Observation seen i.e. use pointwise pdf
    pdf_help = HALF * (1 / sigma_squared_1) * ((target_1 - mu_1)**2)
    llh[sol_flag] = torch.flatten(-(HALF * LOG_2PI) - (HALF * log_sigma_squared_1) - pdf_help)

    # Upperbound i.e. use survival function = 1-CDF
    cdf = _standard_gaussian_cdf((target_2 - mu_2) / torch.sqrt(sigma_squared_2))
    llh[~sol_flag] = torch.flatten(torch.log(1 - cdf + EPSILON))

    return torch.mean(-llh) if reduce else -llh


def normal_scipy_loss(prediction: torch.Tensor, observation: torch.Tensor, reduce: bool = True) -> torch.Tensor:
    """
    Calculates the mean log-likelihood following the Normal distribution
    for the given sample

    Note:
        Calculates the log-likelihood using the scipy distribution

    Args:
        prediction (torch.Tensor): The predicted distribution parameters
        observation (torch.Tensor): The observation as an input to the distribution
        reduce (bool): Whether to reduce to mean or not

    Returns:
        A single dim tensor representing the mean log-likelihood of the observation
    """

    with torch.no_grad():
        mu = prediction[:, 0] + EPSILON
        sigma_squared = prediction[:, 1] + EPSILON
        target = observation[:, 0] + EPSILON

        nll = []
        for i, (m, s, t) in enumerate(zip(mu, sigma_squared, target)):
            nll.append((stats.norm.logpdf(t.item(), loc=m.item(), scale=np.sqrt(s.item()))))

    llh = torch.tensor([nll], dtype=torch.float32, requires_grad=True)
    return torch.mean(-llh) if reduce else -llh


class TestLossFunctions(unittest.TestCase):

    def testLognormalLoss(self):
        DELTA = 1e-3
        device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

        mus = torch.Tensor([[1.0, 1.0, 1.0, 0.75, 0.75, 0.75]])
        sigmas = torch.Tensor([[0.5, 0.5, 0.5, 3.0, 3.0, 3.0]])
        # params = torch.t(torch.cat((torch.exp(mus), sigmas**2), 0)).to(device)
        params = torch.t(torch.cat((mus, sigmas**2), 0)).to(device)
        targets = torch.Tensor([[1.0, 2.0, 8.0, 0.5, 10.0, 20.0]])
        mask = torch.Tensor([[1.0, 1.0, 1.0, 0.0, 0.0, 0.0]])
        observations = torch.t(torch.cat((targets, mask), 0)).to(device)
        nllh = torch.Tensor([2.22579073241, 1.10725452544, 4.6356211461, 0.378688327, 1.196024476, 1.48255859])

        results = lognormal_loss(params, observations, False)

        for n, r in zip(nllh, results):
            self.assertTrue(abs(n - r) < DELTA)

        # Check random parameters between the 3 various implementations
        for _ in range(100):
            mu = random.uniform(2.0, 3.0)
            theta = random.uniform(1.0, 2.0)
            target = random.uniform(0.0, 20.0)
            params = torch.Tensor([[mu, theta]]).to(device)
            observations = torch.Tensor([[target, 1]]).to(device)
            l1 = lognormal_loss(params, observations).data.cpu().numpy()
            l2 = lognormal_scipy_loss(params.cpu(), observations.cpu()).data.cpu().numpy()
            l3 = lognormal_torch_loss(params, observations).data.cpu().numpy()
            self.assertTrue(abs(l1 - l2) < DELTA)
            self.assertTrue(abs(l1 - l3) < DELTA)
            self.assertTrue(abs(l2 - l3) < DELTA)

    def testInverseGaussianLoss(self):
        DELTA = 1e-3
        device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

        mus = torch.Tensor([[1.0, 1.0, 1.0, 0.75, 0.75, 0.75]])
        lambdas = torch.Tensor([[0.5, 0.5, 0.5, 3.0, 3.0, 3.0]])
        params = torch.t(torch.cat((mus, lambdas), 0)).to(device)
        targets = torch.Tensor([[0.1, 0.3, 1, 0.3, 0.5, 1.5]])
        mask = torch.Tensor([[1.0, 1.0, 1.0, 0.0, 0.0, 0.0]])
        observations = torch.t(torch.cat((targets, mask), 0)).to(device)
        nllh = torch.Tensor([-0.163368068876, -0.1321153906, 1.2655113853, 0.0440765557, 0.31954294910, 3.085131954818])

        results = invgauss_loss(params, observations, False)

        for n, r in zip(nllh, results):
            self.assertTrue(abs(n - r) < DELTA)

        for _ in range(100):
            mu = random.uniform(2.0, 3.0)
            lambda_ = random.uniform(1.0, 2.0)
            target = random.uniform(0.0, 20.0)
            params = torch.Tensor([[mu, lambda_]]).to(device)
            observations = torch.Tensor([[target, 1]]).to(device)
            l1 = invgauss_loss(params, observations).data.cpu().numpy()
            l2 = invgauss_scipy_loss(params.cpu(), observations.cpu()).data.cpu().numpy()
            self.assertTrue(abs(l1 - l2) < DELTA)

    def testNormalLoss(self):
        DELTA = 1e-4
        device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

        mus = torch.Tensor([[1.0, 1.0, 1.0, 0.75, 0.75, 0.75]])
        sigma = torch.Tensor([[0.5, 0.5, 0.5, 3.0, 3.0, 3.0]])
        params = torch.t(torch.cat((mus, sigma**2), 0)).to(device)
        targets = torch.Tensor([[0.1, 0.3, 1, 0.3, 0.5, 1.5]])
        mask = torch.Tensor([[1.0, 1.0, 1.0, 0.0, 0.0, 0.0]])
        observations = torch.t(torch.cat((targets, mask), 0)).to(device)
        nllh = torch.Tensor([1.845793357720, 1.2057911231327, 0.22579080219, 0.58050087087, 0.628845562, 0.9130609532])

        results = normal_loss(params, observations, False)

        for n, r in zip(nllh, results):
            self.assertTrue(abs(n - r) < DELTA)

        for _ in range(100):
            mu = random.uniform(0.0, 3.0)
            sigma_squared = random.uniform(1.0, 2.0)
            target = random.uniform(0.0, 20.0)
            params = torch.Tensor([[mu, sigma_squared]]).to(device)
            observations = torch.Tensor([[target, 1]]).to(device)
            l1 = normal_loss(params, observations).data.cpu().numpy()
            l2 = normal_scipy_loss(params.cpu(), observations.cpu()).data.cpu().numpy()
            self.assertTrue(abs(l1 - l2) < DELTA)

    def testExponentialLoss(self):
        DELTA = 1e-3
        device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

        lambda_ = torch.Tensor([[1.0, 1.0, 1.0, 0.75, 0.75, 0.75]])
        # params = torch.t(torch.cat((lambda_), 0)).to(device)
        params = torch.t(1.0 / lambda_).to(device)
        targets = torch.Tensor([[0.1, 0.3, 1, 0.3, 0.5, 1.5]])
        mask = torch.Tensor([[1.0, 1.0, 1.0, 0.0, 0.0, 0.0]])
        observations = torch.t(torch.cat((targets, mask), 0)).to(device)
        nllh = torch.Tensor([0.100000462, 0.30000029, 1, 0.22500027, 0.375000405, 1.125001439])

        results = expo_loss(params, observations, False)

        for n, r in zip(nllh, results):
            self.assertTrue(abs(n - r) < DELTA)

        for _ in range(100):
            scale = random.uniform(0.0, 1.0)
            target = random.uniform(0.0, 20.0)
            params = torch.Tensor([[scale]]).to(device)
            observations = torch.Tensor([[target, 1]]).to(device)
            l1 = expo_loss(params, observations).data.cpu().numpy()
            l2 = expo_scipy_loss(params.cpu(), observations.cpu()).data.cpu().numpy()
            self.assertTrue(abs(l1 - l2) < DELTA)

    def testApproxErf(self):
        DELTA = 1e-5
        x = torch.Tensor([0.0, 0.1234, -0.1234, 1.0, -1.0, 2.12, -2.12])
        erf = torch.Tensor([0.0, 0.13853800, -0.13853800, 0.84270079, -0.84270079, 0.99728361, -0.99728361])
        result = _approx_erf(x)

        for e, r in zip(erf, result):
            self.assertTrue(abs(e - r) < DELTA)

    def testStandardGaussian(self):
        DELTA = 1e-5
        x = torch.Tensor([0.0, 0.1234, -0.1234, 1.0, -1.0, 2.12, -2.12])
        stg = torch.Tensor([0.5, 0.549105, 0.450895, 0.8413447461, 0.15865525393, 0.982997, 0.017003])
        result = _standard_gaussian_cdf(x)

        for s, r in zip(stg, result):
            self.assertTrue(abs(s - r) < DELTA)


if __name__ == "__main__":
    unittest.main()
