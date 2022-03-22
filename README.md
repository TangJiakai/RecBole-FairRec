# RecBole-FairRec
Implement Fair Recommendation Model In RecBole
- [x] FOCF([Beyond Parity：Fairness Objectives for Collaborative Filtering](https://proceedings.neurips.cc/paper/2017/hash/e6384711491713d29bc63fc5eeb5ba4f-Abstract.html) in NIPS 2017)
- [x] PFCN([Towards Personalized Fairness based on Causal Notion](https://dl.acm.org/doi/abs/10.1145/3404835.3462966?casa_token=zzHePKuKP6AAAAAA:YzZp_qUbzsgd3TXWCAGSRAfEHO2oM0_BuWZ5uZlfj_rudqKGYq8douOaZ0GoizxP54jtz3JDFw725xo) in SIGIR 2021)
-------------------------------------------------------------
## Model Performance
### FOCF
- **MovieLens-1M**  
    
    Common Hyper Parameters:

        - train batch size: 160000
        - learner: adam
        - SST_FIELD: gender

    FOCF_None   
    
        **best** hyper parameters:  
        - learning rate: 0.006  
        - emebdding size: 4  
        - regular_weight: 0.001
| Unfairness | Error(RMSE) | Value | Absolute | Underestimation | Overestimation | Non-Parity |
|:-:| :-:| :-: | :-: | :-: | :-: | :-: |
| None | 1.2214 | 0.6882 | 0.5545 | 0.5473 | 0.1409 | 0.0705 |
| Value | 1.1073 | 0.5798 | 0.4693 | 0.4208 | 0.1590 | 0.0154 |
| Absolute | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 |
| Underestimation | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 |
| Overestimation | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 |
| Non-Parity| 单元格 | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 |

### PFCN_MLP
- **MovieLens-1M**

|   | Orgi.  | SM-G | CM-G  |  SM-A | CM-A  |  SM-O | CM-O | SM-GA | CM-GA | SM-GO | CM-GO | SM-AO | CM-AO | SM-GAO | CM-GAO |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|  NDCG@5 |   | 0.2319 |  |  |  | | | | | |   |   | |   |   |
|  HIT@5 |   | 0.6293 | | |  | | | | | | | | |   |   |
|  (macro)AUC |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |

