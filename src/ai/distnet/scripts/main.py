
# Library
import sys
import argparse
import configparser
import logging
import traceback
from datetime import datetime
import unittest

# Numeric
import numpy as np
import pandas as pd

# Pytorch
import torch

import wandb

# Modules
from data_handler import CustomDataset, getDataLoader, splitData
from data_handler import PreprocessMode, InitializeMode, getTrainTestSubsetSampler
import loss_functions
from loss_functions import LossFunctionTypes, getLossFunction
from model import DisNetCNN, getDisnetConfig, FilterMode, distnetConfigToStr
from train import train, getTrainingConfig, trainingConfigToStr
from validation import gridSearch, getParamGrid, kfoldValidation, runKSTest


def parseConfig(config_section):
    # Get logger/config
    logger = logging.getLogger()
    config = configparser.ConfigParser()
    config.read('training_config.ini')

    if config_section not in config.sections() and config_section != "DEFAULT":
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
            training_config['split_ratio'] = float(config.get(config_section, 'split_ratio'))
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
            model_config['filter_mode'] = FilterMode[config.get(config_section, 'filter_mode')]
        if config.has_option(config_section, 'single_out_filter'):
            model_config['single_out_filter'] = bool(config.get(config_section, 'single_out_filter'))
    except Exception:
        logger.error("Unknown error parsing configuration.")
        logger.error(traceback.format_exc())
        sys.exit()

    return training_config, model_config


def runKFold(config_section, attribute_prefix, samples_per_level, data_divider=1, max_samples=-1):
    logger = logging.getLogger()

    # Parse and get configs
    training_config, model_config = parseConfig(config_section)

    combined_config = {**training_config, **model_config}
    combined_config['attribute_prefix'] = attribute_prefix
    combined_config['ssl'] = samples_per_level
    combined_config['data_divider'] = data_divider

    # Build model
    model = DisNetCNN(model_config, str(samples_per_level))

    # Get data
    levelsets = ["train1", "train2", "train3", "train4"]
    # levelsets = ["train1", "train2"]
    dataset = CustomDataset(levelsets, samples_per_level, attribute_prefix, training_config['preprocess_mode'], InitializeMode.LOAD_FROM_FILE)

    # Run kfold
    kfold_df_, avg_loss, ks_fail_percentage = kfoldValidation(model, dataset, training_config, model_config, combined_config,
                                                              data_divider=data_divider, num_folds=10, max_samples=max_samples)
    kfold_df_['config'] = 'samp-{}-lb-{}'.format(max_samples, samples_per_level - 1)
    kfold_df_.to_pickle('../ssl_compare_dfs/df_{}_{}_{}.pkl'.format(config_section, max_samples, samples_per_level - 1))
    output_msg = "Average Validation Loss {:>18,.4f}, %KS < 0.01 = {:.2f}"
    logger.info(output_msg.format(avg_loss, ks_fail_percentage))


def runGridSearch(config_section, attribute_prefix, samples_per_level):
    # Parse and get configs
    training_config, model_config = parseConfig(config_section)
    param_grid = getParamGrid(model_config['filter_mode'])

    # Get data
    levelsets = ["train1", "train2", "train3", "train4"]
    dataset = CustomDataset(levelsets, samples_per_level, attribute_prefix, training_config['preprocess_mode'], InitializeMode.LOAD_FROM_FILE)

    gridSearch(model_config, training_config, param_grid, dataset, num_folds=5,
               start_msg="{}_{}".format(training_config['loss_fn'].name, model_config['filter_mode'].name))


