

# Numerical
import pandas as pd


def getEmptyDataFrame() -> pd.DataFrame:
    """
    Gets an empty dataframe used to represent the training information

    Return:
        An empty dataframe
    """
    DF_COLUMNS = ['Epoch', 'Loss', 'Run Type', 'Model']
    return pd.DataFrame(columns=DF_COLUMNS)


def addMetricRow(df: pd.DataFrame, epoch: int, metric_avg: float, metric_source: str, model_id: str) -> pd.DataFrame:
    """
    Add a metric row to the given dataframe

    Args:
        df (pd.DataFrame): Dataframe to add to
        epoch (int): Epoch which the metric belongs to
        metric_avg (float): Metric to add
        metric_source (str): Where the metric originates from (training/testing/ks)
        model_id (str): Model string name

    Returns:
        The input dataframe with the added row
    """
    metric_row = (epoch, metric_avg, metric_source, model_id)
    df = df.append(pd.Series(metric_row, index=df.columns), ignore_index=True)
    return df
