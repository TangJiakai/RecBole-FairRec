import pytorch_lightning as pl
from pytorch_lightning.utilities.types import STEP_OUTPUT
import torch.nn as nn
from sklearn import metrics
from torch.optim import optimizer
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
            loss = self.bce_loss_fn(preds, user_attribute)
            result['auc'] = metrics.roc_auc_score(user_attribute.cpu().numpy(), preds.cpu().detach().numpy())
            result['auc'] = max(result['auc'], 1-result['auc'])

            self.log('train/auc', result['auc'], prog_bar=True, on_epoch=True, on_step=True)
        else:
            loss = self.cross_etropy_loss_fn(preds, user_attribute)
            result['f1_micro'] = metrics.f1_score(user_attribute.cpu().numpy(), preds.cpu().detach().numpy().argmax(axis=-1), average='micro')
            result['f1_macro'] = metrics.f1_score(user_attribute.cpu().numpy(), preds.cpu().detach().numpy().argmax(axis=-1), average='macro')
            
            self.log('train/f1_micro', result['f1_micro'], prog_bar=True, on_epoch=True, on_step=True)
            self.log('train/f1_macro', result['f1_macro'], prog_bar=True, on_epoch=True, on_step=True)

        result['loss'] = loss
        self.log('train/loss', loss.item(), prog_bar=True, on_epoch=True, on_step=True)

        return result

    def validation_step(self, batch, batch_idx):
        user_embedding, user_attribute = batch
        preds = self(user_embedding)
        
        result = {}

        if self.output_dim == 1:
            preds = self.sigmoid(preds)
            loss = self.bce_loss_fn(preds, user_attribute)
            result['auc'] = metrics.roc_auc_score(user_attribute.cpu().numpy(), preds.cpu().detach().numpy())
            result['auc'] = max(result['auc'], 1-result['auc'])

            self.log('valid/auc', result['auc'], prog_bar=True, on_epoch=True, on_step=True)
        else:
            loss = self.cross_etropy_loss_fn(preds, user_attribute)
            result['f1_micro'] = metrics.f1_score(user_attribute.cpu().numpy(), preds.cpu().detach().numpy().argmax(axis=-1), average='micro')
            result['f1_macro'] = metrics.f1_score(user_attribute.cpu().numpy(), preds.cpu().detach().numpy().argmax(axis=-1), average='macro')
            
            self.log('valid/f1_micro', result['f1_micro'], prog_bar=True, on_epoch=True, on_step=True)
            self.log('valid/f1_macro', result['f1_macro'], prog_bar=True, on_epoch=True, on_step=True)

        result['loss'] = loss
        self.log('valid/loss', loss.item(), prog_bar=True, on_epoch=True, on_step=True)

        return result

    def test_step(self, batch, batch_idx):
        user_embedding, user_attribute = batch
        preds = self(user_embedding)
        
        result = {}

        if self.output_dim == 1:
            result['auc'] = metrics.roc_auc_score(user_attribute.cpu().numpy(), preds.cpu().detach().numpy())
            result['auc'] = max(result['auc'], 1-result['auc'])

            self.log('test/auc', result['auc'], prog_bar=True, on_epoch=True, on_step=True)
        else:
            result['f1_micro'] = metrics.f1_score(user_attribute.cpu().numpy(), preds.cpu().detach().numpy().argmax(axis=-1), average='micro')
            result['f1_macro'] = metrics.f1_score(user_attribute.cpu().numpy(), preds.cpu().detach().numpy().argmax(axis=-1), average='macro')
            
            self.log('test/f1_micro', result['f1_micro'], prog_bar=True, on_epoch=True, on_step=True)
            self.log('test/f1_macro', result['f1_macro'], prog_bar=True, on_epoch=True, on_step=True)

        return result

    def configure_optimizers(self):
        weight_decay = self.hparams.weight_decay
        lr = self.hparams.lr

        optimizer = torch.optim.Adam(self.model.parameters(), lr=lr, weight_decay=weight_decay)

        return optimizer





        
