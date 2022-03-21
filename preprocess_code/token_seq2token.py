import pandas as pd
import numpy as np

def change_seq_value(old_str,name_list):
    names = old_str.lower().split(' ')
    new_name = old_str
    for name in names:
        if name in name_list:
            new_name = name
    if new_name == old_str:
        new_name = names[0]
    return new_name


def change_seq2token(filename,change_column_name,keep_values):
    df = pd.read_csv(filename, sep='\t')
    df[change_column_name] = df[change_column_name].apply(lambda x:change_seq_value(x,keep_values))
    df.rename(columns={change_column_name:change_column_name.split('_')[0]},inplace=True)
    return df


keep_values = ['action','crime','musical','romance','sci-fi']
change_column_name = 'genre:token_seq'
df = change_seq2token(r'F:\DATASET\MovieLens\ml-1m\ml-1m.item','genre:token_seq',['action','crime','musical','romance','sci-fi'])
df.to_csv('F:\DATASET\MovieLens\FOCF ml-1M\ml-1M.item',sep='\t',index=False)