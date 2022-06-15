# _*_ coding: utf-8 _*_
# @Time   : 2022/3/8
# @Author : Jiakai Tang
# @Email  : whut_tangjiakai@qq.com

r"""
FOCF
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
    r""" FOCF is a fair-aware recommendation model by adding fairness regulation

    Base recommendation model is MF
    """

    input_type = InputType.POINTWISE

    def __init__(self, config, dataset):
        super(FOCF, self).__init__(config, dataset)

        # load dataset info
        self.embedding_size = config['embedding_size']
        self.RATING = config['RATING_FIELD']
        self.SST_FIELD = config['sst_attr_list'][0]
        self.fair_weight = config['fair_weight']
        self.max_rating = dataset.inter_feat[self.RATING].max()

        # define layers and loss
        self.user_embedding_layer = nn.Embedding(self.n_users, self.embedding_size)
        self.item_embedding_layer = nn.Embedding(self.n_items, self.embedding_size)
        self.rating_loss_fun = nn.MSELoss()
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

        sst_unique_value, sst_inverse = torch.unique(interaction[self.SST_FIELD], return_inverse=True)
        iid_unique_value, iid_inverse = torch.unique(interaction[self.ITEM_ID], return_inverse=True)
        iid_unique_len = len(iid_unique_value)
        interaction_len = len(pred_scores)
        avg_pred_list = torch.zeros((iid_unique_len,2), device=self.device)
        sst_num = torch.zeros((iid_unique_len,2), device=self.device)
        avg_true_list = torch.zeros((iid_unique_len,2), device=self.device)

        index = (iid_inverse, sst_inverse)
        avg_pred_list.index_put_(index, pred_scores, accumulate=True)
        avg_true_list.index_put_(index, interaction[self.RATING], accumulate=True)
        sst_num.index_put_(index, torch.ones(interaction_len, device=self.device), accumulate=True)
        sst_num += 1e-5

        return avg_pred_list/sst_num, avg_true_list/sst_num

    def value_unfairness(self, pred_scores, interaction):
        avg_pred_list, avg_true_list = self.get_item_ratings(pred_scores, interaction)
        diff = avg_pred_list - avg_true_list 
        loss_input = torch.abs(diff[:,0]- diff[:,1])
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def absolute_unfairness(self, pred_scores, interaction):
        avg_pred_list, avg_true_list = self.get_item_ratings(pred_scores, interaction)
        diff = torch.abs(avg_pred_list - avg_true_list)
        loss_input = torch.abs(diff[:,0]- diff[:,1])
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def under_unfairness(self, pred_scores, interaction):
        avg_pred_list, avg_true_list = self.get_item_ratings(pred_scores, interaction)
        zero_tensor = torch.tensor(0., dtype=torch.float32, device=self.device)
        diff = torch.where((avg_true_list - avg_pred_list)>zero_tensor, avg_true_list - avg_pred_list, zero_tensor)
        loss_input = torch.abs(diff[:,0]- diff[:,1])
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def over_unfairness(self, pred_scores, interaction):
        avg_pred_list, avg_true_list = self.get_item_ratings(pred_scores, interaction)
        zero_tensor = torch.tensor(0., dtype=torch.float32, device=self.device)
        diff = torch.where((avg_pred_list - avg_true_list)>zero_tensor, avg_pred_list - avg_true_list, zero_tensor)
        loss_input = torch.abs(diff[:,0]- diff[:,1])
        loss_target = torch.zeros_like(loss_input, device=self.device)

        return F.smooth_l1_loss(loss_input, loss_target)

    def nonparity_unfairness(self, pred_scores, interaction):
        sst_unique_value = torch.unique(interaction[self.SST_FIELD])
        sst1 = sst_unique_value[0]
        sst2 = sst_unique_value[1]
        avg_score_1 = pred_scores[interaction[self.SST_FIELD] == sst1].mean()
        avg_score_2 = pred_scores[interaction[self.SST_FIELD] == sst2].mean()
 
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

    def calculate_loss(self, interaction):
        users = interaction[self.USER_ID]
        items = interaction[self.ITEM_ID]
        scores = interaction[self.RATING]

        pred_scores, user_embeddings, item_embeddings = self.forward(users, items)
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

        user_embed = self.user_embedding_layer(user)
        all_item_embed = self.item_embedding_layer.weight
        pred_scores = torch.mm(user_embed, all_item_embed.t()).view(-1)

        return torch.clamp(pred_scores, min=0., max=self.max_rating) / self.max_rating


