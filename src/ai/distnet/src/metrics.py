import torch


def calculate_kl(log_alpha):
    return 0.5 * torch.sum(torch.log1p(torch.exp(-log_alpha)))
