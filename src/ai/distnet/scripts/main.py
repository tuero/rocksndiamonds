
# Library
import sys
import argparse
import configparser
import logging
import traceback
from datetime import datetime
from typing import List
import unittest

# Pytorch
import torch

# Modules
from data_handler import loadDataTensorsAndPreprocess, getDataLoader, splitData
from data_handler import DataMode, PreprocessMode, gatherDataFromEngineOutput, saveConsolidatedTensor
import loss_functions
from loss_functions import LossFunctionTypes
from model import DisNetCNN, getDisnetConfig, FilterMode
from train import train, getTrainingConfig


def parseConfig(config_section):
    # Get logger/config
    logger = logging.getLogger()
    config = configparser.ConfigParser()
    config.read('training_config.ini')

    if config_section not in config.section():
        logger.error("Training config {} section was not found. See training_config.ini".format(config_section))
        sys.exit()

    # Start with default configs
    training_config = getTrainingConfig()
    model_config = getDisnetConfig()

    # Get items in the config option and update our training/model config
    try:
        # Training config items
        if config.has_option(config_section, 'n_epochs'):
            training_config['n_epochs'] = int(config.get(config_section, 'n_epochs'))
        if config.has_option(config_section, 'batch_size'):
            training_config['batch_size'] = int(config.get(config_section, 'batch_size'))
        if config.has_option(config_section, 'loss_fn'):
            training_config['loss_fn'] = LossFunctionTypes[config.get(config_section, 'loss_fn')]
        if config.has_option(config_section, 'start_rate'):
            training_config['start_rate'] = float(config.get(config_section, 'start_rate'))
        if config.has_option(config_section, 'end_rate'):
            training_config['end_rate'] = float(config.get(config_section, 'end_rate'))
        if config.has_option(config_section, 'clip_gradient_norm'):
            training_config['clip_gradient_norm'] = float(config.get(config_section, 'clip_gradient_norm'))
        if config.has_option(config_section, 'split_ratio'):
            training_config['split_ratio'] = int(config.get(config_section, 'split_ratio'))
        if config.has_option(config_section, 'data_mode'):
            training_config['data_mode'] = DataMode[config.get(config_section, 'data_mode')]
        if config.has_option(config_section, 'preprocess_mode'):
            training_config['preprocess_mode'] = PreprocessMode[config.get(config_section, 'preprocess_mode')]
        if config.has_option(config_section, 'seed'):
            training_config['seed'] = int(config.get(config_section, 'seed'))

        # Model config items
        if config.has_option(config_section, 'n_convdepth'):
            model_config['n_convdepth'] = int(config.get(config_section, 'n_convdepth'))
        if config.has_option(config_section, 'kernel_size'):
            model_config['kernel_size'] = int(config.get(config_section, 'kernel_size'))
        if config.has_option(config_section, 'n_fcdepth'):
            model_config['n_fcdepth'] = int(config.get(config_section, 'n_fcdepth'))
        if config.has_option(config_section, 'output_size'):
            model_config['output_size'] = int(config.get(config_section, 'output_size'))
        if config.has_option(config_section, 'drop_value'):
            model_config['drop_value'] = float(config.get(config_section, 'drop_value'))
        if config.has_option(config_section, 'filter_mode'):
            training_config['filter_mode'] = FilterMode[config.get(config_section, 'filter_mode')]
        if config.has_option(config_section, 'single_out_filter'):
            model_config['single_out_filter'] = bool(config.get(config_section, 'single_out_filter'))
    except Exception:
        logger.error("Unknown error parsing configuration.")
        logger.error(traceback.format_exc())
        sys.exit()

    return training_config, model_config


def runTrain(config_section):
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

    # Parse and get configs
    training_config, model_config = parseConfig(config_section)

    # Build model
    model = DisNetCNN(model_config)

    # Get data
    features, observations = loadDataTensorsAndPreprocess(training_config['data_mode'], training_config['preprocess_mode'])

    # Create dataloaders
    train_features, validation_features, train_observations, \
        validation_observations = splitData(features, observations, training_config['split_ratio'], training_config['seed'])
    batch_size = training_config['batch_size']
    train_loader = getDataLoader(train_features, train_observations, batch_size)
    validation_loader = getDataLoader(validation_features, validation_observations, batch_size)

    # Train
    df_train, model = train(model, device, train_loader, validation_loader, training_config)


def runGatherData(levelsets: List[str], file_name_prefix: str, only_solution: bool):
    """
    Gather the raw data from the engine and store for learning

    Args:
        levelsets (List[str]): List of levelset names to gather from
        file_name_prefix (str): File name prefix to identify data
        only_solution (bool): Flag is we are only gathering solution observations
    """
    logger = logging.getLogger()

    # No file given
    if file_name_prefix == "":
        logger.error('No filename given.')

    features, observations = gatherDataFromEngineOutput(levelsets, only_solution)
    saveConsolidatedTensor(features, file_name_prefix + '_feats')
    saveConsolidatedTensor(observations, file_name_prefix + '_obs')


def testLossFunctions():
    """
    Run the individual tests in each module
    """
    suite = unittest.TestLoader().loadTestsFromModule(loss_functions)
    unittest.TextTestRunner(verbosity=2).run(suite)


if __name__ == "__main__":
    # Arg parser
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", help="Mode: Type of script to run.", required=True,
                        choices=["train", "unit_test", "gather_data"], type=str.lower)
    parser.add_argument("--config_section", help="Configuration to use for training", required=False,
                        default="DEFAULT", type=str.lower)
    parser.add_argument("--levelsets", help="List of levelsets", required=False, nargs='+')
    parser.add_argument("--obs_single", help="Flag to only gather solution data", required=False,
                        dest='only_solution', action='store_true')
    parser.add_argument("--obs_all", help="Flag to only gather solution data", required=False,
                        dest='only_solution', action='store_false')
    parser.add_argument("--output_name", help="Name prefix of output data", required=False,
                        default="", type=str.lower)
    args = parser.parse_args()

    # Set logger settings
    str_now = datetime.now().strftime('%Y-%m-%d-%H:%M:%S')
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s [%(threadName)-12.12s] [%(levelname)-5.5s]  %(message)s",
        handlers=[
            logging.FileHandler("../logs/{}_{}.log".format(args.mode, str_now)),
            logging.StreamHandler(sys.stdout)
        ]
    )

    # Run respective script
    if args.mode == "train":
        runTrain(args.config_section)
    elif args.mode == "gather_data":
        runGatherData(args.levelsets, args.output_name, args.only_solution)
    elif args.mode == "unit_test":
        testLossFunctions()

    # Save mode?
