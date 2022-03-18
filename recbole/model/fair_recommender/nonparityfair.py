# _*_ coding: utf-8 _*_
# @Time   : 2022/3/8
# @Author : Jiakai Tang
# @Email  : whut_tangjiakai@qq.com

r"""
NonParityFair
################################################
Reference:
Yao, S. and B. Huang, "Beyond parity: fairness objectives for collaborative filtering." in NIPS. 2017

"""

import torch
import torch.nn as nn
import numpy as np

from recbole.model.abstract_recommender import FairRecommender
from recbole.utils import InputType


class NonParityFair(FairRecommender):
    r""" NonParityFair is fair-aware recommendation model by adding fairness regulation

    Base recommendation model is MF
    """

    input_type = InputType.POINTWISE

    def __init__(self, config, dataset):
        super(NonParityFair, self).__init__(config, dataset)

        # load dataset info
        self.embedding_size = config['embedding_size']
        self.RATING = config['RATING_FIELD']
        self.sst_attr = config['sst_attr']
        self.sst_1, self.sst_2 = self._get_sst_size(dataset.get_user_feature())

        # generate intermediate data
        self.history_user_id, self.history_user_value, _ = dataset.history_user_matrix(value_field=self.RATING)
        self.history_item_id, self.history_item_value, _ = dataset.history_item_matrix(value_field=self.RATING)
        self.interaction_matrix = dataset.inter_matrix(form='csr', value_field=self.RATING).astype(np.float32)
        self.history_user_id = self.history_user_id.to(self.device)
        self.history_user_value = self.history_user_value.to(self.device)
        self.history_item_id = self.history_item_id.to(self.device)
        self.history_item_value = self.history_item_value.to(self.device)

        # define layers and loss
        self.user_embedding_layer = nn.Linear(in_features=self.n_items, out_features=self.embedding_size, bias=False)
        self.item_embedding_layer = nn.Linear(in_features=self.n_users, out_features=self.embedding_size, bias=False)
        self.loss_fun = nn.MSELoss()

        # parameters initialization
        self.all_item_embeddings = None
        self.other_parameter_name = ['all_item_embeddings']

    def _get_sst_size(self, user_feature):
        r""" set size of sensitive arrtibute

        Params:
            user_feature(Interaction): user features
        Return:
            size of sensitive attribute
        """
        try:
            assert self.sst_attr in user_feature.columns
        except AssertionError:
            raise ValueError(f'{self.sst_attr} is not in user feature')

        try:
            assert len(user_feature[self.sst_attr][1:].unique()) == 2
        except AssertionError:
            raise ValueError(f'{self.sst_attr} should be binary attribute')

        return user_feature[self.sst_attr][1:].unique()

    def forward(self, user, item):

        user_embedding = self.get_user_embedding(user)

        col_indices = self.history_user_id[item].flatten()
        row_indices = torch.arange(item.shape[0]).to(self.device). \
            repeat_interleave(self.history_user_id.shape[1], dim=0)
        matrix_rating = torch.zeros(1).to(self.device).repeat(item.shape[0], self.n_users)
        matrix_rating.index_put_((row_indices, col_indices), self.history_user_value[item].flatten())
        item_embedding = self.item_embedding_layer(matrix_rating)

        pred_scores = torch.mul(user_embedding, item_embedding).sum(dim=-1)

        return pred_scores

    def predict(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]

        return self.forward(user, item)

    def get_user_embedding(self, user):
        r"""Get a batch of user's embedding with the user's id and history interaction matrix.

        Args:
            user (torch.LongTensor): The input tensor that contains user's id, shape: [batch_size, ]

        Returns:
            torch.FloatTensor: The embedding tensor of a batch of user, shape: [batch_size, embedding_size]
        """
        col_indices = self.history_item_id[user].flatten()
        row_indices = torch.arange(user.shape[0]).to(self.device)
        row_indices = row_indices.repeat_interleave(self.history_item_id.shape[1], dim=0)
        matrix_rating = torch.zeros(1).to(self.device).repeat(user.shape[0], self.n_items)
        matrix_rating.index_put_((row_indices, col_indices), self.history_item_value[user].flatten())
        user_embedding = self.user_embedding_layer(matrix_rating)

        return user_embedding

    def get_item_embedding(self):
        r"""Get all item's embedding with history interaction matrix.

        Considering the RAM of device, we use matrix multiply on sparse tensor for generalization.

        Returns:
            torch.FloatTensor: The embedding tensor of all item, shape: [n_items, embedding_size]
        """
        interaction_matrix = self.interaction_matrix.tocoo()
        row = interaction_matrix.row
        col = interaction_matrix.col
        i = torch.LongTensor([row, col])
        data = torch.FloatTensor(interaction_matrix.data)
        item_matrix = torch.sparse.FloatTensor(i, data, torch.Size(interaction_matrix.shape)).to(self.device). \
            transpose(0, 1)
        item_embedding = torch.sparse.mm(item_matrix, self.item_embedding_layer.weight.t())

        return item_embedding

    def calculate_loss(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        scores = interaction[self.RATING]

        pred_scores = self.forward(user, item)
        sst_1_num = (interaction[self.sst_attr]==self.sst_1).sum()
        sst_2_num = (interaction[self.sst_attr]==self.sst_2).sum()
        avg_score_1 = torch.where(interaction[self.sst_attr]==self.sst_1, pred_scores, torch.FloatTensor([0]).to(self.device)).sum()/sst_1_num
        avg_score_2 = torch.where(interaction[self.sst_attr]==self.sst_2, pred_scores, torch.FloatTensor([0]).to(self.device)).sum()/sst_2_num

        loss = self.loss_fun(pred_scores, scores) + torch.abs(avg_score_1 - avg_score_2)

        return loss

    def full_sort_predict(self, interaction):
        user = interaction[self.USER_ID]
        user_embedding = self.get_user_embedding(user)

        if self.all_item_embeddings is None:
            self.all_item_embeddings = self.get_item_embedding()

        pred_scores = torch.matmul(user_embedding, self.all_item_embeddings.t()).view(-1)

        return pred_scores


