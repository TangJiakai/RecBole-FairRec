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
| Underestimation | 0.9555 | 0.5691 | 0.5098 | 0.068 | 0.5011 | 0.3410 |
| Overestimation | 0.9437 | 0.5776 | 0.5271 | 0.5251 | 0.0525 | 0.2314 |
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
|  NDCG@5 |  0.3442 |0.2048 |0.2058 | 0.2044 | 0.2028 | 0.2079 | 0.2088 | 0.2033 | 0.2054 | 0.1952 | 0.2083  | 0.2049  | 0.2067 |  0.2079 | 0.2061  |
|  HIT@5 |  0.7737  |0.595 |0.5997 | 0.6007 |0.5962 | 0.6007| 0.6058 | 0.5914 | 0.5995  | 0.5829 | 0.6046  | 0.593  | 0.5983 | 0.6048  |  0.5998 |
|  AUC<br/>bin:micro<br/>mul:macro |  G0.7558<br/>A0.6588<br/>O0.5429 | 0.5418  | 0.5135  | 0.6170 | 0.5171  | 0.6929  | 0.5016  |  G0.5558<br/>A0.6371 | G0.5231<br/>A0.5171 | G0.5243<br/>O0.6908  | G0.5162<br/>O0.5030  |  A0.6239<br/>O0.7018 | A0.5063<br/>O0.5017  | G0.5450<br/>A0.5359<br/>O0.7047  |  G0.5133<br/>A0.5128<br/>O0.5057 |

### PFCN_PMF
- **MovieLens-1M**


|   | Orgin| SM-G | CM-G  |  SM-A | CM-A  |  SM-O | CM-O | SM-GA | CM-GA | SM-GO | CM-GO | SM-AO | CM-AO | SM-GAO | CM-GAO |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|  NDCG@5 |  0.2619 |0.2626 | 0.2643 |0.2619  | 0.2620|0.2615 | 0.2622 |0.2602 |0.2632 | 0.2618 | 0.2633  | 0.2626  | 0.2625 | 0.2623  | 0.2620  |
|  HIT@5 | 0.6469   | 0.6568 | 0.6656 | 0.6641 | 0.6599 | 0.6598 |0.6649  | 0.6603 | 0.6649 | 0.6589 | 0.6636  | 0.6609  | 0.6627 | 0.6669  | 0.6644 |
|  AUC<br/>bin:micro<br/>mul:macro |  G0.5221<br/>A0.5671<br/>O0.5128 | 0.5710 | 0.5119  | 0.5337 | 0.5127 | 0.7551  | 0.6667 |  G0.5013<br/>A0.5318 | G0.5140<br/>A0.5016 | G0.5093<br/>O0.7438  | G0.5199<br/>O0.6538 |  A0.6428<br/>O0.7498 | A0.5010<br/>O0.5087  |  G0.5413<br/>A0.6296<br/>O0.7550 | G0.5126<br/>A0.5150<br/>O0.5763  |
