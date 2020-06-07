"""
File: export.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Export model and helper functions
"""

# Library
import logging

# PyTorch
import torch

# Module
from models.distnet import DistNetCNN
from models.bayes_distnet import BayesDistNetCNN


# Export extensions
EXT_LEVEL = '.txt'
EXT_MODEL = '.pt'


def exportModel(model, dataset, indices, save_name):
    logger = logging.getLogger()
    level_names = dataset.getLevelsFromIndices(indices)

    if type(model) is BayesDistNetCNN:
        path = './export/Bayesian/bayesian_{}'.format(save_name)
    elif type(model) is DistNetCNN:
        path = './export/DistNet/distnet_{}'.format(save_name)
    else:
        raise ValueError('Unknown net type.')

    # Export the levels to test for the engine
    logger.info('Exporting test files to {}{}'.format(path, EXT_LEVEL))
    with open(path + EXT_LEVEL, 'w') as file_out:
        for level in level_names:
            file_out.write(level + '\n')

    # Set model parameters to not require gradient
    model.eval()
    for p in model.parameters():
        p.requires_grad_(False)

    # Export the model
    logger.info('Exporting model to {}{}'.format(path, EXT_MODEL))
    traced_model = torch.jit.script(model)
    traced_model.save(path + EXT_MODEL)