def runTrain(config_section, attribute_prefix, samples_per_level, data_divider=1, num_train=1, max_samples=-1,
             max_sol=-1, max_lb=-1):
    logger = logging.getLogger()
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

    # Parse and get configs
    training_config, model_config = parseConfig(config_section)

    # Build model
    model = DisNetCNN(model_config)

    # Get data
    levelsets = ["train1", "train2", "train3", "train4"]
    dataset = CustomDataset(levelsets, samples_per_level, attribute_prefix, training_config['preprocess_mode'], InitializeMode.LOAD_FROM_FILE)

    ks_counter = 0.0
    ks_total = 0.0
    test_loss = 0.0
    loss_fn = getLossFunction(training_config['loss_fn'])
    np.random.seed(training_config['seed'])

    combined_config = {**training_config, **model_config}
    combined_config['attribute_prefix'] = attribute_prefix
    combined_config['ssl'] = samples_per_level
    combined_config['data_divider'] = data_divider
    combined_config['max_samples'] = max_samples
    combined_config['max_sol'] = max_sol
    combined_config['max_lb'] = max_lb

    wandb.init(project="rnd_compare_lb_lognormal", config=combined_config)

    loss_data = []
    for c in range(num_train):

        # Create dataloaders
        split_ratio = 1 - ((1 - training_config['split_ratio']) / data_divider)
        train_indices, test_indices, = splitData(dataset, split_ratio, None, max_sol, max_lb)
        if max_samples > 0 and len(train_indices) > max_samples:
            train_indices = train_indices[:max_samples]
        logger.info('Number of training samples: {}'.format(len(train_indices)))
        logger.info('Number of testing samples: {}'.format(len(test_indices)))
        train_sampler, test_sampler = getTrainTestSubsetSampler(train_indices, test_indices)
        batch_size = training_config['batch_size']
        train_loader = getDataLoader(dataset, batch_size, train_sampler)
        validation_loader = getDataLoader(dataset, batch_size, test_sampler)

        logger.info(trainingConfigToStr(training_config))
        logger.info(distnetConfigToStr(model_config))

        # Train
        df_train, model = train(model, device, train_loader, validation_loader, training_config, c + 1)
        loss_data.append(df_train)

        model.eval()
        with torch.no_grad():
            for i, data in enumerate(validation_loader):
                inputs, rts = data
                inputs, rts = inputs.to(device), rts.to(device)
                outputs = model(inputs)
                loss = loss_fn(outputs, rts, reduce=False)
                # test_loss += loss.item()
                test_loss += torch.sum(loss)

                for j in range(len(inputs)):
                    # if i == 0:
                    #     logger.info('{} {}'.format(rts[j], outputs[j]))
                    ks_total += 1
                    D, p = runKSTest(rts[j], training_config['loss_fn'], outputs[j])
                    if p < 0.01:
                        ks_counter += 1

    df = pd.concat(loss_data)
    for i in range(training_config['n_epochs']):
        row = df[df['Epoch'] == i + 1]
        train_row = row[row['Run Type'] == 'Train']
        eval_row = row[row['Run Type'] == 'Train_Eval']
        test_row = row[row['Run Type'] == 'Test']
        ks_row = row[row['Run Type'] == 'KS']
        wandb.log({'epoch': i + 1, 'train_loss': train_row['Loss'].mean(), 'eval_loss': eval_row['Loss'].mean(),
                   'validation_loss': test_row['Loss'].mean(), 'ks': ks_row['Loss'].mean()})

    output_msg = "Validation Loss {:>18,.4f}, %KS < 0.01 = {:.2f}"
    logger.info(output_msg.format(test_loss / ks_total, ks_counter / ks_total * 100))


def init_dataset(config_section, attribute_prefix, samples_per_level):
    # Parse and get configs
    levelsets = ["train1", "train2", "train3", "train4"]
    training_config, _ = parseConfig(config_section)
    CustomDataset(levelsets, samples_per_level, attribute_prefix, training_config['preprocess_mode'], InitializeMode.SAVE_TO_FILE)


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
                        choices=["train", "init_dataset", "kfold", "grid_search", "unit_test"], type=str.lower)
    parser.add_argument("--config_section", help="Configuration to use for training", required=False,
                        default="DEFAULT", type=str.upper)
    parser.add_argument("--levelsets", help="List of levelsets", required=False, nargs='+')
    parser.add_argument("--attribute_prefix", help="Name prefix of dataset attributes", required=False,
                        default="", type=str.lower)
    parser.add_argument("--ssl", help="Samples per level", required=False,
                        default=1, type=int)
    parser.add_argument("--data_divider", help="Amount to divide number of samples", required=False,
                        default=1, type=int)
    parser.add_argument("--num_train", help="Number of times to retrain", required=False,
                        default=1, type=int)
    parser.add_argument("--save_model", help="Flag to save model", required=False,
                        default=1, type=int)
    parser.add_argument("--max_samples", help="Maximum number of samples to train on", required=False,
                        default=-1, type=int)
    parser.add_argument("--max_sol", help="Maximum number of solution samples", required=False,
                        default=-1, type=int)
    parser.add_argument("--max_lb", help="Maximum number of lowerbound samples", required=False,
                        default=-1, type=int)
    args = parser.parse_args()

    # Set logger settings
    str_now = datetime.now().strftime('%Y-%m-%d-%H:%M:%S')
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s [%(threadName)-12.12s] [%(levelname)-5.5s]  %(message)s",
        handlers=[
            logging.FileHandler("../logs/{}_{}_{}_{}.log".format(args.mode, args.config_section, args.ssl, str_now)),
            logging.StreamHandler(sys.stdout)
        ]
    )

    # Run respective script
    if args.mode == "train":
        runTrain(args.config_section, args.attribute_prefix, args.ssl, args.data_divider, args.num_train, args.max_samples,
                 args.max_sol, args.max_lb)
    elif args.mode == "init_dataset":
        init_dataset(args.config_section, args.attribute_prefix, args.ssl, args.data_divider)
    elif args.mode == "unit_test":
        testLossFunctions(args.config_section)
    elif args.mode == "kfold":
        runKFold(args.config_section, args.attribute_prefix, args.ssl, args.data_divider, args.max_samples)
    if args.mode == "grid_search":
        runGridSearch(args.config_section, args.attribute_prefix, args.ssl)

    # Save mode?
