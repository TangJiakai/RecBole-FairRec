# RecBole-FairRec
Implement Fair Recommendation Model In RecBole
- [x] FOCF([Beyond Parity：Fairness Objectives for Collaborative Filtering](https://proceedings.neurips.cc/paper/2017/hash/e6384711491713d29bc63fc5eeb5ba4f-Abstract.html) in NIPS 2017)
## Model Performance
### FOCF
- **MovieLens-1M**  
    hyper parameters for **hyper tuning**:   
    - learning rate: loguniform [-6,0]   
    - embedding size: choice [2,4,8,16,32]  
    
    **best** hyper parameters:  
    - learning rate:   
    - emebdding size:   

| Unfairness | Error(RMSE) | Value | Absolute | Underestimation | Overestimation | Non-Parity |
|:-:| :-:| :-: | :-: | :-: | :-: | :-: |
| None | 1.2214 | 0.6882 | 0.5545 | 0.5473 | 0.1409 | 0.0705 |
| Value | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 |
| Absolute | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 |
| Underestimation | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 |
| Overestimation | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 |
| Non-Parity| 单元格 | 单元格 | 单元格 | 单元格 | 单元格 | 单元格 |



