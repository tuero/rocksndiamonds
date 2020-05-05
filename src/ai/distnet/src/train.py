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
from data.learning_metrics import getEmptyDataFrame, addMetricRow
from config.config_handler import getModelConfig, getTrainingConfig, trainingConfigToStr, modelConfigToStr
from models.model_util import weights_init, getModel, count_parameters
from models.bayes_distnet import BayesDistNetCNN
from models.distnet import DistNetCNN
from loss_functions import getLossFunction, getNumberOfParameters, runKSTest, runKSTestBBB, runTTest, runTTestBBB


def train_model(model, device, train_loader, optimizer, training_config, total_len):
    training_loss = 0.0
    model.train()
    loss_fn = getLossFunction(training_config['loss_fn'])
    n_ens = training_config['n_ens']
    clip_gradient_norm = training_config['clip_gradient_norm']

    # Train with dropout
    train_data = []
    for i, data in enumerate(train_loader):
        # get the inputs; data is a list of [features, runtime]
        inputs, rts = data
        train_data.append((inputs, rts))
        inputs, rts = inputs.to(device), rts.to(device)

        # zero the parameter gradients
        optimizer.zero_grad()

        # Propogate input to model and calculate loss
        if type(model) is BayesDistNetCNN:
            kl = 0.0
            outputs = torch.zeros(inputs.shape[0], 1, n_ens).to(device)
            # Sample from variational posterior
            for j in range(n_ens):
                net_out, _kl = model(inputs)
                kl += _kl
                outputs[:, :, j] = net_out
            kl = kl / n_ens
            loss = kl*0.35 + total_len * loss_fn(outputs, rts)
        elif type(model) is DistNetCNN:
            outputs = model(inputs)
            loss = loss_fn(outputs, rts)
        else:
            raise ValueError('Unknown net type.')

        # Propogate loss backwards and step optimizer
        loss.backward()

        # Clip the gradients
        if clip_gradient_norm is not None:
            torch.nn.utils.clip_grad_norm_(model.parameters(), clip_gradient_norm)

        # Step optimizer
        optimizer.step()

        training_loss += loss.item()

    return training_loss / len(train_loader)


def validate_model(model, device, validation_loader, training_config, val_len, verbose_length=-1):
    logger = logging.getLogger()
    loss_fn = getLossFunction(training_config['loss_fn'])
    n_ens = training_config['n_ens']

    # Bayesian can't be in eval mode so that we can sample and get a distribution
    if not type(model) is BayesDistNetCNN:
        model.eval()

    validation_loss, ks_counter, t_counter, ks_total = (0.0, 0.0, 0.0, 0.0)
    dist_from_means = []
    variances = []
    for i, val_data in enumerate(validation_loader):
        val_inputs, val_rts = val_data
        val_inputs, val_rts = val_inputs.to(device), val_rts.to(device)

        if type(model) is BayesDistNetCNN:
            kl = 0.0
            outputs = torch.zeros(val_inputs.shape[0], 1, n_ens).to(device)
            for j in range(n_ens):
                net_out, _kl = model(val_inputs)
                kl += _kl
                outputs[:, :, j] = net_out
            kl = kl / n_ens
            val_loss = kl*0.35 + val_len * loss_fn(outputs, val_rts)

            # Find distribution parameters of samples
            mu = outputs.mean(dim=2)
            sigma_squared = outputs.var(dim=2)

            # Run statistical tests
            for j in range(len(val_inputs)):
                ks_total += 1
                _, p1 = runKSTestBBB(val_rts[j], mu[j], sigma_squared[j])
                p2 = runTTestBBB(val_rts[j], mu[j], sigma_squared[j])
                if p1 < 0.01:
                    ks_counter += 1
                if p2 > 1.960:
                    t_counter += 1
                dist_from_means.append(abs(mu[j].item() - val_rts[j][0].item()))
                variances.append(sigma_squared[j].item())
                # Verbose printing
                if verbose_length > 0:
                    verb_str = 't={:>10.5f} mu={:.4f} sigma2={:.4f} ks_p={:.4f} z_p={:.4f}'
                    logger.info(verb_str.format(val_rts[j][0].item(), mu[j].item(), sigma_squared[j].item(), p1, p2))

        elif type(model) is DistNetCNN:
            outputs = model(val_inputs)
            val_loss = loss_fn(outputs, val_rts)

            # Run statistical tests
            for j in range(len(val_inputs)):
                ks_total += 1
                _, p1 = runKSTest(val_rts[j], training_config['loss_fn'], outputs[j])
                p2 = runTTest(val_rts[j], training_config['loss_fn'], outputs[j])
                if p1 < 0.01:
                    ks_counter += 1
                if p2 > 1.960:
                    t_counter += 1
                dist_from_means.append(abs(outputs[j][0].item() - val_rts[j][0].item()))
                variances.append(outputs[j][1].item())
                # Verbose printing
                if verbose_length > 0:
                    verb_str = 't={:>10.5f} mu={:.4f} sigma2={:.4f} ks_p={:.4f} z_p={:.4f}'
                    logger.info(verb_str.format(val_rts[j][0].item(), outputs[j][0].item(), outputs[j][1].item(), p1, p2))
        else:
            raise ValueError('Unknown net type.')

        validation_loss += val_loss.item()

    ks_avg = 0 if ks_total == 0 else ks_counter / ks_total * 100
    t_avg = 0 if ks_total == 0 else t_counter / ks_total * 100
    return validation_loss / len(validation_loader), ks_avg, t_avg, dist_from_means, variances


