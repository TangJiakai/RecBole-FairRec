import pytorch_lightning as pl
from pytorch_lightning.utilities.types import STEP_OUTPUT
import torch.nn as nn
import torch.nn.functional as F
from sklearn import metrics
from torch.optim import optimizer
from sklearn.preprocessing import LabelBinarizer
import torch


class ModelInterface(pl.LightningModule):
    def __init__(self, model, args) -> None:
        super().__init__()
        self.save_hyperparameters(args)
        self.model = model
        self.output_dim = args.output_dim
    
    def forward(self, embedding):
        return self.model(embedding)

    def bce_loss_fn(self, pred, true):
        criterion = nn.BCELoss()
        loss = criterion(pred, true.float())

        return loss

    def cross_etropy_loss_fn(self, pred, true):
        criterion = nn.CrossEntropyLoss()
        loss = criterion(pred, true.squeeze())

        return loss

    def sigmoid(self, x):
        active_fn = nn.Sigmoid()

        return active_fn(x)

    def training_step(self, batch, batch_idx) -> STEP_OUTPUT:
        user_embedding, user_attribute = batch
        preds = self(user_embedding)
        
        result = {}

        if self.output_dim == 1:
            preds = self.sigmoid(preds)
            loss = self.bce_loss_fn(preds, user_attribute.unsqueeze(-1))
            result['auc'] = metrics.roc_auc_score(user_attribute.cpu().numpy(), preds.cpu().detach().numpy(), average='micro')
            result['auc'] = max(result['auc'], 1-result['auc'])

            self.log('train/auc', result['auc'], prog_bar=True, on_epoch=True, on_step=True)
        else:
            loss = self.cross_etropy_loss_fn(preds, user_attribute.long())
            label = F.one_hot(user_attribute.long(), self.output_dim)
            try:
                result['auc'] = metrics.roc_auc_score(label.cpu().numpy(), preds.cpu().detach().numpy(), average='macro', multi_class='ovo')
                self.log('train/auc', result['auc'], prog_bar=True, on_epoch=True, on_step=True)
            except:
                pass

        result['loss'] = loss
        self.log('train/loss', loss.item(), prog_bar=True, on_epoch=True, on_step=True)

        return result

    def validation_step(self, batch, batch_idx):
        user_embedding, user_attribute = batch
        preds = self(user_embedding)
        
        result = {}

        if self.output_dim == 1:
            preds = self.sigmoid(preds)
            loss = self.bce_loss_fn(preds, user_attribute.unsqueeze(-1))
            result['auc'] = metrics.roc_auc_score(user_attribute.cpu().numpy(), preds.cpu().detach().numpy(), average='micro')
            result['auc'] = max(result['auc'], 1-result['auc'])

            self.log('valid/auc', result['auc'], prog_bar=True, on_epoch=True, on_step=True)
        else:
            loss = self.cross_etropy_loss_fn(preds, user_attribute.long())
            label = F.one_hot(user_attribute.long(), self.output_dim)
            try:
                result['auc'] = metrics.roc_auc_score(label.cpu().numpy(), preds.cpu().detach().numpy(), average='macro', multi_class='ovo')
                self.log('valid/auc', result['auc'], prog_bar=True, on_epoch=True, on_step=True)
            except:
                pass

        result['loss'] = loss
        self.log('valid/loss', loss.item(), prog_bar=True, on_epoch=True, on_step=True)

        return result

    def test_step(self, batch, batch_idx):
        user_embedding, user_attribute = batch
        preds = self(user_embedding)
        
        result = {}

        if self.output_dim == 1:
            preds = self.sigmoid(preds)
            result['auc'] = metrics.roc_auc_score(user_attribute.cpu().numpy(), preds.cpu().detach().numpy(), average='micro')
            result['auc'] = max(result['auc'], 1-result['auc'])

            self.log('test/auc', result['auc'], prog_bar=True, on_epoch=True, on_step=True)
        else:
            label = F.one_hot(user_attribute.long(), self.output_dim)
            try:
                result['auc'] = metrics.roc_auc_score(label.cpu().numpy(), preds.cpu().detach().numpy(), average='macro', multi_class='ovo')
                self.log('test/auc', result['auc'], prog_bar=True, on_epoch=True, on_step=True)
            except:
                pass

        return result

    def configure_optimizers(self):
        weight_decay = self.hparams.weight_decay
        lr = self.hparams.lr

        optimizer = torch.optim.Adam(self.model.parameters(), lr=lr, weight_decay=weight_decay)

        return optimizer





        
