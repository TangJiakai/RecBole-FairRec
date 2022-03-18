# -*- coding: utf-8 -*-
# @Time   : 2022/3/6
# @Author : Jiakai Tang
# @Email  : whut_tangjiakai@qq.com

r"""
FairGO
################################################
Reference:
    Wu Le et al. "Learning Fair Representations for Recommendation: A Graph-based Perspective." in WWW 2021.
"""

import torch
import torch.nn as nn
from recbole.model.abstract_recommender import FairRecommender
from recbole.model.layers import MLPLayers
from recbole.utils import InputType
import numpy as np
import scipy.sparse as sp


class FairGO(FairRecommender):
    r""" FairGo is a fair-aware model for learning fair graph embeddings that be trained in the pointwise way.

    """
    input_type = InputType.POINTWISE

    def __init__(self, config, dataset):
        super(FairGO, self).__init__(config, dataset)

        # load parameters info
        self.LABEL = config['LABEL_FIELD']
        self.RATING = config['RATING_FIELD']
        self.device = config['device']
        self.n_layers = config['n_layers']
        self.activation = config['activation']
        self.embedding_size = config['embedding_size']
        self.dis_hidden_size_list = config['dis_hidden_size_list']
        self.filter_hidden_size_list = config['filter_hidden_size_list']
        try:
            assert self.filter_hidden_size_list[-1] == self.embedding_size
        except AssertionError:
            raise AssertionError('the last of filter_hidden_size_list should be '
                                 'equal to embedding size')

        # storage variables for full sort evaluation acceleration
        self.restore_user_e = None
        self.restore_item_e = None

        # load dataset info
        self.rating_matrix = dataset.inter_matrix(form='coo', value_field=self.RATING).astype(np.float32)
        user_emb = dataset.get_preload_weight('uid')
        item_emb = dataset.get_preload_weight('iid')

        # define layers and loss
        self.user_embedding = torch.nn.Embedding(self.n_users, self.embedding_size, padding_idx=0)
        self.item_embedding = torch.nn.Embedding(self.n_items, self.embedding_size, padding_idx=0)
        self.user_embedding.weight.data.copy_(torch.from_numpy(user_emb))
        self.item_embedding.weight.data.copy_(torch.from_numpy(item_emb))
        self.dis_layer = MLPLayers(layers=[self.embedding_size] + self.dis_hidden_size_list,
                                   activation=self.activation)
        self.filter_layer = MLPLayers(layers=[self.embedding_size] + self.filter_hidden_size_list,
                                      activation=self.activation)
        self.aggr_layer = MLPLayers(layers=[self.n_layers * self.embedding_size, self.embedding_size],
                                    activation=self.activation)
        self.dis_loss_fun = nn.CrossEntropyLoss()
        self.mse_loss_fun = nn.MSELoss()

        # generate intermediate data
        self.norm_rating_matrix = self.get_norm_rating_matrix().to(self.device)

        self.other_parameter_name = ['restore_user_e', 'restore_item_e']

    def get_norm_rating_matrix(self):
        r""" Get norm rating matrix according training rating matrix

        Return:
            torch.sparse.FloatTensor: The norm rating matrix in form of sparse matrix
        """
        # build rating matrix
        A = sp.dok_matrix((self.n_users + self.n_items, self.n_users + self.n_items), dtype=np.float32)
        rating_M = self.rating_matrix
        rating_M_T = self.rating_matrix.transpose()
        data_dict = dict(zip(zip(rating_M.row, rating_M.col + self.n_users), rating_M.data))
        data_dict.update(dict(zip(zip(rating_M_T.row + self.n_users, rating_M_T.col), rating_M_T.data)))
        A._update(data_dict)
        # norm rating matrix
        sumArr = A.sum(axis=1)
        # add epsilon to avoid divide by zero Warning
        diag = np.array(sumArr.flatten())[0] + 1e-7
        diag = 1.0 / diag
        D = sp.diags(diag)
        L = D * A
        L = sp.coo_matrix(L)
        row = L.row
        col = L.col
        # covert norm rating matrix to tensor
        i = torch.LongTensor([row, col])
        data = torch.FloatTensor(L.data)
        SparseL = torch.sparse.FloatTensor(i, data, torch.Size(L.shape))
        return SparseL

    def get_ego_embeddings(self):
        r"""Get embedding matrix of users and items.

        Returns:
            torch.FloatTensor: The embedding matrix of all users and items, shape: [user_num + item_num, embedding_size]
        """
        user_embeddings = self.user_embedding.weight
        item_embeddings = self.item_embedding.weight
        ego_embeddings = torch.cat([user_embeddings, item_embeddings], dim=0)
        return ego_embeddings

    def forward(self):
        all_embedding = self.get_ego_embeddings()
        all_embedding = self.filter_layer(all_embedding)
        user_all_embeddings, item_all_embeddings = torch.split(all_embedding, [self.n_users, self.n_items])

        return user_all_embeddings, item_all_embeddings

    def calculate_loss(self, interaction):
        if self.restore_user_e is not None or self.restore_item_e is not None:
            self.restore_user_e, self.restore_item_e = None, None

        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        rating = interaction[self.RATING]

        user_all_embeddings, item_all_embeddings = self.forward()
        user_embeddings = user_all_embeddings[user]
        item_embeddings = item_all_embeddings[item]

        pred_ratings = (user_embeddings * item_embeddings).sum(dim=-1)
        mse_loss = self.mse_loss_fun(pred_ratings, rating)
        # fair_loss = self.fair_weight * self.calculate_dis_loss(interaction)

        return mse_loss

    def calculate_dis_loss(self, interaction):
        r""" Calculate loss of discriminator

        """
        if self.restore_user_e is not None or self.restore_item_e is not None:
            self.restore_user_e, self.restore_item_e = None, None

        user = interaction[self.USER_ID]
        label = interaction[self.LABEL].long()

        user_all_embeddings, item_all_embeddings = self.forward()
        user_node_embedding = user_all_embeddings[user]
        all_embeddings = torch.cat([user_all_embeddings, item_all_embeddings], dim=0)
        graph_embedding_list = []
        for _ in range(self.n_layers):
            all_embeddings = torch.sparse.mm(self.norm_rating_matrix, all_embeddings)
            graph_embedding_list.append(all_embeddings)

        all_graph_embeddings = self.aggr_layer(torch.cat(graph_embedding_list, dim=1))
        user_all_graph_embeddings, _ = torch.split(all_graph_embeddings, [self.n_users, self.n_items])
        user_graph_embedding = user_all_graph_embeddings[user]

        node_dis_loss = self.dis_loss_fun(self.dis_layer(user_node_embedding), label)
        graph_dis_loss = self.dis_loss_fun(self.dis_layer(user_graph_embedding), label)

        return node_dis_loss + graph_dis_loss

    def predict(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]

        user_all_embeddings, item_all_embeddings = self.forward()

        u_embeddings = user_all_embeddings[user]
        i_embeddings = item_all_embeddings[item]
        scores = torch.mul(u_embeddings, i_embeddings).sum(dim=1)
        return scores

    def full_sort_predict(self, interaction):
        user = interaction[self.USER_ID]
        if self.restore_user_e is None or self.restore_item_e is None:
            self.restore_user_e, self.restore_item_e = self.forward()
        # get user embedding from storage variable
        u_embeddings = self.restore_user_e[user]
        # dot with all item embedding to accelerate
        pred_ratings = torch.matmul(u_embeddings, self.restore_item_e.transpose(0, 1))

        return pred_ratings.view(-1)
