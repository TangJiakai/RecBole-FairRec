from torch.utils.data import Dataset
import torch
import random


class Dataset(Dataset):
    def __init__(self, embedding, attr):
        super().__init__()
        self.user_embeddiing, self.user_attribute = embedding, attr

    def __len__(self):
        return len(self.user_embeddiing)

    def __getitem__(self, index):
        return self.user_embeddiing[index], self.user_attribute[index]
