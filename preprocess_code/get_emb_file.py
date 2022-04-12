import pandas as pd
import torch
import numpy as np

file_path = r'../saved_GraphSAGE/100k_BS_GraphSAGE_embed-[gender_age_occupation]-Mar-28-2022_16-45-26.pth'
user_emb_save_path = r'../dataset/FairGR_GraphSAGE_100k/FairGR_GraphSAGE_100k.user_emb'
item_emb_save_path = r'../dataset/FairGR_GraphSAGE_100k/FairGR_GraphSAGE_100k.item_emb'

checkpoint = torch.load(file_path)
user_id = checkpoint['user_id']
item_id = checkpoint['item_id']
user_embed = checkpoint['user_embedding'].detach().cpu().numpy()
item_embed = checkpoint['item_embedding'].detach().cpu().numpy()


user_embed_list = [' '.join([str(x) for x in _]) for _ in user_embed]
user_embed_df = pd.DataFrame(data={'uid:token':user_id,'user_emb:float_seq':user_embed_list})
user_embed_df.to_csv(user_emb_save_path,sep='\t',index=False)

item_embed_list = [' '.join([str(x) for x in _]) for _ in item_embed]
item_embed_df = pd.DataFrame(data={'iid:token':item_id,'item_emb:float_seq':item_embed_list})
item_embed_df.to_csv(item_emb_save_path,sep='\t',index=False)


