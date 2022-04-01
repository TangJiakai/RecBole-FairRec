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
from recbole.model.layers import MLPLayers, activation_layer
from recbole.utils import InputType
import numpy as np
import scipy.sparse as sp
from torch_geometric.nn import GCN

class FairGo_GCN(FairRecommender):
    r""" FairGo is a fair-aware model for learning fair graph embeddings that be trained in the pointwise way.

    """
    input_type = InputType.POINTWISE

    def __init__(self, config, dataset):
        super(FairGo_GCN, self).__init__(config, dataset)

        # load parameters info
        self.RATING = config['RATING_FIELD']
        self.n_layers = config['n_layers']
        self.act = config['activation']
        self.embedding_size = config['embedding_size']
        self.dis_hidden_size_list = config['dis_hidden_size_list']
        self.filter_hidden_size_list = config['filter_hidden_size_list']
        self.sst_attrs = config['sst_attr_list']
        self.fair_weight = config['fair_weight']
        self.load_pretrain_weight = config['load_pretrain_weight']
        self.train_stage = None
        self.aggr_method = config['aggr_method'].upper()
        if self.aggr_method == 'LVA':
            self.vs_weights = config['vs_weights']
            self.vs_weights = torch.tensor(self.vs_weights, device=self.device, dtype=torch.float32)
            self.vs_weights /= sum(self.vs_weights)
            assert self.n_layers == len(self.vs_weights), 'n_layers should be equal to length of vs_weights'

        self.gcn = GCN(in_channels=self.embedding_size,
                    hidden_channels=config['hidden_channels'],
                    out_channels=self.embedding_size,
                    num_layers= config['gcn_n_layers'],
                    dropout=config['gcn_dropout'],
                    act=config['gcn_act']).to(self.device)

        # load dataset info
        self.rating_matrix = dataset.inter_matrix(form='coo', value_field=self.RATING).astype(np.float32)
        edge_indice1 = torch.cat((self.rating_matrix.row, self.rating_matrix.col+self.n_users))
        edge_indice2 = torch.cat((self.rating_matrix.col+self.n_users, self.rating_matrix.row))
        edge_weights = self.rating_matrix.data
        self.edge_indices = torch.stack([edge_indice1, edge_indice2], dim=0).to(self.device)
        self.edge_weights = torch.cat([edge_weights, edge_weights])

        if self.load_pretrain_weight:
            user_emb = dataset.get_preload_weight('uid')
            item_emb = dataset.get_preload_weight('iid')
        self.sst_size = self._get_sst_size(dataset.get_user_feature())

        # define layers and loss
        self.user_embedding_layer = torch.nn.Embedding(self.n_users, self.embedding_size, padding_idx=0)
        self.item_embedding_layer = torch.nn.Embedding(self.n_items, self.embedding_size, padding_idx=0)
        if self.load_pretrain_weight:
            self.user_embedding_layer.weight.data.copy_(torch.from_numpy(user_emb))
            self.item_embedding_layer.weight.data.copy_(torch.from_numpy(item_emb))
        self.dis_layer_dict = self.init_dis_layers()
        self.filter_layer_dict = self.init_filter_layers()
        if self.aggr_method == 'LBA':
            self.aggr_layer = nn.Sequential(nn.Linear(self.n_layers*self.embedding_size, self.embedding_size),
                                            activation_layer(self.act),
                                            nn.Linear(self.embedding_size, self.embedding_size),
                                            activation_layer(self.act),
                                            nn.Linear(self.embedding_size, self.embedding_size))

        self.bin_dis_fun = nn.BCELoss()
        self.multi_dis_fun = nn.CrossEntropyLoss()
        self.mse_loss_fun = nn.MSELoss()
        self.sigmoid = nn.Sigmoid()

        # generate intermediate data
        self.norm_rating_matrix = self.get_norm_rating_matrix().to(self.device)

    def _get_sst_size(self, user_feature):
        r""" calculate size of each sensitive attribute for discriminator construction

        Args:
            user_feature(Interaction): contain user's features, such as gender, age, etc.

        Returns:
            dict: every sensitive attribute and its number
        """
        sst_size = {}
        for sst in self.sst_attrs:
            try:
                assert sst in user_feature.columns
            except AssertionError:
                raise ValueError(f'{sst} sensitive attribute not in user feature')

            sst_size[sst] = len(user_feature[sst][1:].unique())

        return sst_size

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
        user_embeddings = self.user_embedding_layer.weight
        item_embeddings = self.item_embedding_layer.weight
        ego_embeddings = torch.cat([user_embeddings, item_embeddings], dim=0)
        return ego_embeddings

    def init_dis_layers(self):
        dis_layer_dict = {}
        for sst in self.sst_attrs:
            output_dim = self.sst_size[sst]
            if output_dim == 2:
                output_dim = 1
            dis_layer_dict[sst] = MLPLayers(layers=[self.embedding_size]+self.dis_hidden_size_list+[output_dim], 
                                activation=self.act).to(self.device)
        
        return dis_layer_dict

    def init_filter_layers(self):
        filter_layer_dict = {}
        for sst in self.sst_attrs:
            filter_layer_dict[sst] = MLPLayers(layers=[self.embedding_size]+self.filter_hidden_size_list+[self.embedding_size],
                                    activation=self.act).to(self.device)
        
        return filter_layer_dict

    def forward(self, sst_list):
        all_embedding = self.get_ego_embeddings()
        if self.train_stage == 'pretrain':
            all_embedding = self.gcn(all_embedding, self.edge_indices, self.edge_weights)
        if self.train_stage == 'finetune':
            temp = None
            for sst in sst_list:
                temp = self.filter_layer_dict[sst](all_embedding) if temp is None else temp+self.filter_layer_dict[sst](all_embedding)
            
            all_embedding = temp/len(self.filter_layer_dict)
        user_all_embeddings, item_all_embeddings = torch.split(all_embedding, [self.n_users, self.n_items])

        return user_all_embeddings, item_all_embeddings

    def calculate_loss(self, interaction, sst_list=None):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        rating = interaction[self.RATING]

        user_all_embeddings, item_all_embeddings = self.forward(sst_list)
        user_embeddings = user_all_embeddings[user]
        item_embeddings = item_all_embeddings[item]

        pred_ratings = (user_embeddings * item_embeddings).sum(dim=-1)
        mse_loss = self.mse_loss_fun(pred_ratings, rating)
        if self.train_stage == 'finetune':
            fair_loss = self.fair_weight * self.calculate_dis_loss(interaction, sst_list)
            return mse_loss - fair_loss
        
        return mse_loss

    def calculate_dis_loss(self, interaction, sst_list):
        r""" Calculate loss of discriminator

        """
        user = interaction[self.USER_ID]

        user_all_embeddings, item_all_embeddings = self.forward(sst_list)
        user_node_embedding = user_all_embeddings[user]
        all_embeddings = torch.cat([user_all_embeddings, item_all_embeddings], dim=0)
        graph_embedding_list = []
        for _ in range(self.n_layers):
            all_embeddings = torch.sparse.mm(self.norm_rating_matrix, all_embeddings)
            graph_embedding_list.append(all_embeddings)

        if self.n_layers == 1:
            all_graph_embeddings = graph_embedding_list[0]
        elif self.aggr_method == 'WAP':
            all_graph_embeddings = torch.stack(graph_embedding_list, dim=1)
            all_graph_embeddings = torch.mean(all_graph_embeddings, dim=1)
        elif self.aggr_method == 'LBA':
            all_graph_embeddings = self.aggr_layer(torch.cat(graph_embedding_list, dim=1))
        elif self.aggr_method == 'LVA':
            all_graph_embeddings = [all_embed[:self.n_users][user] for all_embed in graph_embedding_list]
        
        if self.aggr_method != 'LVA' or self.n_layers == 1:
            user_all_graph_embeddings, _ = torch.split(all_graph_embeddings, [self.n_users, self.n_items])
            user_local_embedding = user_all_graph_embeddings[user]

        node_dis_loss = 0.
        local_dis_loss = 0.
        for sst in sst_list:
            if self.sst_size[sst] == 2:
                node_dis_loss += self.bin_dis_fun(self.sigmoid(self.dis_layer_dict[sst](user_node_embedding)),interaction[sst].float().unsqueeze(1))
                if self.aggr_method == 'LVA' and self.n_layers > 1:
                    for i, weight in enumerate(self.vs_weights):
                        local_dis_loss += weight * self.bin_dis_fun(self.sigmoid(self.dis_layer_dict[sst](all_graph_embeddings[i])),interaction[sst].float().unsqueeze(1))
                else:
                    local_dis_loss += self.bin_dis_fun(self.sigmoid(self.dis_layer_dict[sst](user_local_embedding)),interaction[sst].float().unsqueeze(1))
            else:
                node_dis_loss += self.multi_dis_fun(self.dis_layer_dict[sst](user_node_embedding),interaction[sst].long())
                if self.aggr_method == 'LVA' and self.n_layers > 1:
                    for i, weight in enumerate(self.vs_weights):
                        local_dis_loss += weight * self.multi_dis_fun(self.sigmoid(self.dis_layer_dict[sst](all_graph_embeddings[i])),interaction[sst].long())
                else:
                    local_dis_loss += self.multi_dis_fun(self.sigmoid(self.dis_layer_dict[sst](user_local_embedding)),interaction[sst].long())

        return node_dis_loss + local_dis_loss

    def predict(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]

        user_all_embeddings, item_all_embeddings = self.forward(self.sst_attrs)

        u_embeddings = user_all_embeddings[user]
        i_embeddings = item_all_embeddings[item]
        scores = torch.mul(u_embeddings, i_embeddings).sum(dim=1)
        return scores

    def full_sort_predict(self, interaction):
        user = interaction[self.USER_ID]
        all_user_embedding, all_item_embedding = self.forward(self.sst_attrs)
        user_embedding = all_user_embedding[user]
        # dot with all item embedding to accelerate
        pred_ratings = torch.matmul(user_embedding, all_item_embedding.transpose(0, 1))

        return pred_ratings.view(-1)

    def get_sst_embed(self, user_data, sst_list=None):
        ret_dict = {}
        user_indices = torch.arange(1,self.n_users)
        sst_list = self.sst_attrs if sst_list is None else sst_list
        for sst in sst_list:
            ret_dict[sst] = user_data[sst][user_indices-1]
        user_embeddings, _ = self.forward(sst_list)
        ret_dict['embedding'] = user_embeddings[user_indices]

        return ret_dict

