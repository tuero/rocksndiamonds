"""
File: preprocess.py
Author: Jake Tuero (tuero@ualberta.ca)
Date: April 4, 2020
Description: Helper functions for data preprocessing
"""

# Library
import sys
import logging
from enum import IntEnum
from typing import Tuple, List
import itertools

# Numeric
import numpy as np

# Pytorch
import torch
from torch.utils.data import DataLoader, Dataset


# Data paths and extensions
ENGINE_DATA_PATH = "../../training_data/"
CONSOLIDATED_DATA_PATH = "../consolidated_training_data/"
FEATURE_EXT = "feature.zip"
OBSERVATION_EXT = "observation.zip"


class CustomDataset(Dataset):

    def __init__(self, levelsets, level_range, lbd_per_level):
        self.level_names = []
        self.sample_counts = []

        # Set level data file names
        for levelset in levelsets:
            for level_num in range(1, level_range + 1):
                self.level_names.append(levelset + '_' + str(level_num))

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
                for i, channel in enumerate(feature):
                    for row in channel:
                        for d in row:
                            channel_sums[i].add_(d)
                            channel_squared_sums[i].add_(d**2)
                            counts += 1
                self.samples.append((level, idx))

                # Look for max runtime to normalize
                self.max_runtime = max(self.max_runtime, obs[0])

            del level_features
            del level_obs
            self.sample_counts.append(counts)

        # Calculate mean/std of features
        N = sum(self.sample_counts)
        self.mean = channel_sums.div_(N)
        self.variance = channel_squared_sums.div_(N - 1) - (channel_sums**2).div_(N * (N - 1))
        self.std = torch.sqrt(self.variance)

    def __len__(self):
        return len(self.sample_counts)

    def __getitem__(self, idx):
        if torch.is_tensor(idx):
            idx = idx.tolist()

        # Load level tensor and get feature/obs row
        level_name, row = self.samples[idx]
        feat = torch.load(ENGINE_DATA_PATH + level_name + '_' + FEATURE_EXT)[row]
        obs = torch.load(ENGINE_DATA_PATH + level_name + '_' + OBSERVATION_EXT)[row]

        # Standardize features
        feat.sub_(self.mean)
        feat.div_(self.std)

        # Normalize observations
        obs.div_(self.max_runtime)

        return (feat, obs)


def preprocessFeatures(features: torch.Tensor) -> torch.Tensor:
    """
    Standardize the 2-channel layer input features (channel-wise)

    Args:
        features (torch.Tensor) : A (N,C,W,H) tensor to preprocess

    Returns:
        The same shape tensor, with (W,H) standardized over each channel C
    """
    # features is shape [N, 2, 39, 39]
    # permute so that channel is top-most axis
    temp_feats = features.clone().permute(*[i for i in range(1, len(features.shape))], 0).contiguous()

    # Calculate mean/std for standardization
    mean = temp_feats.view(2, -1).mean(dim=1)
    std = temp_feats.view(2, -1).std(dim=1)

    # Normalize original features
    for f in features:
        for i, (mu, sigma) in enumerate(zip(mean.tolist(), std.tolist())):
            f[i].sub_(mu)
            f[i].div_(sigma)

    return features


# IntEnum for preprocessing types
class PreprocessMode(IntEnum):
    NONE = 0
    LOG_TRANSFORM = 1
    NORMALIZE = 2
    BOTH = 3


