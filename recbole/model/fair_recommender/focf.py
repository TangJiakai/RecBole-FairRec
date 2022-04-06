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
from recbole.model.init import xavier_normal_initialization

from recbole.model.abstract_recommender import FairRecommender
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
        # self.LABEL = config['LABEL_FIELD']
        self.SST_FIELD = config['sst_attr_list'][0]
        self.fair_weight = config['fair_weight']
        self.max_rating = dataset.inter_feat[self.RATING].max()

        # define layers and loss
        self.user_embedding_layer = nn.Embedding(self.n_users, self.embedding_size)
        self.item_embedding_layer = nn.Embedding(self.n_items, self.embedding_size)
        self.rating_loss_fun = nn.MSELoss()
        # self.sigmoid = nn.Sigmoid()
        # self.rec_loss_fun = nn.BCELoss()
        self.fair_loss_fun = self.get_loss_fun(config['fair_objective'])

        self.apply(xavier_normal_initialization)

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
    def get_average_score(self, scores):
        res_score = 0.
        if len(scores) > 0:
            res_score = scores.mean()

        return res_score

    def get_item_ratings(self, pred_scores, interaction):

        sst_unique_value, sst_indices = torch.unique(interaction[self.SST_FIELD], return_inverse=True)
        iid_unique_value, iid_indices = torch.unique(interaction[self.ITEM_ID], return_inverse=True)
        avg_pred_list1 = torch.zeros(len(iid_indices), device=self.device)
        avg_pred_list2 = torch.zeros(len(iid_indices), device=self.device)

        sst1_index = sst_indices == 0
        sst2_index = sst_indices == 1
        avg_true_score1 = self.get_average_score(pred_scores[sst1_index])
        avg_true_score2 = self.get_average_score(pred_scores[sst2_index])

        for i in range(len(iid_unique_value)):
            indices = iid_indices==i
            avg_pred_list1[i] = self.get_average_score(pred_scores[indices*sst1_index])
            avg_pred_list2[i] = self.get_average_score(pred_scores[indices*sst2_index])

        return avg_pred_list1, avg_true_score1, avg_pred_list2, avg_true_score2

    def value_unfairness(self, pred_scores, interaction):
        avg_pred_list1, avg_true_score1, avg_pred_list2, avg_true_score2 = self.get_item_ratings(pred_scores, interaction)
        diff1, diff2 = avg_pred_list1 - avg_true_score1, avg_pred_list2 - avg_true_score2 
        loss_input = torch.abs(diff1 - diff2)
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def absolute_unfairness(self, pred_scores, interaction):
        avg_pred_list1, avg_true_score1, avg_pred_list2, avg_true_score2 = self.get_item_ratings(pred_scores, interaction)
        diff1, diff2 = avg_pred_list1 - avg_true_score1, avg_pred_list2 - avg_true_score2 
        loss_input = torch.abs(torch.abs(diff1) - torch.abs(diff2))
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def under_unfairness(self, pred_scores, interaction):
        avg_pred_list1, avg_true_score1, avg_pred_list2, avg_true_score2 = self.get_item_ratings(pred_scores, interaction)
        diff1, diff2 = avg_true_score1 - avg_pred_list1, avg_true_score2 - avg_pred_list2
        loss_input = torch.abs(torch.where(diff1>0, diff1, torch.tensor(0.,device=self.device)) - torch.where(diff2>0, diff2, torch.tensor(0.,device=self.device)))
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def over_unfairness(self, pred_scores, interaction):
        avg_pred_list1, avg_true_score1, avg_pred_list2, avg_true_score2 = self.get_item_ratings(pred_scores, interaction)
        diff1, diff2 = avg_pred_list1 - avg_true_score1, avg_pred_list2 - avg_true_score2 
        loss_input = torch.abs(torch.where(diff1>0, diff1, torch.tensor(0.,device=self.device)) - torch.where(diff2>0, diff2, torch.tensor(0.,device=self.device)))
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def nonparity_unfairness(self, pred_scores, interaction):
        sst_unique_value = torch.unique(interaction[self.SST_FIELD])
        sst1 = sst_unique_value[0]
        sst2 = sst_unique_value[1]
        avg_score_1 = pred_scores[torch.where(interaction[self.SST_FIELD] == sst1)[0]].mean()
        avg_score_2 = pred_scores[torch.where(interaction[self.SST_FIELD] == sst2)[0]].mean()
 
        return F.smooth_l1_loss(avg_score_1, avg_score_2)

    def forward(self, user, item):

        user_embedding = self.user_embedding_layer(user)
        item_embedding = self.item_embedding_layer(item)
        pred_scores = torch.mul(user_embedding, item_embedding).sum(dim=-1)
        # pred_scores = self.sigmoid(torch.mul(user_embedding, item_embedding).sum(dim=-1))

        return pred_scores, user_embedding, item_embedding

    def predict(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        pred_scores, _, _ = self.forward(user, item)

        return torch.clamp(pred_scores, min=0., max=self.max_rating) / self.max_rating
        # return pred_scores

    def calculate_loss(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        scores = interaction[self.RATING]
        # scores = interaction[self.LABEL]

        pred_scores, user_embeddings, item_embeddings = self.forward(user, item)
        rating_loss = self.rating_loss_fun(pred_scores, scores)
        # rec_loss = self.rec_loss_fun(pred_scores, scores)

        fair_loss = 0.
        if self.fair_loss_fun:
            fair_loss = self.fair_loss_fun(pred_scores, interaction)

        # rating loss + fair objective loss
        loss = rating_loss + self.fair_weight * fair_loss
        # loss = rec_loss + self.fair_weight * fair_loss

        return loss

    def full_sort_predict(self, interaction):
        user = interaction[self.USER_ID]
        user_embedding = self.user_embedding_layer(user)
        all_item_embeddings = self.item_embedding_layer.weight

        pred_scores = torch.matmul(user_embedding, all_item_embeddings.t()).view(-1)
        # pred_scores = self.sigmoid(torch.matmul(user_embedding, all_item_embeddings.t()).view(-1))

        return torch.clamp(pred_scores, min=0., max=self.max_rating) / self.max_rating
        # return pred_scores


