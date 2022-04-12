# -*- coding: utf-8 -*-
# @Time   : 2022/3/14
# @Author : Jiakai Tang
# @Email  : whut_tangjiakai@qq.com

r"""
FATR
################################################
Reference:
    Zhu, Z., X. Hu, and J. Caverlee, "Fairness-Aware Tensor-Based Recommendation“ in CIKM 2018.
"""

import torch
import torch.nn as nn
import torch.nn.functional as F
import scipy.sparse as sp

from recbole.model.abstract_recommender import FairRecommender
from recbole.utils import InputType, ModelType


class FATR(FairRecommender):
    r""" FATR is a novel fairness-aware tensor recommendation framework that is
    designed to maintain quality while dramatically improving fairness

    Here only implement user-side and item-side fairness
    """
    input_type = InputType.POINTWISE
    type = ModelType.TRADITIONAL

    def __init__(self, config, dataset):
        super(FATR, self).__init__(config, dataset)

        self.embedding_size = config['embedding_size']
        self.sst_info = config['sst_info']
        if len(self.sst_info) == 0:
            raise ValueError('sst_info could not be null')
        self.Lambda = config['Lambda']
        self.gamma = config['gamma']
        self.alpha = config['learning_rate']
        self.tol = config['tol']

        self.user_embedding = nn.Embedding(self.n_users, self.embedding_size)
        self.item_embedding = nn.Embedding(self.n_items, self.embedding_size)
        self.sst_max_size = self.get_sst_max_size(dataset)
        self.user_sst_embedding = self.get_user_sst_embedding(dataset)
        self.item_sst_embedding = self.get_item_sst_embedding(dataset)
        self.true_X = torch.tensor(dataset.inter_matrix().todense()).to(self.device)
        self.pre_X = self.true_X

    def get_sst_max_size(self, dataset):
        r"""

        Args:
            dataset(Interaction): dataset in form of Interaction
        Return:
             sst_tot_size(int): maximum size of all sensitive feature
        """
        user_feature = dataset.get_user_feature()
        item_feature = dataset.get_item_feature()
        user_sst_size = 0

        if 'user' in self.sst_info:
            for user_sst in self.sst_info['user']:
                assert user_sst in user_feature, f'{user_sst} not in user features'
                user_sst_size += len(user_feature[user_sst][1:].unique())

        item_sst_size = 0
        if 'item' in self.sst_info:
            for item_sst in self.sst_info['item']:
                assert item_sst in item_feature, f'{item_sst} not in item features'
                item_sst_size += len(item_sst[item_sst][1:].unique())

        return max(user_sst_size, item_sst_size)

    def get_user_sst_embedding(self, dataset):
        sst_embedding = torch.zeros(self.n_users, self.sst_tot_size)
        if 'user' not in self.sst_info:
            return nn.Embedding.from_pretrained(sst_embedding)
        user_feature = dataset.get_user_feature()
        sst_idx_dict = {}
        idx = 0
        for user_sst in self.sst_info['user']:
            assert user_sst in user_feature, f'{user_sst} not in user features'
            sst_set = user_feature[user_sst][1:].unique()
            sst_idx_dict[user_sst] = {}
            for s in sst_set:
                sst_idx_dict[user_sst][s.item()] = idx
                idx += 1
        for user in range(1, self.n_users, 1):
            for user_sst in self.sst_info['user']:
                s = user_feature[user_sst][user]
                i = sst_idx_dict[user_sst][s.item()]
                sst_embedding.data[user][i] = 1
        sst_embedding = nn.Embedding.from_pretrained(sst_embedding)

        return sst_embedding

    def get_item_sst_embedding(self, dataset):
        sst_embedding = torch.zeros(self.n_items, self.sst_tot_size)
        if 'item' not in self.sst_info:
            return nn.Embedding.from_pretrained(sst_embedding)
        item_feature = dataset.get_item_feature()
        sst_idx_dict = {}
        idx = 0
        for item_sst in self.sst_info['item']:
            assert item_sst in item_feature, f'{item_sst} not in item features'
            sst_set = item_feature[item_sst][1:].unique()
            sst_idx_dict[item_sst] = {}
            for s in sst_set:
                sst_idx_dict[item_sst][s.item()] = idx
                idx += 1
        for item in range(1, self.n_items + 1, 1):
            feature = item_feature[item]
            for item_sst in self.sst_info['item']:
                s = feature[item_sst]
                i = sst_idx_dict[item_sst][s.item()]
                sst_embedding.data[item][i] = 1
        sst_embedding = nn.Embedding.from_pretrained(sst_embedding)

        return sst_embedding

    def forward(self, user, item):
        user_embed_1 = self.user_embedding(user)
        user_embed_2 = self.user_sst_embedding(user)
        user_embed = torch.cat([user_embed_1, user_embed_2], dim=-1)

        item_embed_1 = self.item_embedding(item)
        item_embed_2 = self.item_sst_embedding(item)
        item_embed = torch.cat([item_embed_1, item_embed_2], dim=-1)

        return F.sigmoid((user_embed * item_embed).sum(dim=-1))

    def update_unsst_embedding(self, X_m, A_embedding, A_sst_embedding,
                               B_embedding, B_sst_embedding):
        r""" update non-sensitive-mode matrix by ALS algorithm

        Args:
            X_m(torch.tensor): Mode-n unfolding matrix of tensor true_X
            A_embedding(nn.Embedding): Embedding of un-sensitive dimensions which will be update
            A_sst_embedding(nn.Embedding): Embedding of sensitive dimensions which will be update
            B_embedding(nn.Embedding): Khatri-Rao product Embedding of un-sensitive dimensions which will not be update
            B_sst_embedding(nn.Embedding): Khatri-Rao product Embedding of sensitive dimensions which will not be update
        """
        X_m = X_m.float()
        A_k = torch.cat((B_embedding.weight.data, B_sst_embedding.weight.data), dim=-1)
        I = torch.eye(A_k.shape[-1]).to(self.device)
        newA_m = X_m.matmul(A_k).matmul(torch.linalg.pinv(self.gamma * I + A_k.t().matmul(A_k)))
        A_embedding.weight.data = newA_m[:, :self.embedding_size]
        A_sst_embedding.weight.data = newA_m[:, self.embedding_size:]

    def update_sst_embedding(self, X_n, A_embedding, A_sst_embedding,
                         B_embedding, B_sst_embedding):
        r""" update sensitive-mode matrix by ALS algorithm

        Args:
            X_n(torch.tensor): Mode-n unfolding matrix of tensor true_X
            A_embedding(nn.Embedding): Embedding of un-sensitive dimensions which will be update
            A_sst_embedding(nn.Embedding): Embedding of sensitive dimensions which will be update
            B_embedding(nn.Embedding): Khatri-Rao product Embedding of un-sensitive dimensions which will not be update
            B_sst_embedding(nn.Embedding): Khatri-Rao product Embedding of sensitive dimensions which will not be update
        """
        X_n = X_n.float()
        A_n_pp = A_sst_embedding.weight.data
        B_n_pp = B_sst_embedding.weight.data
        B_n_p = B_embedding.weight.data
        I = torch.eye(self.embedding_size).to(self.device)
        A_embedding.weight.data = (X_n - A_n_pp.matmul(B_n_pp.t())).matmul(
            B_n_p).matmul(torch.linalg.pinv(self.gamma * I + B_n_p.t().matmul(B_n_p)))

    def update_sst_sgd(self, embedding, sst_embedding):
        r""" update sensitive dimensions for sensitive-mode matrix by SGD algorithm

        Args:
            embedding(nn.Embedding): Embedding of un-sensitive dimensions which will be update
            sst_embedding(nn.Embedding): Embedding of sensitive dimensions which will be update
        Return:
        """
        A_n_pp = sst_embedding.weight.data
        A_n_p = embedding.weight.data
        deriv = self.Lambda * A_n_pp.matmul(A_n_pp.t()).matmul(A_n_p)
        embedding.weight.data -= self.alpha * deriv

    def calculate_loss(self, interaction):
        coo_matrix = sp.coo_matrix(self.true_X.cpu())
        user = torch.LongTensor(coo_matrix.row).to(self.device)
        item = torch.LongTensor(coo_matrix.col).to(self.device)
        pred_scores = self.forward(user, item)

        loss = float('inf')

        while loss >= self.tol:
            if 'user' in self.sst_info:
                self.update_sst_embedding(self.true_X, self.user_embedding, self.user_sst_embedding,
                                          self.item_embedding, self.item_sst_embedding)
            else:
                self.update_unsst_embedding(self.true_X,self.user_embedding, self.user_sst_embedding,
                                            self.item_embedding, self.item_sst_embedding)

            if 'item' in self.sst_info:
                self.update_sst_embedding(self.true_X.t(), self.item_embedding, self.item_sst_embedding,
                                          self.user_embedding, self.user_sst_embedding)
            else:
                self.update_unsst_embedding(self.true_X.t(), self.item_embedding, self.item_sst_embedding,
                                            self.user_embedding, self.user_sst_embedding)

            if 'user' in self.sst_info:
                self.update_sst_sgd(self.user_embedding, self.user_sst_embedding)
            if 'item' in self.sst_info:
                self.update_sst_sgd(self.item_embedding, self.item_sst_embedding)

            X = sp.coo_matrix((pred_scores.cpu().detach().numpy(),\
                               (user.cpu().detach().numpy(), item.cpu().detach().numpy())),\
                              shape=self.pre_X.shape).todense()
            X = torch.tensor(X).to(self.device)
            loss = torch.norm(X - self.pre_X) / torch.norm(self.pre_X)
            self.pre_X = X

        return torch.nn.Parameter(torch.zeros(1))

    def predict(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        user_embed = self.user_embedding(user)
        item_embed = self.item_embedding(item)

        return (user_embed * item_embed).sum(dim=-1)

    def full_sort_predict(self, interaction):
        user = interaction[self.ITEM_ID]
        user_embed = self.user_embedding(user)
        item_embed = self.item_embedding.weight

        return torch.matmul(user_embed, item_embed.t())


