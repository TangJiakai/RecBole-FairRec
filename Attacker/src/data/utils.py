from ast import Attribute
import torch


import torch


def get_data(data_path, sst):
    checkpoint = torch.load(data_path)
    embedding, attr = checkpoint['embedding'].detach(), checkpoint[sst]
    if all(attr):
        attr -= 1
    return embedding, attr