def preprocessObservations(observations: torch.Tensor, preprocess_mode: PreprocessMode = PreprocessMode.NONE,
                           col: int = 0) -> torch.Tensor:
    """
    Preprocess the observed runtimes (output) following the input mode.

    Args:
        observations (torch.Tensor): A (N,m) tensor
        preprocess_mode (PreprocessMode): Type of preprocessing to perform
        col (int): The column to perform the transformation on

    Returns:
        The same shape tensor, with the transformation applied to the given column
    """
    logger = logging.getLogger()

    # Sanity check
    if not (preprocess_mode >= 0 and preprocess_mode < 4):
        logger.error("Unknown proprocess mode: {}".format(preprocess_mode))
        sys.exit()
    if len(observations.shape) < 2:
        logger.error("Unknown input tensor shape, {}. Should be (N,...)".format(preprocess_mode))
        sys.exit()
    if not (col >= 0 and col < observations.shape[1]):
        logger.error("Input column is out of range: {}".format(col))
        sys.exit()

    # Log transform
    if preprocess_mode & PreprocessMode.LOG_TRANSFORM:
        observations = torch.log(observations)

    # Normalize to range [0,1]
    if preprocess_mode & PreprocessMode.NORMALIZE:
        max_ = torch.max(observations)
        observations = observations / max_

    return observations


def getDataLoader(features: torch.Tensor, observations: torch.Tensor, batch_size: int = 64) -> DataLoader:
    """
    Get a DataLoader for the given featurs/observations

    Args:
        features (Torch.tensor): Tensor of input preprocessed features
        observations (Torch.tensor): Tensor of output preprocessed observations
        batch_size (int): Size of the batches
    """
    data = [(feat, obs) for feat, obs in zip(features, observations)]
    return DataLoader(data, batch_size=batch_size, shuffle=True)


DATA_DIR = '../consolidated_training_data/'


# Type of data to use
class DataMode(IntEnum):
    NOKEY_SOLUTIONS = 1
    NOKEY_BOTH = 2
    WITHKEY_SOLUTIONS = 3
    WITHKEY_ALL = 4


def getDataModeFile(data_mode: DataMode = DataMode.NOKEY_SOLUTIONS) -> str:
    """
    Get the file for the data mode type, used in training

    Args:
        data_mode (DataMode): The data type to train on

    Returns:
        String file name of the data to use
    """
    logger = logging.getLogger()

    if data_mode == DataMode.NOKEY_SOLUTIONS:
        return "train_nokey_only_solution"
    if data_mode == DataMode.NOKEY_SOLUTIONS:
        return ""
    if data_mode == DataMode.NOKEY_SOLUTIONS:
        return ""
    if data_mode == DataMode.NOKEY_SOLUTIONS:
        return ""

    # Othewise, we have an unknown data mode
    logger.error("Unknown data mode: {}".format(data_mode))
    sys.exit()


def loadDataTensors(data_mode: DataMode = DataMode.NOKEY_SOLUTIONS) -> Tuple[torch.Tensor, torch.Tensor]:
    """
    Loads training data based on the data mode given

    Args:
        data_mode (DataMode): The type of data to load

    Returns:
        Tuple of tensors representing the features and observations
    """
    features = torch.load(DATA_DIR + getDataModeFile() + '_feats.pt')
    observations = torch.load(DATA_DIR + getDataModeFile() + '_obs.pt')
    return features, observations


def loadDataTensorsAndPreprocess(data_mode: DataMode = DataMode.NOKEY_SOLUTIONS,
                                 preprocess_mode: PreprocessMode = PreprocessMode.NONE,
                                 col: int = 0) -> Tuple[torch.Tensor, torch.Tensor]:
    """
    Loads training data and preprocesses based on data and preprocessing modes

    Args:
        data_mode (DataMode): The type of data to load
        preprocess_mode (PreprocessMode): Type of preprocessing to perform
        col (int): The column to perform the transformation on

    Returns:
        Tuple of tensors representing the preprocessed features and observations
    """
    features, observations = loadDataTensors(data_mode)
    features = preprocessFeatures(features)
    observations = preprocessObservations(observations, preprocess_mode, col)

    return features, observations


