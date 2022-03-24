import torch


sst_embed = torch.load('../dataset/PFCN_MLP_embed-[gender_age_occupation]-none.pth')
print(sst_embed['embedding'].shape)