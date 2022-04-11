from recbole.data.dataloader.abstract_dataloader import AbstractDataLoader, NegSampleDataLoader
import numpy as np


class FOCFDataLoader(NegSampleDataLoader):

    def __init__(self, config, dataset, sampler, shuffle=False):
        self._set_neg_sample_args(config, dataset, config['MODEL_INPUT_TYPE'], config['train_neg_sample_args'])
        super().__init__(config, dataset, sampler, shuffle=False)
        self.ITEM_ID = config['ITEM_ID_FIELD']
        self.dataset.sort(by=self.ITEM_ID)
        self.item_num = self.dataset.item_num
        self.item_uniques = np.unique(self.dataset.inter_feat[self.ITEM_ID])

    def _init_batch_size_and_step(self):
        batch_size = self.config['train_batch_size']
        if self.neg_sample_args['strategy'] == 'by':
            batch_num = max(batch_size // self.times, 1)
            new_batch_size = batch_num * self.times
            self.step = batch_num
            self.set_batch_size(new_batch_size)
        else:
            self.step = batch_size
            self.set_batch_size(batch_size)

    def update_config(self, config):
        self._set_neg_sample_args(config, self.dataset, config['MODEL_INPUT_TYPE'], config['train_neg_sample_args'])
        super().update_config(config)

    @property
    def pr_end(self):
        return len(self.dataset)

    def _shuffle(self):
        self.dataset.shuffle()

    def _next_batch_data(self):
        # cur_data = self._neg_sampling(self.dataset[self.pr:self.pr + self.step])
        cnt = 0
        select_item = np.arange(0, self.item_num)
        is_select = np.array([False]*self.item_num)
        is_select[self.item_uniques] = True
        indices_list = []
        while cnt < self.step or not any(is_select): 
            iid = np.random.choice(select_item[is_select],1,False)[0]
            indice = np.where(self.dataset.inter_feat[self.ITEM_ID] == iid)[0]
            cnt += len(indice)
            is_select[iid] = False
            indices_list.extend(indice)
        self.pr += self.step
        return self.dataset[indices_list]