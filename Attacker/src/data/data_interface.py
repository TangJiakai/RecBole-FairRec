from typing import Optional
from pytorch_lightning.utilities.types import EVAL_DATALOADERS, TRAIN_DATALOADERS
from torch.utils.data import DataLoader
import pytorch_lightning as pl
import torch
from torch.utils.data import SubsetRandomSampler
import numpy as np


from .data_process import Dataset


class DataInterface(pl.LightningDataModule):
    def __init__(self, data_path, sst, batch_size, num_workers=8,
    train_transforms=None, val_transforms=None, test_transforms=None, dims=None):
        super().__init__(train_transforms=train_transforms, val_transforms=val_transforms, test_transforms=test_transforms, dims=dims)

        self.data_path = data_path
        self.batch_size = batch_size
        self.num_workers = num_workers
        self.dataset = Dataset(self.data_path, sst)

        self.train_split_ratio = 0.7
        self.valid_split_ratio = 0.1

    def setup(self, stage: Optional[str] = None):

        dataset_size = len(self.dataset)
        indices = list(range(dataset_size))
        train_split = int(np.floor(self.train_split_ratio*dataset_size))
        valid_split = int(np.floor(self.valid_split_ratio*dataset_size)) + train_split
        train_indices, valid_indices, test_indices = indices[:train_split], indices[train_split:valid_split], indices[valid_split:]

        if stage == 'fit' or stage is None:
            self.train_sampler = SubsetRandomSampler(train_indices)
            self.valid_sampler = SubsetRandomSampler(valid_indices)

        if stage == 'test' or stage is None:
            self.test_sampler = SubsetRandomSampler(test_indices)
    
    def train_dataloader(self) -> TRAIN_DATALOADERS:
        return DataLoader(self.dataset, self.batch_size, sampler=self.train_sampler)

    def val_dataloader(self) -> EVAL_DATALOADERS:
        return DataLoader(self.dataset, self.batch_size, sampler=self.valid_sampler, shuffle=False)
    
    def test_dataloader(self) -> EVAL_DATALOADERS:
        return DataLoader(self.dataset, self.batch_size, sampler=self.test_sampler, shuffle=False)