# _*_ coding: utf-8 _*_
# @Time   : 2022/3/24
# @Author : Jiakai Tang
# @Email  : whut_tangjiakai@qq.com


import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np

from recbole.model.layers import MLPLayers
from recbole.model.abstract_recommender import FairRecommender
from recbole.model.loss import BPRLoss
from recbole.utils import InputType


class PFCN_DMF(FairRecommender):

    input_type = InputType.PAIRWISE

    def __init__(self, config, dataset):
        super(PFCN_DMF, self).__init__(config, dataset)

        # load dataset info
        self.embedding_size = config['embedding_size']
        self.sst_attrs = config['sst_attr_list']
        self.filter_mode = config['filter_mode'].lower()
        self.num_layers = config['num_layers']
        self.mlp_dropout = config['mlp_dropout']
        self.mlp_activation = config['mlp_activation']
        self.dis_activation = config['dis_activation']
        try:
            assert self.filter_mode in ('cm','sm','none')
        except AssertionError:
            raise AssertionError('filter_mode must be cm, sm or none')
        self.filter_num, self.sst_dict = self._get_filter_info()
        self.sst_size = self._get_sst_size(dataset.get_user_feature())

        if self.filter_mode != 'none':
            self.dis_drop_out = config['dis_dropout']
            self.dis_weight = config['dis_weight']
            self.dis_hidden_size_list = config['dis_hidden_size_list']

        # define layers and loss
        self.user_embedding_layer = nn.Embedding(self.n_users, self.embedding_size)
        self.item_embedding_layer = nn.Embedding(self.n_items, self.embedding_size)
        self.user_mlp = MLPLayers(layers=[self.embedding_size]+[self.embedding_size for _ in range(self.num_layers)],
                        dropout=self.mlp_dropout, activation=self.mlp_activation, init_method='norm')
        self.item_mlp = MLPLayers(layers=[self.embedding_size]+[self.embedding_size for _ in range(self.num_layers)],
                        dropout=self.mlp_dropout, activation=self.mlp_activation, init_method='norm')
        self.loss_fun = BPRLoss()
        self.cosine_similarity = nn.CosineSimilarity()

        if self.filter_mode != 'none':
            self.filter_layer = self.init_filter()
            self.dis_layer_dict = self.init_dis_layer()
            self.multi_dis_fun = nn.CrossEntropyLoss()
            self.bin_dis_fun = nn.BCELoss()

    def _get_filter_info(self):
        if self.filter_mode == 'cm':
            filter_num = len(self.sst_attrs)
            sst_dict = {}
            for i, sst in enumerate(self.sst_attrs):
                sst_dict[sst] = i + 1
        elif self.filter_mode == 'sm':
            filter_num = 2 ** len(self.sst_attrs) - 1
            sst_dict = {}
            for i, sst in zip(2 ** np.array(range(len(self.sst_attrs))), self.sst_attrs):
                sst_dict[sst] = i
        else:
            filter_num = 0
            sst_dict = {}

        return filter_num, sst_dict

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
        filter_layer = {}
        embedding_size = self.embedding_size
        for i in range(self.filter_num):
            filter_model = MLPLayers([embedding_size, embedding_size*2, embedding_size],
                               activation=self.dis_activation,
                               bn=True,
                               init_method='norm')
            filter_layer[i+1] = filter_model.to(self.device)

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
                                   activation=self.dis_activation,
                                   bn=True,
                                   init_method='norm').to(self.device)

        return dis_layer_dict

    def forward(self, user, item=None, sst_list=None):
        user_embed = self.user_embedding_layer(user)
        user_embed = self.user_mlp(user_embed)
        item_embed = None
        if item is not None:
            item_embed = self.item_embedding_layer(item)
            item_embed = self.item_mlp(item_embed)
        if self.filter_mode == 'none':
            return user_embed, item_embed
        elif self.filter_mode == 'sm':
            idx = 0
            for sst in sst_list:
                idx += self.sst_dict[sst]

            user_embed = self.filter_layer[idx](user_embed)
        else:
            user_temp = None
            for sst in sst_list:
                idx = self.sst_dict[sst]
                embed = self.filter_layer[idx](user_embed)
                user_temp = embed if user_temp is None else user_temp + embed

            user_embed = user_temp / len(self.filter_layer)

        return user_embed, item_embed

    def predict(self, interaction, sst_list=None):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        user_embeddings, item_embeddings = self.forward(user, item, sst_list)

        pred_scores = self.cosine_similarity(user_embeddings, item_embeddings)

        return pred_scores

    def calculate_loss(self, interaction, sst_list):
        user = interaction[self.USER_ID]
        pos_item = interaction[self.POS_ITEM_ID]
        neg_item = interaction[self.NEG_ITEM_ID]

        user_embed, pos_item_embed = self.forward(user, pos_item, sst_list)
        neg_item_embed = self.item_embedding_layer(neg_item)
        neg_item_embed = self.item_mlp(neg_item_embed)

        pos_scores = self.cosine_similarity(user_embed, pos_item_embed) * 10
        neg_scores = self.cosine_similarity(user_embed, neg_item_embed) * 10

        bpr_loss = self.loss_fun(pos_scores, neg_scores)
        if self.filter_mode != 'none':
            dis_loss = self.calculate_dis_loss(interaction,sst_list)
            return bpr_loss - self.dis_weight * dis_loss

        return bpr_loss

    def calculate_dis_loss(self, interaction, sst_list):
        user = interaction[self.USER_ID]
        sst_label_dict = {}
        for sst in sst_list:
            sst_label_dict[sst] = interaction[sst]
        dis_loss = .0

        user_embed, _ = self.forward(user,None,sst_list)
        for sst in sst_list:
            dis_layer = self.dis_layer_dict[sst]
            if self.sst_size[sst] == 2:
                logits = nn.Sigmoid()(dis_layer(user_embed))
                dis_loss += self.bin_dis_fun(logits, sst_label_dict[sst].float().unsqueeze(1))
            else:
                dis_loss += self.multi_dis_fun(dis_layer(user_embed), sst_label_dict[sst].long())

        return dis_loss

    def full_sort_predict(self, interaction, sst_list=None):
        user = interaction[self.USER_ID]

        user_embed = self.forward(user,None,sst_list)
        all_item_embed = self.item_embedding_layer.weight
        all_item_embed = self.item_mlp(all_item_embed)
        # dot with all item embedding to accelerate
        pred_scores = self.cosine_similarity(torch.repeat_interleave(user_embed,self.n_items,dim=0), 
                                            all_item_embed.repeat(self.n_users,1))

        return pred_scores.view(-1)

    def get_sst_embed(self, user_data, sst_list=None):
        ret_dict = {}
        indices = torch.unique(user_data[self.USER_ID])
        sst_list = self.sst_attrs if self.filter_mode == 'none' else sst_list
        for sst in sst_list:
            ret_dict[sst] = user_data[sst][indices - 1]
        user_embeddings, _ = self.forward(indices.to(self.device),None,sst_list)
        ret_dict['embedding'] = user_embeddings

        return ret_dict

