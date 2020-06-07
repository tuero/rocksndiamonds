
# Library
import sys
import argparse
import logging
from datetime import datetime
import unittest

# Numeric
import numpy as np

# Pytorch
import torch

# Modules
from data.data_handler import CustomDataset, getDataLoader, splitData, divideAndClipData, calculateDatasetMeanVar
from data.data_handler import InitializeMode, getTrainTestSubsetSampler
from config.config_handler import parseConfig
import loss_functions
from train import train, validate_model
from model_assessment import gridSearch, getParamGrid, kfoldValidation


levelsets = ["train1", "train2", "train3", "train4", "train5", "train6", "train7", "train8"]


def runKFold(net_type, config_section, attribute_prefix, samples_per_level, data_divider=1, max_samples=0, save_name=None):
    logger = logging.getLogger()

    # Parse and get configs
    training_config, model_config = parseConfig(config_section)

    # Get data
    dataset = CustomDataset(levelsets, samples_per_level, attribute_prefix, training_config['preprocess_mode'], InitializeMode.LOAD_FROM_FILE)

    # Run kfold
    # kfold_df_, avg_loss, ks_avg, t_avg = kfoldValidation(net_type, dataset, training_config, model_config, data_divider=data_divider, 
    #                                                      num_folds=5, max_samples=max_samples, save_name=save_name)
    kfold_df_ = kfoldValidation(net_type, dataset, training_config, model_config, data_divider=data_divider,
                                num_folds=5, max_samples=max_samples, save_name=save_name)
    kfold_df_['config'] = 'samp-{}-lb-{:.2f}-div-{}'.format(max_samples, samples_per_level, data_divider)
    # kfold_df_.to_pickle('../kfold_dfs/df_{}_samp-{}-lb-{}-div-{}_2.pkl'.format(config_section, max_samples, samples_per_level - 1, data_divider))
    kfold_df_.to_pickle('../kfold_dfs_key/df_{}_lb-{:.2f}-div-{}-full.pkl'.format(config_section, samples_per_level, data_divider))


def runGridSearch(net_type, config_section, attribute_prefix, samples_per_level):
    # Parse and get configs
    training_config, model_config = parseConfig(config_section)
    param_grid = getParamGrid(model_config['filter_mode'])

    # Get data
    dataset = CustomDataset(levelsets, samples_per_level, attribute_prefix, training_config['preprocess_mode'], InitializeMode.LOAD_FROM_FILE)
    start_msg = "{}_{}".format(training_config['loss_fn'].name, model_config['filter_mode'].name)

    gridSearch(net_type, model_config, training_config, param_grid, dataset, num_folds=5, start_msg=start_msg)


def runTrain(net_type, config_section, attribute_prefix, samples_per_level, data_divider=1, max_samples=-1,
             max_sol=-1, max_lb=-1):
    logger = logging.getLogger()
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

    # Parse and get configs
    training_config, model_config = parseConfig(config_section)

    # Get data
    dataset = CustomDataset(levelsets, samples_per_level, attribute_prefix, training_config['preprocess_mode'], InitializeMode.LOAD_FROM_FILE)

    np.random.seed(training_config['seed'])

    # Split data
    train_indices, test_indices = splitData(dataset, training_config['split_ratio'], max_sol, max_lb)
    train_indices = divideAndClipData(train_indices, data_divider, max_samples)
    test_indices = divideAndClipData(test_indices)
    logger.info('Number of training samples: {}'.format(len(train_indices)))
    logger.info('Number of testing samples: {}'.format(len(test_indices)))

    # Create dataloaders
    train_sampler, test_sampler = getTrainTestSubsetSampler(train_indices, test_indices)
    batch_size = training_config['batch_size']
    train_loader = getDataLoader(dataset, batch_size, train_sampler)
    validation_loader = getDataLoader(dataset, batch_size, test_sampler)

    # Train
    val_len = len(validation_loader) * training_config['batch_size']
    df_train, model = train(net_type, device, train_loader, validation_loader, training_config, model_config)
    _ = validate_model(model, device, validation_loader, training_config, val_len, num_epochs=4, verbose_length=200)

    # logger.info('Average distance from means: {}'.format(np.mean(dist_from_means)))
    # logger.info('Average variances: {}'.format(np.mean(variances)))


def init_dataset(attribute_prefix):
    # Parse and get configs
    # levelsets = ["train1", "train2", "train3", "train4"]
    # training_config, _ = parseConfig(config_section)
    calculateDatasetMeanVar(levelsets, attribute_prefix)
    # CustomDataset(levelsets, samples_per_level, attribute_prefix, training_config['preprocess_mode'], InitializeMode.SAVE_TO_FILE)


def testLossFunctions():
    """
    Run the individual tests in each module
    """
    suite = unittest.TestLoader().loadTestsFromModule(loss_functions)
    unittest.TextTestRunner(verbosity=2).run(suite)


if __name__ == "__main__":
    # Arg parser
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", help="Mode: Type of script to run.", required=False,
                        choices=["train", "init_dataset", "kfold", "grid_search", "unit_test"], type=str.lower)
    parser.add_argument("--net_type", help="Model type", required=False, choices=["distnet", "bayes_distnet"],
                        default="bayes_distnet", type=str.lower)
    parser.add_argument("--config_section", help="Configuration to use for training", required=False,
                        default="DEFAULT", type=str.upper)
    parser.add_argument("--levelsets", help="List of levelsets", required=False, nargs='+')
    parser.add_argument("--attribute_prefix", help="Name prefix of dataset attributes", required=False,
                        default="", type=str.lower)
    parser.add_argument("--ssl", help="Samples per level", required=False,
                        default=1, type=float)
    parser.add_argument("--data_divider", help="Amount to divide number of samples", required=False,
                        default=1, type=float)
    parser.add_argument("--save", help="Flag to save model", required=False, default=None)
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
            logging.FileHandler("../logs/{}_{}_{}_{}_{}.log".format(args.mode, args.config_section, args.ssl, args.attribute_prefix, str_now)),
            logging.StreamHandler(sys.stdout)
        ]
    )

    # Run respective script
    if args.mode == "train":
        runTrain(args.net_type, args.config_section, args.attribute_prefix, args.ssl, args.data_divider, args.max_samples,
                 args.max_sol, args.max_lb)
    elif args.mode == "init_dataset":
        # init_dataset(args.config_section, args.attribute_prefix, args.ssl, args.data_divider)
        init_dataset(args.attribute_prefix)
    elif args.mode == "unit_test":
        testLossFunctions(args.config_section)
    elif args.mode == "kfold":
        runKFold(args.net_type, args.config_section, args.attribute_prefix, args.ssl, args.data_divider, args.max_samples, args.save)
    if args.mode == "grid_search":
        runGridSearch(args.net_type, args.config_section, args.attribute_prefix, args.ssl)

    # Save mode?
