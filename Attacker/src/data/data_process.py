from torch.utils.data import Dataset
import torch
from torch.utils.data.dataset import T_co


class Dataset(Dataset):
    def __init__(self, data_path, sst) -> None:
        super().__init__()
        self.user_embeddiing, self.user_attribute = self.get_data(data_path, sst)

    def get_data(self, data_path, sst):
        checkpoint = torch.load(data_path)

        return checkpoint['embedding'][0], checkpoint[sst]

    def __len__(self):
        return len(self.user_embeddiing)

    def __getitem__(self, index) -> T_co:
        return self.user_embeddiing[index], self.user_attribute[index]
