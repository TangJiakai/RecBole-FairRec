import torch
import os

checkpoint = torch.load('../dataset/BS_FairGR_embed-cm-[age_occupation].pth')
print(checkpoint.keys())