def splitData(features: torch.Tensor, observations: torch.Tensor, split_ratio: int,
              seed: int) -> Tuple[torch.Tensor, torch.Tensor, torch.Tensor, torch.Tensor]:
    """
    Split the features and observations randomly into subsets sized by split_ratio

    Args:
        features (torch.Tensor): Tensor of features
        observations (torch.Tensor): Tensors of observations
        split_ration (float): Percentage of data to use for training
        seed (int): Seed to use, or -1 if no seed.

    Returns:
        The train feautres, validation features, train observations, and validation observations
    """
    # Seed
    if seed == -1:
        np.random.seed(0)

    # Get split indices
    indices = np.random.permutation(features.shape[0])
    num_train = int(len(indices) * split_ratio)

    # Split
    training_idx, validation_idx = indices[:num_train], indices[num_train:]
    train_features, validation_features = features[training_idx, :], features[validation_idx, :]
    train_observations, validation_observations = observations[training_idx, :], observations[validation_idx, :]

    return train_features, validation_features, train_observations, validation_observations


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


def _uniqueFeaturesAndObservations(feats: torch.Tensor, obs: torch.Tensor) -> Tuple[torch.Tensor, torch.Tensor]:
    """
    Find the unique observation tensors, and matches with the corresponding feature tensor

    Args:
        feats (torch.Tensor): The tensor of features
        obs (torch.Tensor): The tensor of observations

    Returns
        Tuple of unique features with corresponding observations
    """
    # Convert to numpy
    feats_np = feats.numpy()
    obs_np = obs.numpy()

    # Get unique and find matching indices
    _, idx = np.unique(feats_np, return_index=True, axis=0)
    feats_np = feats_np[np.sort(idx)]
    obs_np = obs_np[np.sort(idx)]

    return torch.from_numpy(feats_np), torch.from_numpy(obs_np)


def gatherDataFromEngineOutput(data_levelsets: List[str], only_solution: bool = True,
                               extend_dim: bool = True)  -> Tuple[torch.Tensor, torch.Tensor, torch.Tensor, torch.Tensor]:
    """
    Gather the data produced by the engine runs into a single tensor output for training

    Args:
        data_levelsets (List[str]): List of levelset names to import from
        only_solution (bool): Flag if we are only wanting to gather solution observations
        extend_dim (bool): Flag to extend the dimension of the features

    Returns:
        Consolidated feature and observation tensors, where top-most dim is are the samples
    """
    logger = logging.getLogger()
    features = []
    observations = []
    data_levels = range(1, 991)

    for levelset, levelnum in itertools.product(*[data_levelsets, data_levels]):
        level_feature_path = '{}{}_{}_{}'.format(ENGINE_DATA_PATH, levelset, levelnum, FEATURE_EXT)
        level_observation_path = '{}{}_{}_{}'.format(ENGINE_DATA_PATH, levelset, levelnum, OBSERVATION_EXT)

        try:
            level_features = torch.load(level_feature_path)
        except Exception:
            logger.error('File not found: {}'.format(level_feature_path))

        try:
            level_observations = torch.load(level_observation_path)
        except Exception:
            logger.error('File not found: {}'.format(level_observation_path))

        found_solution = False
        for feat, obs in zip(level_features, level_observations):
            # If we are only looking for solution observations
            if only_solution and obs[1] != 1:
                continue
            # Solution observation for this level
            if obs[1] == 1:
                found_solution = True

            features.append(_addRowAndCol(feat.clone()) if extend_dim else feat.clone())
            observations.append(obs.clone())

        # No solution found
        if not found_solution:
            logger.warning('No solution found for level {}_{}'.format(levelset, levelnum))

        # Not sure if required, release tensor from system memory if dataset is extremely large
        del level_features
        del level_observations

    return _uniqueFeaturesAndObservations(torch.stack(features), torch.stack(observations))


def saveConsolidatedTensor(tensor: torch.Tensor, file_name: str):
    """
    Saves the tensor to the consolidated training data directory

    Args:
        tensor (torch.Tensor): Tensor to save
        file_name (str): Name of the saved file
    """
    torch.save(tensor, CONSOLIDATED_DATA_PATH + file_name + '.pt')
