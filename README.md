# RecBole-FairRec
Implement Fair Recommendation Model In RecBole
- [x] FOCF([Beyond Parity：Fairness Objectives for Collaborative Filtering](https://proceedings.neurips.cc/paper/2017/hash/e6384711491713d29bc63fc5eeb5ba4f-Abstract.html) in NIPS 2017)
- [x] PFCN([Towards Personalized Fairness based on Causal Notion](https://dl.acm.org/doi/abs/10.1145/3404835.3462966?casa_token=zzHePKuKP6AAAAAA:YzZp_qUbzsgd3TXWCAGSRAfEHO2oM0_BuWZ5uZlfj_rudqKGYq8douOaZ0GoizxP54jtz3JDFw725xo) in SIGIR 2021)
-------------------------------------------------------------
## Model Performance
### FOCF
- **MovieLens-1M**  
    
    Hyper Parameters:

        - train batch size: 20000
        - epochs: 250
        - epoch: 1000
        - learner: adam
        - learning rate: 0.001 
        - emebdding size: 4  
        - regular_weight: 0.001
        - fair_objective: none|value|absolute|under|over|nonparity
        - require_pow: True
        - valid_matric: RMSE

| Unfairness | Error(RMSE) | Value | Absolute | Underestimation | Overestimation | Non-Parity |
|:-:| :-:| :-: | :-: | :-: | :-: | :-: |
| None | 1.0921 | 0.5608 | 0.4473 | 0.3886 | 0.1722 | 0.0391 |
| Value | 1.0921 | 0.5608 | 0.4473 | 0.3886 | 0.1722 | 0.0391 |
| Absolute | 1.0921 | 0.5608 | 0.4473 | 0.3886 | 0.1722 | 0.0391 |
| Underestimation | 1.0921 | 0.5608 | 0.4473 | 0.3886 | 0.1722 | 0.0391 |
| Overestimation | 1.0921 | 0.5608 | 0.4473 | 0.3886 | 0.1722 | 0.0391 |
| Non-Parity| 1.092 | 0.5773 | 0.4396 | 0.3881 | 0.1892 | 0.0272 |

### PFCN_MLP
- **MovieLens-1M**

    Hyper Parameters:

        - train batch size: 20000
        - training_neg_sample: 1
        - train_epoch_interval: 10
        - epoch: 1000
        - learner: adam
        - learning rate: 0.001 
        - dropout: 0.2
        - dis_dropout: 0.3
        - activation: leakyrelu
        - mlp_hidden_size_list: [64, 32, 16]
        - dis_hidden_size_list: [128, 256, 128, 128, 64, 32]
        - emebdding size: 64  
        - regular_weight: 0.0001
        - dis_weight: 10
        - valid_matric: NDCG@5

|   | Orgin| SM-G | CM-G  |  SM-A | CM-A  |  SM-O | CM-O | SM-GA | CM-GA | SM-GO | CM-GO | SM-AO | CM-AO | SM-GAO | CM-GAO |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|  NDCG@5 | 0.4149  | 0.2352 | 0.2351 | 0.2221 | 0.2220 | 0.0648 | 0.0648 | 0.2149 | 0.2190 | 0.2375 |  0.0649 | 0.2177  |0.0650 |   |   |
|  HIT@5 | 0.8460  | 0.6315 | 0.6310 | 0.6162 | 0.6154 | 0.2619 | 0.2619 | 0.5950 | 0.5975 | 0.6253| 0.2531 | 0.6041 | 0.2576 |   |   |
|  AUC<br/>bin:micro<br/>mul:macro |  G0.8015<br/>A0.7234<br/>O0.5600 | 0.5081  | 0.5081  | 0.5054  | 0.5054  | 0.5154  |  0.5154 |  G0.5128<br/>A0.5117 | G0.5063<br/>A0.5148 | G0.5122<br/>O0.5072  | G0.5393<br/>O0.5247  |  A0.5154<br/>G0.5286 | A0.5054<br/>O0.5171  |   |   |

