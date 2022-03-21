# -*- coding: utf-8 -*-
# @Time   : 2022/3/7
# @Author : Jiakai Tang
# @Email  : whut_tangjiakai@qq.com

r"""
PersonaliedFair
################################################
Reference:
    Li, Y., et al., ”Towards Personalized Fairness based on Causal Notion.“ in SIGIR 2021
"""

import torch
import torch.nn as nn
from recbole.model.abstract_recommender import FairRecommender
from recbole.model.layers import MLPLayers
from recbole.model.loss import BPRLoss
from recbole.utils import InputType


class PFCN(FairRecommender):
    r"""PFCN is a personalized and fair-aware recommendation algorithm

    which has 2 version: combination model(cm) and separated model(sm)
    """
    input_type = InputType.PAIRWISE

    def __init__(self, config, dataset):
        super(PFCN, self).__init__(config, dataset)

        # load dataset info
        self.filter_mode = config['filter_mode'].lower()
        self.sst_attr = config['sst_attr_list']
        try:
            assert self.filter_mode in ('cm','sm')
        except AssertionError:
            raise AssertionError('filter_mode must be cm or sm')
        self.POS_ITEM_ID = self.ITEM_ID
        self.NEG_ITEM_ID = config['NEG_PREFIX'] + self.ITEM_ID

        # load parameters info
        self.embedding_size = config['embedding_size']
        try:
            assert self.embedding_size % 2 == 0
        except AssertionError:
            raise AssertionError('embedding size must be multiples of 2')
        self.drop_out = config['dropout']
        self.dis_drop_out = config['dis_dropout']
        self.activation = config['activation']
        # self.deep_model_hidden_size_list = config['deep_model_hidden_size_list']
        self.dis_hidden_size_list = config['dis_hidden_size_list']
        self.mlp_hidden_size_list = config['mlp_hidden_size_list']
        try:
            assert self.mlp_hidden_size_list[0] == 2 * self.embedding_size and \
                   self.mlp_hidden_size_list[-1] == 1
        except AssertionError:
            raise AssertionError('the first of mlp_num_layers should be '
                                 'equal to embedding size and the last should be 1')

        # define layers and loss
        self.sst_size = self._get_sst_size(dataset.get_user_feature())
        self.user_embedding = nn.Embedding(self.n_users, self.embedding_size)
        self.item_embedding = nn.Embedding(self.n_items, self.embedding_size)
        self.filter_layer = self.init_filter()
        self.mlp_layer = MLPLayers(self.mlp_hidden_size_list, dropout=self.drop_out)
        self.dis_layer_dict = self.init_dis_layer()
        self.loss_fun = BPRLoss()
        self.dis_fun = nn.CrossEntropyLoss()

    def _get_sst_size(self, user_feature):
        r""" calculate size of each sensitive attribute for discriminator construction

        Args:
            user_feature(Interaction): contain user's features, such as gender, age, etc.

        Returns:
            dict: every sensitive attribute and its number
        """
        sst_size = {}
        for sst in self.sst_attr:
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
            filter_num = len(self.sst_attr)
        for _ in range(filter_num):
            filter_model = MLPLayers([embedding_size, embedding_size//2, embedding_size],
                               dropout=self.drop_out,
                               activation=self.activation,
                               bn=True)
            filter_layer.append(filter_model.to(self.device))

        return filter_layer

    def init_dis_layer(self):
        r""" build discriminator for each sensitive attribute

        Return:
            dict: sensitive attribute and its discriminator
        """
        embedding_size = self.embedding_size
        sst_size = self.sst_size
        sst_attr = self.sst_attr
        dis_hidden_size_list = self.dis_hidden_size_list
        dis_layer_dict = {}
        for sst in sst_attr:
            dis_layer_dict[sst] = MLPLayers([embedding_size] + dis_hidden_size_list + [sst_size[sst]],
                                   dropout=self.drop_out,
                                   activation=self.activation,
                                   bn=True).to(self.device)

        return dis_layer_dict

    def forward(self, user, item=None):
        user_embed = self.user_embedding(user)
        item_embed = None
        if item is not None:
            item_embed = self.item_embedding(item)
        user_temp = None
        for layer in self.filter_layer:
            embed = layer(user_embed)
            user_temp = embed if user_temp is None else user_temp + embed

        user_embed = user_temp / len(self.filter_layer)

        return user_embed, item_embed

    def predict(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]

        user_all_embeddings, item_all_embeddings = self.forward(user, item)

        return self.mlp_layer(torch.cat((user_all_embeddings, item_all_embeddings), dim=1))

    def calculate_loss(self, interaction):
        user = interaction[self.USER_ID]
        pos_item = interaction[self.POS_ITEM_ID]
        neg_item = interaction[self.NEG_ITEM_ID]

        user_embed, pos_item_embed = self.forward(user, pos_item)
        neg_item_embed = self.item_embedding(neg_item)

        pos_scores = self.mlp_layer(torch.cat((user_embed, pos_item_embed), dim=1))
        neg_scores = self.mlp_layer(torch.cat((user_embed, neg_item_embed), dim=1))

        bpr_loss = self.loss_fun(pos_scores, neg_scores)
        # dis_loss = self.calculate_dis_loss(interaction)

        return bpr_loss

    def calculate_dis_loss(self, interaction):
        user = interaction[self.USER_ID]
        sst_label_dict = {}
        for sst in self.sst_attr:
            sst_label_dict[sst] = interaction[sst]
        dis_loss = .0

        user_embed, _ = self.forward(user)
        for sst, dis_layer in self.dis_layer_dict.items():
            dis_loss += self.dis_fun(dis_layer(user_embed), sst_label_dict[sst].long())

        return dis_loss

    def full_sort_predict(self, interaction):
        user = interaction[self.USER_ID]

        user_embed = self.forward(user)
        all_item_embed = self.item_embedding.weight
        # dot with all item embedding to accelerate
        pred_scores = self.mlp_layer(torch.cat((user_embed.repeat_interleave(self.n_items, dim=0),
                                                all_item_embed.repeat(self.n_users,1))))

        return pred_scores.view(-1)