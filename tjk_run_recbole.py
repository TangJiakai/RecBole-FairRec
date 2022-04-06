# @Time   : 2020/10/6
# @Author : Shanlei Mu
# @Email  : slmu@ruc.edu.cn

"""
recbole.quick_start
########################
"""
import logging
from logging import getLogger

import torch
import pickle
import sys
import os

from recbole.config import Config
from recbole.data import create_dataset, data_preparation, save_split_dataloaders, load_split_dataloaders
from recbole.utils import init_logger, get_model, get_trainer, init_seed, logger, set_color


def run_recbole(model=None, dataset=None, config_file_list=None, config_dict=None, saved=True):
    r""" A fast running api, which includes the complete process of
    training and testing a model on a specified dataset

    Args:
        model (str, optional): Model name. Defaults to ``None``.
        dataset (str, optional): Dataset name. Defaults to ``None``.
        config_file_list (list, optional): Config files used to modify experiment parameters. Defaults to ``None``.
        config_dict (dict, optional): Parameters dictionary used to modify experiment parameters. Defaults to ``None``.
        saved (bool, optional): Whether to save the model. Defaults to ``True``.
    """
    # configurations initialization
    # logger initialization

    config, model, dataset, train_data, valid_data, test_data = load_data_and_model(
        model_file='saved/saved_FairGo_PMF/FairGo_PMF-Apr-04-2022_19-16-23.pth',
        dataset_file='saved/saved_FairGo_PMF/ml-1M-dataset.pth',
        dataloader_file='saved/saved_FairGo_PMF/ml-1M-for-FairGo_PMF-dataloader.pth',
    )

    init_seed(config['seed'], config['reproducibility'])

    init_logger(config)
    logger = getLogger()

    logger.info(config)
    logger.info(dataset)
    logger.info(model)

    # trainer loading and initialization
    trainer = get_trainer(config['MODEL_TYPE'], config['model'])(config, model)
    trainer.resume_checkpoint(resume_file='saved/saved_FairGo_PMF/FairGo_PMF-Apr-04-2022_19-16-23.pth')

    # trainer._save_sst_embed(train_data)

    # model training
    # best_valid_score, best_valid_result = trainer.fit(
    #     train_data, valid_data, saved=saved, show_progress=config['show_progress']
    # )

    # model evaluation
    test_result = trainer.evaluate(test_data, load_best_model=saved, show_progress=config['show_progress'])

    # logger.info(set_color('best valid ', 'yellow') + f': {best_valid_result}')
    logger.info(set_color('test result', 'yellow') + f': {test_result}')

    # return {
    #     'best_valid_score': best_valid_score,
    #     'valid_score_bigger': config['valid_metric_bigger'],
    #     'best_valid_result': best_valid_result,
    #     'test_result': test_result
    # }


def load_data_and_model(model_file, dataset_file=None, dataloader_file=None):
    r"""Load filtered dataset, split dataloaders and saved model.

    Args:
        model_file (str): The path of saved model file.
        dataset_file (str, optional): The path of filtered dataset. Defaults to ``None``.
        dataloader_file (str, optional): The path of split dataloaders. Defaults to ``None``.

    Note:
        The :attr:`dataset` will be loaded or created according to the following strategy:
        If :attr:`dataset_file` is not ``None``, the :attr:`dataset` will be loaded from :attr:`dataset_file`.
        If :attr:`dataset_file` is ``None`` and :attr:`dataloader_file` is ``None``,
        the :attr:`dataset` will be created according to :attr:`config`.
        If :attr:`dataset_file` is ``None`` and :attr:`dataloader_file` is not ``None``,
        the :attr:`dataset` will neither be loaded or created.

        The :attr:`dataloader` will be loaded or created according to the following strategy:
        If :attr:`dataloader_file` is not ``None``, the :attr:`dataloader` will be loaded from :attr:`dataloader_file`.
        If :attr:`dataloader_file` is ``None``, the :attr:`dataloader` will be created according to :attr:`config`.

    Returns:
        tuple:
            - config (Config): An instance object of Config, which record parameter information in :attr:`model_file`.
            - model (AbstractRecommender): The model load from :attr:`model_file`.
            - dataset (Dataset): The filtered dataset.
            - train_data (AbstractDataLoader): The dataloader for training.
            - valid_data (AbstractDataLoader): The dataloader for validation.
            - test_data (AbstractDataLoader): The dataloader for testing.
    """
    checkpoint = torch.load(model_file)
    config = checkpoint['config']
    init_logger(config)

    dataset = None
    if dataset_file:
        with open(dataset_file, 'rb') as f:
            dataset = pickle.load(f)

    if dataloader_file:
        train_data, valid_data, test_data = load_split_dataloaders(config)
    else:
        if dataset is None:
            dataset = create_dataset(config)
        train_data, valid_data, test_data = data_preparation(config, dataset)

    model = get_model(config['model'])(config, train_data.dataset).to(config['device'])
    model.load_state_dict(checkpoint['state_dict'])
    model.load_other_parameter(checkpoint.get('other_parameter'))

    return config, model, dataset, train_data, valid_data, test_data


if __name__ == '__main__':
    os.chdir(sys.path[0])
    results = run_recbole()


