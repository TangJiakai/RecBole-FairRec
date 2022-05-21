from email.policy import strict
import torch
import torch.nn as nn
from recbole.model.abstract_recommender import FairRecommender

from recbole.model.layers import MLPLayers
from recbole.utils import InputType

r"""
NFCF
################################################
Reference:
Rashidul Islam, Kamrun Naher Keya, Ziqian Zeng, Shimei Pan, and James Foulds. 2021. Debiasing Career Recommendations with Neural Fair Collaborative Filtering. In Proceedings of the Web Conference 2021.
"""

class NFCF(FairRecommender):
    r""" neural fair collaborative filtering (NFCF), a practical framework for mitigating gender bias in recommending career-related 
    sensitive items (e.g. jobs, academic concentrations, or courses of study) using a pre-training and fine-tuning approach to neural collaborative filtering, 
    augmented with bias correction techniques
    """
    input_type = InputType.POINTWISE

    def __init__(self, config, dataset):
        super(NFCF, self).__init__(config, dataset)

        # load dataset info
        self.LABEL = config['LABEL_FIELD']

        # load parameters info
        self.embedding_size = config['embedding_size']
        self.mlp_hidden_size = config['mlp_hidden_size']
        self.dropout = config['dropout']
        self.sst_attr = config['sst_attr_list'][0]
        self.fair_weight = config['fair_weight']
        self.load_pretrain_path = config['load_pretrain_path']

        # define layers and loss
        self.user_embedding = nn.Embedding(self.n_users, self.embedding_size)
        self.item_embedding = nn.Embedding(self.n_items, self.embedding_size)
        self.mlp_layers = MLPLayers([2 * self.embedding_size] + self.mlp_hidden_size + [1], self.dropout)
        self.mlp_layers.logger = None  # remove logger to use torch.save()
        self.sigmoid = nn.Sigmoid()
        self.loss = nn.BCELoss()

        # parameters initialization
        if self.load_pretrain_path is not None:
            self.reset_params(self.load_pretrain_path, dataset.get_user_feature()[1:])

    def reset_params(self, pretrain_path, user_data):
        checkpoint = torch.load(pretrain_path)
        self.load_state_dict(checkpoint['state_dict'], strict=False)

        sst_value = user_data[self.sst_attr]
        sst_unique_value = torch.unique(sst_value)
        sst1_indices = sst_value == sst_unique_value[0]
        sst2_indices = sst_value == sst_unique_value[1]
        ncf_user_embedding = self.user_embedding.weight.data[1:].clone()

        sst_embedding1 = ncf_user_embedding[sst1_indices].mean(dim=0)
        sst_embedding2 = ncf_user_embedding[sst2_indices].mean(dim=0)
        vector_bias = (sst_embedding1 - sst_embedding2)/torch.linalg.norm(sst_embedding1 - sst_embedding2, keepdim=True)
        vector_bias = torch.mul(ncf_user_embedding, vector_bias).sum(dim=1, keepdim=True) * vector_bias        
        user_embedding = ncf_user_embedding - vector_bias

        self.user_embedding.weight.data[1:] = user_embedding
        self.user_embedding.weight.requires_grad = False
        self.item_embedding = nn.Embedding(self.n_items, self.embedding_size)

    def forward(self, user, item):
        user_mlp_e = self.user_embedding(user)
        item_mlp_e = self.item_embedding(item)
        output = self.mlp_layers(torch.cat((user_mlp_e, item_mlp_e), -1))  # [batch_size, layers[-1]]
        
        return self.sigmoid(output.squeeze(-1))

    def get_differential_fairness(self, interaction, score):
        pos_idx = interaction[self.LABEL]==1
        score = score[pos_idx]
        sst_unique_values, sst_indices = torch.unique(interaction[self.sst_attr][pos_idx], return_inverse=True)
        iid_unique_values, iid_indices = torch.unique(interaction[self.ITEM_ID][pos_idx], return_inverse=True)
        score_matric = torch.zeros((len(iid_unique_values), len(sst_unique_values)), device=self.device)
        norm_matrix = torch.zeros((len(iid_unique_values), len(sst_unique_values)), device=self.device)
        epsilon_values = torch.zeros(len(iid_unique_values), device=self.device)

        concentration_parameter = 1.0
        dirichlet_alpha = concentration_parameter/len(iid_unique_values)

        score_matric.index_put_((iid_indices, sst_indices), score, accumulate=True)
        norm_matrix.index_put_((iid_indices, sst_indices), torch.ones(len(sst_indices), device=self.device), accumulate=True)
        score_matric = (score_matric + dirichlet_alpha) / (norm_matrix + concentration_parameter)

        for i in range(len(sst_unique_values)):
            for j in range(i+1, len(sst_unique_values)):
                epsilon = abs(torch.log(score_matric[:,i])-torch.log(score_matric[:,j]))
                epsilon_values = torch.where(epsilon>epsilon_values, epsilon, epsilon_values)
        
        return epsilon_values.mean()

    def calculate_loss(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        label = interaction[self.LABEL]

        output = self.forward(user, item)
        rec_loss = self.loss(output, label)
        if self.load_pretrain_path is None:
            return rec_loss
        fair_loss = self.get_differential_fairness(interaction, output)

        return rec_loss + self.fair_weight * fair_loss

    def predict(self, interaction):
        user = interaction[self.USER_ID]
        item = interaction[self.ITEM_ID]
        return self.forward(user, item)

