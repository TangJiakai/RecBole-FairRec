import os
import sys
import pytorch_lightning as pl
import argparse
import pytorch_lightning.callbacks as plc
from pytorch_lightning import Trainer
from torch.utils.tensorboard import summary

from model import ModelInterface, Classifier
from data import DataInterface


def load_callbacks():
    callbacks = []

    callbacks.append(plc.EarlyStopping(
        monitor='valid/auc',
        mode='max',
        patience=5,
        min_delta=0.001,
        verbose=True,
        strict=True
    ))

    callbacks.append(plc.ModelCheckpoint(
        filename='{epoch:03d}-{valid/auc:.3f}',
        save_top_k=0,
        monitor='valid/auc',
        mode='max',
        save_last=True,
        verbose=True,
        auto_insert_metric_name=False,
    ))

    return callbacks


def start(args):
    pl.seed_everything(seed=2022)
    
    data_module = DataInterface(args.data_path, args.sst, args.train_bs, args.test_bs)
    model = Classifier(args.input_dim, args.output_dim, args.dropout, args.activation)
    model_mudule = ModelInterface(model, args)

    trainer = Trainer.from_argparse_args(args, callbacks=load_callbacks())
    trainer.fit(model_mudule, data_module)
    res = trainer.test(model_mudule, data_module)

    return res


def main(args):
    res = start(args)
    return str(res)

def main2():
    parser = argparse.ArgumentParser()

    parser.add_argument('--data_path', type=str, default='../dataset/100k_BS_GraphSAGE_embed-[gender_age_occupation]-Mar-28-2022_16-45-26.pth')
 
    parser.add_argument('--input_dim', type=int, default=64)
    parser.add_argument('--train_bs', type=int, default=5120)
    parser.add_argument('--test_bs', type=int, default=2048)
    parser.add_argument('--weight_decay', type=float, default=1e-4)
    parser.add_argument('--lr', type=float, default=1e-4)
    parser.add_argument('--dropout', type=float, default=0.3)
    parser.add_argument('--activation', type=str, default='leakyrelu')
    # parser.add_argument('--sst', type=str, default='gender')
    # parser.add_argument('--output_dim', type=int, default=1)
    parser.add_argument('--sst', type=str, default='age')
    parser.add_argument('--output_dim', type=int, default=7)
    # parser.add_argument('--sst', type=str, default='occupation')
    # parser.add_argument('--output_dim', type=int, default=21)

    parser = Trainer.add_argparse_args(parser)

    parser.set_defaults(max_epochs=300)
    parser.set_defaults(gpus=[0])
    parser.set_defaults(log_every_n_steps=24)
    parser.set_defaults(check_val_every_n_epoch=10)

    args = parser.parse_args()

    dir_path = '../dataset'
    files = os.listdir(dir_path)
    attrs = {'gender':1, 'age':7, 'occupation':21}
    save_res_path = 'results.txt'
    with open(save_res_path, 'w') as f1:
        for file in files:
            file_path = os.path.join(dir_path, file)
            for sst, output_dim  in attrs.items():
                if sst in file or 'none' in file:
                    args.data_path = file_path
                    args.sst = sst
                    args.output_dim = output_dim
                    f1.write(f'{file}({sst}) : \n\t{main(args)}\n\n')


if __name__ == '__main__':
    os.chdir(sys.path[0])

    main2()

    # parser = argparse.ArgumentParser()

    # parser.add_argument('--data_path', type=str, default='../dataset/100k_BS_GraphSAGE_embed-[gender_age_occupation]-Mar-28-2022_16-45-26.pth')
 
    # parser.add_argument('--input_dim', type=int, default=64)
    # parser.add_argument('--train_bs', type=int, default=5120)
    # parser.add_argument('--test_bs', type=int, default=2048)
    # parser.add_argument('--weight_decay', type=float, default=1e-4)
    # parser.add_argument('--lr', type=float, default=1e-3)
    # parser.add_argument('--dropout', type=float, default=0.3)
    # parser.add_argument('--activation', type=str, default='leakyrelu')
    # # parser.add_argument('--sst', type=str, default='gender')
    # # parser.add_argument('--output_dim', type=int, default=1)
    # # parser.add_argument('--sst', type=str, default='age')
    # # parser.add_argument('--output_dim', type=int, default=7)
    # parser.add_argument('--sst', type=str, default='occupation')
    # parser.add_argument('--output_dim', type=int, default=21)

    # parser = Trainer.add_argparse_args(parser)

    # parser.set_defaults(max_epochs=10)
    # # parser.set_defaults(gpus=[0])
    # parser.set_defaults(log_every_n_steps=24)
    # parser.set_defaults(check_val_every_n_epoch=10)

    # args = parser.parse_args()

    # main(args)
    

