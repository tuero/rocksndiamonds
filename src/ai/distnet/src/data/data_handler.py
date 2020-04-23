"""
File: preprocess.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Helper functions for data preprocessing
"""

# Library
from enum import IntEnum
from typing import Tuple, List
import logging

# Numeric
import numpy as np

# Pytorch
import torch
from torch.utils.data import DataLoader, Dataset
from torch.utils.data.sampler import SubsetRandomSampler


def _addRowAndCol(feature_tensor: torch.Tensor):
    """
    Add an extra row and column to the input: (2,n,n) -> (2,n+1,n+1)

    Args:
        feature_tensor (torch.Tensor): A single feature tensor

    Returns:
        The same feature tensor with an added row/col
    """
    n = feature_tensor.size()[1]
    channels = []
    new_col = torch.zeros(n, 1)
    new_row = torch.zeros(1, n + 1)

    # Add row/col for each channel
    for channel in feature_tensor:
        output_tensor = torch.cat((channel, new_col), dim=1)
        output_tensor = torch.cat((output_tensor, new_row), dim=0)
        channels.append(output_tensor)

    return torch.stack(channels)


# IntEnum for preprocessing types
class PreprocessMode(IntEnum):
    NONE = 0
    LOG_TRANSFORM = 1
    NORMALIZE = 2
    BOTH = 3


class InitializeMode(IntEnum):
    SAVE_TO_FILE = 1
    LOAD_FROM_FILE = 2


# Data paths and extensions
ENGINE_DATA_PATH = "../../training_data/"
CONSOLIDATED_DATA_PATH = "../consolidated_training_data/"
DATASET_ATTRIBUTE_PATH = "../dataset_attributes/"
FEATURE_EXT = "feature.zip"
OBSERVATION_EXT = "observation.zip"
LEVEL_RANGE = 990


class CustomDataset(Dataset):

    def __init__(self, levelsets: List[str], lbd_per_level: int, attribute_prefix: str,
                 preprocess_mode: PreprocessMode = PreprocessMode.NORMALIZE,
                 initialize_mode: InitializeMode = InitializeMode.LOAD_FROM_FILE):
        self.level_names = []
        self.sample_counts = []
        self.preprocess_mode = preprocess_mode
        self.sol_observation = []
        logger = logging.getLogger()

        # Set level data file names
        for levelset in levelsets:
            for level_num in range(1, LEVEL_RANGE + 1):
                self.level_names.append(levelset + '_' + str(level_num))

        logger.info('Number of total levels: {}'.format(len(self.level_names)))

        self.samples = []
        self.max_runtime = 0

        # Get raw feature/obs data and aggregate
        channel_sums = torch.tensor([0.0, 0.0])
        channel_squared_sums = torch.tensor([0.0, 0.0])
        for level in self.level_names:
            counts = 0

            # Feature data
            level_features = torch.load(ENGINE_DATA_PATH + level + '_' + FEATURE_EXT)
            level_obs = torch.load(ENGINE_DATA_PATH + level + '_' + OBSERVATION_EXT)
            for idx, (feature, obs) in enumerate(zip(level_features, level_obs)):
                # Found enough samples
                if idx >= lbd_per_level:
                    break
                # Aggregate feature data for each channel
                if initialize_mode == InitializeMode.SAVE_TO_FILE:
                    for i, channel in enumerate(_addRowAndCol(feature)):
                        for row in channel:
                            for d in row:
                                channel_sums[i].add_(d)
                                channel_squared_sums[i].add_(d**2)
                                counts += 1
                # Save level with its index
                self.samples.append((level, idx))

                # Look for max runtime to normalize
                self.max_runtime = max(self.max_runtime, obs[0])
                self.sol_observation.append(obs[1])

            del level_features
            del level_obs
            self.sample_counts.append(counts)

        logger.info('Total Number of samples: {}'.format(len(self.samples)))

        # Calculate mean/std of features
        if initialize_mode == InitializeMode.SAVE_TO_FILE:
            N = sum(self.sample_counts)
            self.mean = channel_sums.div_(N)
            self.variance = channel_squared_sums.div_(N - 1) - (channel_sums**2).div_(N * (N - 1))
            self.std = torch.sqrt(self.variance)
            logger.info('Dataset mean: {}'.format(self.mean))
            logger.info('Dataset std: {}'.format(self.std))

            dataset_mean_path = DATASET_ATTRIBUTE_PATH + attribute_prefix + '_mean.pt'
            dataset_std_path = DATASET_ATTRIBUTE_PATH + attribute_prefix + '_std.pt'
            logger.info('Saving dataset mean to: {}'.format(dataset_mean_path))
            logger.info('Saving dataset std to: {}'.format(dataset_std_path))
            torch.save(self.mean, dataset_mean_path)
            torch.save(self.std, dataset_std_path)

        elif initialize_mode == InitializeMode.LOAD_FROM_FILE:
            if self.preprocess_mode & PreprocessMode.LOG_TRANSFORM:
                self.max_runtime = torch.log(self.max_runtime)
            self.mean = torch.load(DATASET_ATTRIBUTE_PATH + attribute_prefix + '_mean.pt')
            self.std = torch.load(DATASET_ATTRIBUTE_PATH + attribute_prefix + '_std.pt')
            logger.info('Dataset mean: {}'.format(self.mean))
            logger.info('Dataset std: {}'.format(self.std))

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, idx):
        if torch.is_tensor(idx):
            idx = idx.tolist()

        # Load level tensor and get feature/obs row
        level_name, row = self.samples[idx]
        feat = torch.load(ENGINE_DATA_PATH + level_name + '_' + FEATURE_EXT)[row]
        obs = torch.load(ENGINE_DATA_PATH + level_name + '_' + OBSERVATION_EXT)[row]

        # Add extra row/col to feat
        feat = _addRowAndCol(feat.clone())

        # Standardize features
        for i, (mean, std) in enumerate(zip(self.mean, self.std)):
            feat[i].sub_(mean)
            feat[i].div_(std)

        # Observations
        if self.preprocess_mode & PreprocessMode.LOG_TRANSFORM:
            torch.log_(obs[0])
        if self.preprocess_mode & PreprocessMode.NORMALIZE:
            obs[0].div_(self.max_runtime)

        return (feat, obs)

    def findSolutionIdx(self):
        return [i for i, obs in enumerate(self.sol_observation) if obs == 1]

    def findLowerboundIdx(self):
        return [i for i, obs in enumerate(self.sol_observation) if obs == 0]


