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
| None | 0.8780 | 0.4538 | 0.3310 | 0.2208 | 0.2330 | 0.0546 |
| Value | 0.9405 | 0.4798 | 0.3526 | 0.2578 | 0.2220 | 0.0645 |
| Absolute |  0.9414 | 0.4854 | 0.3593 | 0.2652 | 0.2202 | 0.1094 |
| Underestimation | 0.9300 | 0.4741 | 0.3433 | 0.2519 | 0.2222 | 0.0525 |
| Overestimation | 0.9300 | 0.4741 | 0.3433 | 0.2519 | 0.2222 | 0.0525 |
| Non-Parity| 0.8789 | 0.4579 | 0.3323 | 0.2309 | 0.2269 | 0.0028 |

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
        - weight_decay: 0.0001
        - dis_weight: 10
        - valid_matric: NDCG@5

|   | Orgin| SM-G | CM-G  |  SM-A | CM-A  |  SM-O | CM-O | SM-GA | CM-GA | SM-GO | CM-GO | SM-AO | CM-AO | SM-GAO | CM-GAO |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|  NDCG@5 | 0.4081  | 0.2499 | 0.2504 | 0.2501 | 0.2504 | 0.2509 | 0.2518 | 0.2498 | 0.2521 | 0.2465 | 0.2529  | 0.2489  | 0.2520 |  0.2499 | 0.2520  |
|  HIT@5 | 0.8300 | 0.6522 | 0.6500 | 0.6474 | 0.6465 | 0.6560 | 0.6457 | 0.6530 | 0.6459 | 0.6472 | 0.6512 | 0.6485 | 0.6523 | 0.6563  | 0.6472 |
|  AUC<br/>bin:micro<br/>mul:macro |  G0.7960<br/>A0.7128<br/>O0.5402 | 0.5710  | 0.5148  | 0.5596  | 0.5864 | 0.6813 |  0.5062 |  G0.5057<br/>A0.5967 | G0.5227<br/>A0.6154 | G0.5146<br/>O0.6918  | G0.5140<br/>O0.5111  |  A0.6465<br/>O0.6918 | A0.5805<br/>O0.5209  |  G0.5175<br/>A0.6097<br/>O0.6853 | G0.5111<br/>A0.5372<br/>O0.5084  |


### PFCN_BiasedMF
- **MovieLens-1M**

|   | Orgin| SM-G | CM-G  |  SM-A | CM-A  |  SM-O | CM-O | SM-GA | CM-GA | SM-GO | CM-GO | SM-AO | CM-AO | SM-GAO | CM-GAO |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|  NDCG@5 |  0.2656 | 0.2361| 0.2543 |0.2367 | 0.2524 | 0.2334 | 0.2506 | 0.2371 | 0.2552 | 0.2306 | 0.2555  | 0.2297  | 0.2540 | 0.2344  | 0.2560  |
|  HIT@5 | 0.6651   | 0.6197| 0.6444 | 0.6147| 0.6402 |0.6101  | 0.6425 |0.6134 | 0.6434 | 0.6126 | 0.6502  |  0.6051 | 0.6444 | 0.6180  | 0.6459  |
|  AUC<br/>bin:micro<br/>mul:macro |  G0.5845<br/>A0.5531<br/>O0.5248 | 0.5459  | 0.5224  | 0.6667 | 0.5127  | 0.7370  | 0.5119  |  G0.5051<br/>A0.6881 | G0.5120<br/>A0.5331 | G0.5398<br/>O0.7316  | G0.5155<br/>O0.5409  |  A0.6933<br/>O0.7238 | A0.5402<br/>O0.5234  | G0.5355<br/>A0.6600<br/>O0.7303  |  G0.5101<br/>A0.5429<br/>O0.5412 |

### PFCN_DMF
- **MovieLens-1M**

|   | Orgin| SM-G | CM-G  |  SM-A | CM-A  |  SM-O | CM-O | SM-GA | CM-GA | SM-GO | CM-GO | SM-AO | CM-AO | SM-GAO | CM-GAO |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|  NDCG@5 |  0.3442 |0.1466 |0.1018| 0.1434 |0.1198 | 0.0969 | 0.1045 | 0.1463 | 0.1074 | 0.1500 | 0.0952  | 0.1144  | 0.1092 |  0.1454 | 0.1023  |
|  HIT@5 |  0.7737  |0.4722 |0.3288 | 0.4737 |0.3965 | 0.3238 | 0.3555 | 0.4743 |0.3662  | 0.4851 | 0.3224  | 0.3685  | 0.3815 | 0.4672  |  0.3523 |
|  AUC<br/>bin:micro<br/>mul:macro |  G0.7558<br/>A0.6588<br/>O0.5429 | 0.5178  | 0.5340  | 0.5166 | 0.5109  | 0.5118  | 0.5122  |  G0.5121<br/>A0.5050 | G0.5351<br/>A0.5085 | G0.5161<br/>O0.5120  | G0.5217<br/>O0.5065  |  A0.5064<br/>O0.5084 | A0.5147<br/>O0.5159  | G0.5141<br/>A0.5096<br/>O0.5048  |  G0.5152<br/>A0.5136<br/>O0.5307 |

### PFCN_PMF
- **MovieLens-1M**


|   | Orgin| SM-G | CM-G  |  SM-A | CM-A  |  SM-O | CM-O | SM-GA | CM-GA | SM-GO | CM-GO | SM-AO | CM-AO | SM-GAO | CM-GAO |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|  NDCG@5 |  0.2619 |0.1572 |0.0698 | 0.1570 | 0.0672| 0.1575 |0.0721  | 0.1542 | 0.0943 | 0.1505 | 0.0996  | 0.1563  | 0.0954 |  0.1580 | 0.1112  |
|  HIT@5 | 0.6526   | 0.4452|0.2483 | 0.4387 |0.2452 | 0.4442 | 0.2565 | 0.4348 | 0.3139 | 0.4291 | 0.3245  | 0.4406  | 0.3182 | 0.4382  | 0.3588  |
|  AUC<br/>bin:micro<br/>mul:macro |  G0.5809<br/>A0.5539<br/>O0.5254 | 0.5336  | 0.5260  | 0.6412 |  0.5127 | 0.5219  | 0.5150  |  G0.5181<br/>A0.5788 | G0.5303<br/>A0.5171 | G0.5298<br/>O0.5050  | G0.5292<br/>O0.5070  |  A0.6340<br/>O0.5065 | A0.5190<br/>O0.5063  |  G0.5464<br/>A0.6229<br/>O0.5008 | G0.5330<br/>A0.5111<br/>O0.5181  |
