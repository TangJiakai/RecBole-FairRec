import torch.nn as nn
import torch


class Classifier(nn.Module):
    def __init__(self, input_dim, output_dim, dropout=0.0, negative_slope=0.2):
        super().__init__()

        self.input_dim = input_dim
        self.output_dim = output_dim
        self.network = nn.Sequential(
            nn.Linear(input_dim, input_dim*2),
            nn.LeakyReLU(negative_slope),
            nn.Dropout(dropout),
            nn.Linear(input_dim*2, input_dim*2),
            nn.LeakyReLU(negative_slope),
            nn.Dropout(dropout),
            nn.Linear(input_dim*2, input_dim*4),
            nn.LeakyReLU(negative_slope),
            nn.Dropout(dropout),
            nn.Linear(input_dim*4, input_dim*2),
            nn.LeakyReLU(negative_slope),
            nn.Dropout(dropout),
            nn.Linear(input_dim*2, input_dim*2),
            nn.LeakyReLU(negative_slope),
            nn.Dropout(dropout),
            nn.Linear(input_dim*2, input_dim),
            nn.LeakyReLU(negative_slope),
            nn.Dropout(dropout),
            nn.Linear(input_dim, input_dim//2),
            nn.LeakyReLU(negative_slope),
            nn.Dropout(dropout),
            nn.Linear(input_dim//2, output_dim)
        )

    @staticmethod
    def init_weights(m):
        if type(m) == torch.nn.Linear:
            torch.nn.init.normal_(m.weight, mean=0.0, std=0.1)
            if m.bias is not None:
                torch.nn.init.normal_(m.bias, mean=0.0, std=0.1)

    def forward(self, embeddings):
        return self.network(embeddings)
