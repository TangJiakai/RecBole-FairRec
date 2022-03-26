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
        self.fair_weight = config['fair_weight']
        self.require_pow = config['require_pow']

        # define layers and loss
        self.user_embedding_layer = nn.Embedding(self.n_users, self.embedding_size)
        self.item_embedding_layer = nn.Embedding(self.n_items, self.embedding_size)
        self.rating_loss_fun = nn.MSELoss()
        self.regular_loss_fun = EmbLoss()
        self.fair_loss_fun = self.get_loss_fun(config['fair_objective'])

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

        loss_input = torch.zeros(len(avg_pred_list1), device=self.device)
        for i, (pred1, true1, pred2, true2) in enumerate(zip(avg_pred_list1, avg_true_list1,
                                              avg_pred_list2, avg_true_list2)):
            diff = (pred1 - true1) - (pred2 - true2)
            loss_input[i] = diff
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def absolute_unfairness(self, pred_scores, interaction):
        avg_pred_list1, avg_true_list1, avg_pred_list2, avg_true_list2 = \
            self.get_item_ratings(pred_scores, interaction)

        loss_input = torch.zeros(len(avg_pred_list1), device=self.device)
        for i, (pred1, true1, pred2, true2) in enumerate(zip(avg_pred_list1, avg_true_list1,
                                              avg_pred_list2, avg_true_list2)):
            diff = abs(pred1 - true1) - abs(pred2 - true2)
            loss_input[i] = diff
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def under_unfairness(self, pred_scores, interaction):
        avg_pred_list1, avg_true_list1, avg_pred_list2, avg_true_list2 = \
            self.get_item_ratings(pred_scores, interaction)

        loss_input = torch.zeros(len(avg_pred_list1), device=self.device)
        for i, (pred1, true1, pred2, true2) in enumerate(zip(avg_pred_list1, avg_true_list1,
                                              avg_pred_list2, avg_true_list2)):
            diff = max(0, true1 - pred1) - max(0, true2 - pred2)
            loss_input[i] = diff
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def over_unfairness(self, pred_scores, interaction):
        avg_pred_list1, avg_true_list1, avg_pred_list2, avg_true_list2 = \
            self.get_item_ratings(pred_scores, interaction)

        loss_input = torch.zeros(len(avg_pred_list1), device=self.device)
        for i, (pred1, true1, pred2, true2) in enumerate(zip(avg_pred_list1, avg_true_list1,
                                              avg_pred_list2, avg_true_list2)):
            diff = max(0, pred1 - true1) - max(0, pred2 - true2)
            loss_input[i] = diff
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def nonparity_unfairness(self, pred_scores, interaction):
        sst_unique_value = torch.unique(interaction[self.SST_FIELD])
        sst1 = sst_unique_value[0]
        sst2 = sst_unique_value[1]
        sst_1_num = (interaction[self.SST_FIELD] == sst1).sum()
        sst_2_num = (interaction[self.SST_FIELD] == sst2).sum()
        avg_score_1 = torch.where(interaction[self.SST_FIELD] == sst1, pred_scores,
                                  torch.tensor([0], dtype=torch.float32, device=self.device)).sum() / sst_1_num
        avg_score_2 = torch.where(interaction[self.SST_FIELD] == sst2, pred_scores,
                                  torch.tensor([0], dtype=torch.float32, device=self.device)).sum() / sst_2_num

        return F.smooth_l1_loss(avg_score_1, avg_score_2)

    def forward(self, user, item):

        user_embedding = self.user_embedding_layer(user)
        item_embedding = self.item_embedding_layer(item)
        pred_scores = torch.mul(user_embedding, item_embedding).sum(dim=-1)

        return pred_scores, user_embedding, item_embedding

    def predict(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        pred_scores, _, _ = self.forward(user, item)

        return pred_scores

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
        loss = rating_loss + self.regular_weight * regular_loss + self.fair_weight * fair_loss

        return loss

    def full_sort_predict(self, interaction):
        user = interaction[self.USER_ID]
        user_embedding = self.user_embedding_layer(user)
        all_item_embeddings = self.item_embedding_layer.weight

        pred_scores = torch.matmul(user_embedding, all_item_embeddings.t()).view(-1)

        return pred_scores