def splitData(dataset: Dataset, split_ratio: int, max_sol: int = -1,
              max_lb: int = -1) -> Tuple[torch.Tensor, torch.Tensor]:
    """
    Split the features and observations randomly into subsets sized by split_ratio

    Args:
        dataset (torch.tensor) : The complete dateset to use for training/testing
        split_ration (float): Percentage of data to use for training

    Returns:
        The incides for the train/test set
    """
    # Ensure we get proper mix of solution/lower bound data
    dataset_sol_idx = dataset.findSolutionIdx()
    dataset_lb_idx = dataset.findLowerboundIdx()

    # Shuffle
    np.random.shuffle(dataset_sol_idx)
    np.random.shuffle(dataset_lb_idx)

    # Find splits
    _sol_split = int(np.floor(split_ratio * len(dataset_sol_idx)))
    _lb_split = int(np.floor(split_ratio * len(dataset_lb_idx)))

    # Train on mix on solution/lower bound, but we only can validate on solution observations
    train_indices = dataset_sol_idx[_sol_split:] + dataset_lb_idx[_lb_split:]
    val_indices = dataset_sol_idx[:_sol_split]

    return train_indices, val_indices


def divideAndClipData(indices: list, data_divider: int = 1, max_samples: int = 0):
    """
    Divide the train/test set by data_divider and set max number of training samples

    Args:
        indicies: Indices to parce
        data_divider (int): Ratio of indices to use
        max_samples (int): If greater than 0, will clip the number of samples
    """
    np.random.shuffle(indices)
    indices = indices[:int(len(indices) / data_divider)]
    if max_samples > 0 and len(indices) > max_samples:
        indices = indices[:max_samples]

    return indices


def getTrainTestSubsetSampler(training_idx: List[int], test_idx: List[int]):
    """
    Get the training and test subset sampler for the given indices

    Args:
        training_idx (List[int]): List of training indices to use
        test_idx (List[int]): List of test indices to use

    Returns:
        Subset samplers for training and testing respectively
    """
    train_sampler = SubsetRandomSampler(training_idx)
    test_sampler = SubsetRandomSampler(test_idx)
    return train_sampler, test_sampler


def getDataLoader(dataset: Dataset, batch_size: int = 64, sampler: SubsetRandomSampler = None) -> DataLoader:
    """
    Get a DataLoader for the given featurs/observations

    Args:
        dataset (torch.tensor) : The complete dateset to use for training/testing
        batch_size (int): Size of the batches
        sampler (SubsetRandomSampler): The subset sampler to use for specific train/test splits

    Returns:
        The dataloader for the given subset sampler
    """
    return DataLoader(dataset, batch_size=batch_size, sampler=sampler)
