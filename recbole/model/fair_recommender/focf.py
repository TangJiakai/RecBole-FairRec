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
import torch.nn.functional as F
import numpy as np

from recbole.model.abstract_recommender import FairRecommender
from recbole.model.loss import EmbLoss
from recbole.utils import InputType


class FOCF(FairRecommender):
    r""" FOCF is fair-aware recommendation model by adding fairness regulation

    Base recommendation model is MF
    """

    input_type = InputType.POINTWISE

    def __init__(self, config, dataset):
        super(FOCF, self).__init__(config, dataset)

        # load dataset info
        self.embedding_size = config['embedding_size']
        self.RATING = config['RATING_FIELD']
        self.SST_FIELD = config['SST_FIELD']
        self.regular_weight = config['regular_weight']
        self.require_pow = config['require_pow']

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
        self.rating_loss_fun = nn.MSELoss()
        self.regular_loss_fun = EmbLoss()
        self.fair_loss_fun = self.get_loss_fun(config['fair_objective'])

        # parameters initialization
        self.all_item_embeddings = None
        self.other_parameter_name = ['all_item_embeddings']

    def get_loss_fun(self, fair_objective):
        fair_objective = fair_objective.strip().lower()

        if fair_objective == 'none':
            return None
        elif fair_objective == 'value':
            return self.value_unfairness
        elif fair_objective == 'absolute':
            return self.absolute_unfairness
        elif fair_objective == 'under':
            return self.under_unfairness
        elif fair_objective == 'over':
            return self.over_unfairness
        elif fair_objective == 'nonparity':
            return self.nonparity_unfairness
        else:
            raise ValueError("you must set config['fair_objective'] be one of (none,"
                             "value,absolute,under,over,nonparity)")

    def get_item_ratings(self, pred_scores, interaction):
        avg_pred_list1, avg_true_list1, avg_pred_list2, avg_true_list2 = [], [], [], []

        sst_unique_value, sst_indices = torch.unique(interaction[self.SST_FIELD], return_inverse=True)
        iid_unique_value, iid_indices = torch.unique(interaction[self.ITEM_ID], return_inverse=True)
        sst1_index = sst_indices == sst_unique_value[0]
        sst2_index = sst_indices == sst_unique_value[1]
        for iid in iid_unique_value:
            pred_score1 = pred_scores[(iid_indices == iid) * sst1_index]
            true1 = interaction[self.RATING][(iid_indices == iid) * sst1_index]
            avg_pred1 = 0
            avg_true1 = 0
            if len(pred_score1) > 0:
                avg_pred1 = pred_score1.mean()
                avg_true1 = true1.mean()
            avg_pred_list1.append(avg_pred1)
            avg_true_list1.append(avg_true1)
            pred_score2 = pred_scores[(iid_indices == iid) * sst2_index]
            true2 = interaction[self.RATING][(iid_indices == iid) * sst2_index]
            avg_pred2 = 0
            avg_true2 = 0
            if len(pred_score2) > 0:
                avg_pred2 = pred_score2.mean()
                avg_true2 = true2.mean()
            avg_pred_list2.append(avg_pred2)
            avg_true_list2.append(avg_true2)

        return avg_pred_list1, avg_true_list1, avg_pred_list2, avg_true_list2

    def value_unfairness(self, pred_scores, interaction):
        avg_pred_list1, avg_true_list1, avg_pred_list2, avg_true_list2 = \
            self.get_item_ratings(pred_scores, interaction)

        loss_input = []
        for pred1, true1, pred2, true2 in zip(avg_pred_list1, avg_true_list1,
                                              avg_pred_list2, avg_true_list2):
            diff = (pred1 - true1) - (pred2 - true2)
            loss_input.append(diff)
        loss_input = torch.tensor(loss_input, device=self.device)
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def absolute_unfairness(self, pred_scores, interaction):
        avg_pred_list1, avg_true_list1, avg_pred_list2, avg_true_list2 = \
            self.get_item_ratings(pred_scores, interaction)

        loss_input = []
        for pred1, true1, pred2, true2 in zip(avg_pred_list1, avg_true_list1,
                                              avg_pred_list2, avg_true_list2):
            diff = abs(pred1 - true1) - abs(pred2 - true2)
            loss_input.append(diff)
        loss_input = torch.tensor(loss_input, device=self.device)
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def under_unfairness(self, pred_scores, interaction):
        avg_pred_list1, avg_true_list1, avg_pred_list2, avg_true_list2 = \
            self.get_item_ratings(pred_scores, interaction)

        loss_input = []
        for pred1, true1, pred2, true2 in zip(avg_pred_list1, avg_true_list1,
                                              avg_pred_list2, avg_true_list2):
            diff = max(0, true1 - pred1) - max(0, true2 - pred2)
            loss_input.append(diff)
        loss_input = torch.tensor(loss_input, device=self.device)
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def over_unfairness(self, pred_scores, interaction):
        avg_pred_list1, avg_true_list1, avg_pred_list2, avg_true_list2 = \
            self.get_item_ratings(pred_scores, interaction)

        loss_input = []
        for pred1, true1, pred2, true2 in zip(avg_pred_list1, avg_true_list1,
                                              avg_pred_list2, avg_true_list2):
            diff = max(0, pred1 - true1) - max(0, pred2 - true2)
            loss_input.append(diff)
        loss_input = torch.FloatTensor(loss_input, device=self.device)
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def nonparity_unfairness(self, pred_scores, interaction):
        sst_unique_value = torch.unique(interaction[self.SST_FIELD])
        sst1 = sst_unique_value[0]
        sst2 = sst_unique_value[1]
        sst_1_num = (interaction[self.SST_FIELD] == sst1).sum()
        sst_2_num = (interaction[self.SST_FIELD] == sst2).sum()
        avg_score_1 = torch.where(interaction[self.SST_FIELD] == sst1, pred_scores,
                                  torch.FloatTensor([0]).to(self.device)).sum() / sst_1_num
        avg_score_2 = torch.where(interaction[self.SST_FIELD] == sst2, pred_scores,
                                  torch.FloatTensor([0]).to(self.device)).sum() / sst_2_num

        return F.smooth_l1_loss(torch.tensor(avg_score_1,device=self.device), torch.tensor(avg_score_2,device=self.device))

    def forward(self, user, item):

        user_embedding = self.get_user_embedding(user)

        col_indices = self.history_user_id[item].flatten()
        row_indices = torch.arange(item.shape[0]).to(self.device).\
            repeat_interleave(self.history_user_id.shape[1], dim=0)
        matrix_rating = torch.zeros(1).to(self.device).repeat(item.shape[0], self.n_users)
        matrix_rating.index_put_((row_indices, col_indices), self.history_user_value[item].flatten())
        item_embedding = self.item_embedding_layer(matrix_rating)

        pred_scores = torch.mul(user_embedding, item_embedding).sum(dim=-1)

        return pred_scores, user_embedding, item_embedding

    def predict(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        pred_scores, _, _ = self.forward(user, item)

        return pred_scores

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
        item_matrix = torch.sparse.FloatTensor(i, data, torch.Size(interaction_matrix.shape)).to(self.device).\
            transpose(0, 1)
        item_embedding = torch.sparse.mm(item_matrix, self.item_embedding_layer.weight.t())

        return item_embedding

    def calculate_loss(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        scores = interaction[self.RATING]

        pred_scores, user_embeddings, item_embeddings = self.forward(user, item)
        rating_loss = self.rating_loss_fun(pred_scores, scores)
        regular_loss = self.regular_loss_fun(user_embeddings, item_embeddings, require_pow=self.require_pow)
        fair_loss = 0.
        if self.fair_loss_fun:
            fair_loss = self.fair_loss_fun(pred_scores, interaction)

        # rating loss + regularization loss + fair objective loss
        loss = rating_loss + self.regular_weight * regular_loss + fair_loss

        return loss

    def full_sort_predict(self, interaction):
        user = interaction[self.USER_ID]
        user_embedding = self.get_user_embedding(user)

        if self.all_item_embeddings is None:
            self.all_item_embeddings = self.get_item_embedding()

        pred_scores = torch.matmul(user_embedding, self.all_item_embeddings.t()).view(-1)

        return pred_scores