# Optimizers
# - learning rate start 1e-3 to end 1e-5 [done]
# - gradient clipping [done]
# - tanh activation function
# - SGD [done]
# - batch normalization (as a layer)
# - L2 regularization of 1e-4
def train(net_type: str, device: torch.device, train_loader: DataLoader, validation_loader: DataLoader,
          training_config: dict = getTrainingConfig(), model_config: dict = getModelConfig()) -> Tuple[pd.DataFrame, nn.Module]:
    """
    Train the model using the given training configuration

    Args:
        net_type (str): Model string
        device: Where to send data/model for training
        train_loader (torch.DataLoader) : Dataloader for training data
        validation_loader (torch.DataLoader) : Dataloader for validation data
        training_config (dict): Configuration for training
        model_config (dict): Configuration for the model

    Returns:
        DataFrame of loss data during training with testing every epoch
    """
    logger = logging.getLogger()

    # Set output size (loss function dependent) and create model
    model_config['output_size'] = getNumberOfParameters(training_config['loss_fn'])
    model = getModel(net_type, model_config)

    # Log the current configurations
    logger.info(trainingConfigToStr(training_config))
    logger.info(modelConfigToStr(model_config))
    logger.info('Number of parameters: {}'.format(count_parameters(model)))

    # Reset model and send model to device
    model.apply(weights_init)
    model.to(device)

    # Training config
    start_rate = training_config['start_rate']
    end_rate = training_config['end_rate']
    n_epochs = training_config['n_epochs']

    # Optimizer and scheduler
    optimizer = optim.SGD(model.parameters(), lr=start_rate, weight_decay=0.001)
    decay_rate = np.exp(np.log(end_rate / start_rate) / n_epochs)
    lr_scheduler = optim.lr_scheduler.ExponentialLR(optimizer=optimizer, gamma=decay_rate)
    logger.info('Decay rate: {}'.format(decay_rate))

    # Output loss information during training
    df_losses = getEmptyDataFrame()

    total_len = len(train_loader) * training_config['batch_size']
    val_len = len(validation_loader) * training_config['batch_size']
    for epoch in range(n_epochs):
        # Next epoch for training
        train_avg = train_model(model, device, train_loader, optimizer, training_config, total_len)
        eval_avg = 0.0
        if type(model) is DistNetCNN:
            eval_avg, _, _, _, _ = validate_model(model, device, train_loader, training_config, total_len)
        test_avg, ks_avg, t_avg, dist_from_means, variances = validate_model(model, device, validation_loader, training_config, total_len)

        # Calcualte and store metrics
        df_losses = addMetricRow(df_losses, epoch, train_avg, "Train", model.toStr())
        df_losses = addMetricRow(df_losses, epoch, eval_avg, "Train_Eval", model.toStr())
        df_losses = addMetricRow(df_losses, epoch, test_avg, "Test", model.toStr())
        df_losses = addMetricRow(df_losses, epoch, ks_avg, "KS", model.toStr())
        df_losses = addMetricRow(df_losses, epoch, t_avg, "T", model.toStr())
        df_losses = addMetricRow(df_losses, epoch, np.mean(dist_from_means), "D-MEAN", model.toStr())
        df_losses = addMetricRow(df_losses, epoch, np.mean(variances), "VAR", model.toStr())

        output_msg = "Epoch: {:>4d}, Training Loss {:>18,.4f}, Training Eval Loss {:>18,.4f}, Validation Loss {:>18,.4f}, %KS < 0.01 = {:.2f}, %Z < 0.05 = {:.2f}"
        logger.info(output_msg.format(epoch + 1, train_avg, eval_avg, test_avg, ks_avg, t_avg))

        # Update learning rate
        lr_scheduler.step()

    return df_losses, model
