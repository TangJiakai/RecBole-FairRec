from typing import Optional
from pytorch_lightning.utilities.types import EVAL_DATALOADERS, TRAIN_DATALOADERS
from torch.utils.data import DataLoader
import pytorch_lightning as pl
import torch
from torch.utils.data import SubsetRandomSampler
import numpy as np
import copy

from .data_process import Dataset
from .utils import get_data

class DataInterface(pl.LightningDataModule):
    def __init__(self, data_path, sst, train_bs, test_bs, num_workers=8,
    train_transforms=None, val_transforms=None, test_transforms=None, dims=None):
        super().__init__(train_transforms=train_transforms, val_transforms=val_transforms, test_transforms=test_transforms, dims=dims)

        self.data_path = data_path
        self.train_bs = train_bs
        self.test_bs = test_bs
        self.num_workers = num_workers
        self.embedding, self.attr = get_data(data_path, sst)

        self.train_split_ratio = 0.7
        self.valid_split_ratio = 0.1

    def setup(self, stage: Optional[str] = None):

        unique_attr = torch.unique(self.attr)
        train_indices = []
        valid_indices = []
        test_indices = []
        for attr in unique_attr:
            indices = torch.where(self.attr==attr)[0]
            train_split = int(np.floor(self.train_split_ratio*len(indices)))
            valid_split = max(1, int(np.floor(self.valid_split_ratio*len(indices)))) + train_split
            if valid_split >= len(indices):
                raise ValueError('no value in valid or test dataset!')
            train_indices.extend(indices[:train_split])
            valid_indices.extend(indices[train_split:valid_split])
            test_indices.extend(indices[valid_split:])
        
        if stage == 'fit' or stage is None:
            self.train_dataset = Dataset(self.embedding[train_indices], self.attr[train_indices])
            self.valid_dataset = Dataset(self.embedding[valid_indices], self.attr[valid_indices])
        
        if stage == 'test' or stage is None:
            self.test_dataset = Dataset(self.embedding[test_indices], self.attr[test_indices])
    
    def train_dataloader(self) -> TRAIN_DATALOADERS:
        return DataLoader(self.train_dataset, self.train_bs, shuffle=True)

    def val_dataloader(self) -> EVAL_DATALOADERS:
        return DataLoader(self.valid_dataset, self.test_bs)
    
    def test_dataloader(self) -> EVAL_DATALOADERS:
        return DataLoader(self.test_dataset, self.test_bs)