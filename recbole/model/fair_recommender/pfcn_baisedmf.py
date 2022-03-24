# _*_ coding: utf-8 _*_
# @Time   : 2022/3/23
# @Author : Jiakai Tang
# @Email  : whut_tangjiakai@qq.com


import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np
from model.Classify import MLPLayers

from recbole.model.abstract_recommender import FairRecommender
from recbole.model.loss import BPRLoss
from recbole.utils import InputType


class PFCN_BiasedMF(FairRecommender):

    input_type = InputType.PAIRWISE

    def __init__(self, config, dataset):
        super(PFCN_BiasedMF, self).__init__(config, dataset)

        # load dataset info
        self.embedding_size = config['embedding_size']
        self.filter_mode = config['filter_mode'].lower()
        try:
            assert self.filter_mode in ('cm','sm','none')
        except AssertionError:
            raise AssertionError('filter_mode must be cm, sm or none')
        if self.filter_mode != 'none':
            self.dis_drop_out = config['dis_dropout']
            self.dis_weight = config['dis_weight']
            self.dis_hidden_size_list = config['dis_hidden_size_list']
        self.sst_attrs = config['sst_attr_list']
        self.activation = config['activation']

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
        self.loss_fun = BPRLoss()

        self.sst_size = self._get_sst_size(dataset.get_user_feature())
        if self.filter_mode != 'none':
            self.filter_layer = self.init_filter()
            self.dis_layer_dict = self.init_dis_layer()
            self.multi_dis_fun = nn.CrossEntropyLoss()
            self.bin_dis_fun = nn.BCELoss()

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

    def init_filter(self):
        r""" according selection of filter mode, build corresponding filter layer

        Returns:
            list: filter layer
        """
        filter_layer = []
        embedding_size = self.embedding_size
        filter_num = 1
        if self.filter_mode == 'cm':
            filter_num = len(self.sst_attrs)
        for _ in range(filter_num):
            filter_model = MLPLayers([embedding_size, embedding_size*2, embedding_size],
                               dropout=self.drop_out,
                               activation=self.activation,
                               bn=True,
                               init_method='norm')
            filter_layer.append(filter_model.to(self.device))

        return filter_layer

    def init_dis_layer(self):
        r""" build discriminator for each sensitive attribute

        Return:
            dict: sensitive attribute and its discriminator
        """
        embedding_size = self.embedding_size
        sst_size = self.sst_size
        sst_attrs = self.sst_attrs
        dis_hidden_size_list = self.dis_hidden_size_list
        dis_layer_dict = {}
        for sst in sst_attrs:
            output_dim = sst_size[sst]
            if output_dim == 2:
                output_dim = 1
            dis_layer_dict[sst] = MLPLayers([embedding_size] + dis_hidden_size_list + [output_dim],
                                   dropout=self.dis_drop_out,
                                   activation=self.activation,
                                   bn=True,
                                   init_method='norm').to(self.device)

        return dis_layer_dict

    def forward(self, user, item=None):

        user_embedding = self.get_user_embedding(user)
        item_embedding = None
        if item is not None:
            col_indices = self.history_user_id[item].flatten()
            row_indices = torch.arange(item.shape[0]).to(self.device).\
                repeat_interleave(self.history_user_id.shape[1], dim=0)
            matrix_rating = torch.zeros(1).to(self.device).repeat(item.shape[0], self.n_users)
            matrix_rating.index_put_((row_indices, col_indices), self.history_user_value[item].flatten())
            item_embedding = self.item_embedding_layer(matrix_rating)

        if self.filter_mode == 'none':
            return user_embedding, item_embedding

        user_temp = None
        for layer in self.filter_layer:
            embed = layer(user_embedding)
            user_temp = embed if user_temp is None else user_temp + embed

        user_embedding = user_temp / len(self.filter_layer)

        return user_embedding, item_embedding

    def predict(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        user_embeddings, item_embeddings = self.forward(user, item)

        pred_scores = torch.mul(user_embeddings, item_embeddings).sum(dim=-1)

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
        pos_item = interaction[self.POS_ITEM_ID]
        neg_item = interaction[self.NEG_ITEM_ID]

        user_embed, pos_item_embed = self.forward(user, pos_item)

        col_indices = self.history_user_id[neg_item].flatten()
        row_indices = torch.arange(neg_item.shape[0]).to(self.device). \
            repeat_interleave(self.history_user_id.shape[1], dim=0)
        matrix_rating = torch.zeros(1).to(self.device).repeat(neg_item.shape[0], self.n_users)
        matrix_rating.index_put_((row_indices, col_indices), self.history_user_value[neg_item].flatten())
        neg_item_embed = self.item_embedding_layer(matrix_rating)

        pos_scores = torch.mul(user_embed, pos_item_embed).sum(dim=-1)
        neg_scores = torch.mul(user_embed, neg_item_embed).sum(dim=-1)

        bpr_loss = self.loss_fun(pos_scores, neg_scores)
        if self.filter_mode != 'none':
            dis_loss = self.calculate_dis_loss(interaction)
            return bpr_loss - self.dis_weight * dis_loss

        return bpr_loss

    def calculate_dis_loss(self, interaction):
        user = interaction[self.USER_ID]
        sst_label_dict = {}
        for sst in self.sst_attrs:
            sst_label_dict[sst] = interaction[sst]
        dis_loss = .0

        user_embed, _ = self.forward(user)
        for sst, dis_layer in self.dis_layer_dict.items():
            if self.sst_size[sst] == 2:
                logits = F.sigmoid(dis_layer(user_embed))
                dis_loss += self.bin_dis_fun(logits, sst_label_dict[sst].float().unsqueeze(1))
            else:
                dis_loss += self.multi_dis_fun(dis_layer(user_embed), sst_label_dict[sst].long())

        return dis_loss

    def full_sort_predict(self, interaction):
        user = interaction[self.USER_ID]

        user_embed = self.forward(user)
        all_item_embed = self.get_item_embedding()
        # dot with all item embedding to accelerate
        pred_scores = torch.mul(user_embed, all_item_embed).sum(dim=-1)

        return pred_scores.view(-1)

    def get_sst_embed(self, user_data):
        ret_dict = {}
        indices = torch.unique(user_data[self.USER_ID])
        for sst in self.sst_attrs:
            ret_dict[sst] = user_data[sst][indices - 1]
        user_embeddings, _ = self.forward(indices.to(self.device))
        ret_dict['embedding'] = user_embeddings

        return ret_dict

