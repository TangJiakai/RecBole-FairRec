# RecBole-FairRec

![logo](asset/logo.png)

**RecBole-FairRec** is a library toolkit built upon [RecBole](https://recbole.io) for reproducing and developing fairness-aware recommendation.

## Highlights

- **Easy-to-use**: Our library shares unified API and input(atomic files) as RecBole.
- **Conveniently learn and compare**: Our library provides several fairess-metrics and frameworks for learning and comparing.
- **Extensive FairRec library**: Recently proposed fairness-aware algorithms can be easily equipped in our library.

## Requirements

```
python>=3.7.0
recbole>=1.0.1
numpy>=1.20.3
torch>=1.11.0
tqdm>=4.62.3
```

## Quick-Start

With the source code, you can use the provided script for initial usage of our library:

```
python run_recbole.py
```
If you want to change the models or datasets, just run the script by setting additional command parameters:
```
python run_recbole.py -m [model] -d [dataset] -c [config_files]
```

## Implement Models

We list the models that we have implemented up to now:

- [FOCF](recbole/model/fair_recommender/focf.py) from Sirui Yao et al:[Beyond Parity：Fairness Objectives for Collaborative Filtering](https://proceedings.neurips.cc/paper/2017/hash/e6384711491713d29bc63fc5eeb5ba4f-Abstract.html)(NIPS 2017). Note: We implement this model with ranking-based metrics, e.g. NDCG@K.
- PFCN from Yunqi Li et al:[Towards Personalized Fairness based on Causal Notion](https://dl.acm.org/doi/abs/10.1145/3404835.3462966?casa_token=zzHePKuKP6AAAAAA:YzZp_qUbzsgd3TXWCAGSRAfEHO2oM0_BuWZ5uZlfj_rudqKGYq8douOaZ0GoizxP54jtz3JDFw725xo)(SIGIR 2021)
  - [PFCN_MLP](recbole/model/fair_recommender/pfcn_mlp.py)
  - [PFCN_BiasedMF](recbole/model/fair_recommender/pfcn_biasedmf.py)
  - [PFCN_DMF](recbole/model/fair_recommender/pfcn_dmf.py)
  - [PFCN_PMF](recbole/model/fair_recommender/pfcn_pmf.py)
- FairGo from Wu Le et al:[Learning Fair Representations for Recommendation: A Graph-based Perspective](https://dl.acm.org/doi/abs/10.1145/3442381.3450015?casa_token=MACP_5U-E6sAAAAA:L-dsEbdusWfmzF06OnATJhF2OXbjfu6el37nC-cGMjev4jGH_TBUedXyAhpfcBMyCyhyxOxLQkxqe_w) (WWW 2021) 
  - [FairGo_PMF(WAP,LBA,LVA)](recbole/model/fair_recommender/fairgo_pmf.py)
  - [FairGo_GCN(WAP,LBA,LVA)](recbole/model/fair_recommender/fairgo_gcn.py)
- [NFCF](recbole/model/fair_recommender/nfcf.py) from Rashidul Islam et al:[Debiasing career recommendations with neural fair collaborative filtering](https://dl.acm.org/doi/abs/10.1145/3442381.3449904?casa_token=ZzbZbC-Fn_oAAAAA:6KCSThLs7UsT9s0ZzeSryT3Mry067KeTiNdurfa9Q9UHWY7fLGgmjPtQy9i1zU1Yqm4Xf46NVYVuu40) (WWW 2021) 

## Datasets

 The datasets used can be downloaded from [Datasets Link](https://drive.google.com/drive/folders/1W6fvJN9ZjuyeqsIuUeodDJk_ajajHkoG).

# Hyper-parameters
We train the models with the default parameter settings, suggested in their original paper.[[link]](results/ml-1m.md)

## The Team
RecBole-FairRec is developed and maintained by members from [RUCAIBox](http://aibox.ruc.edu.cn/), the main developers is Jiakai Tang ([@Tangjiakai](https://github.com/TangJiakai)).

## Acknowledgement

The implementation is based on the open-source recommendation library [RecBole](https://github.com/RUCAIBox/RecBole).

Please cite the following paper as the reference if you use our code or processed datasets.

```
@inproceedings{zhao2021recbole,
  title={Recbole: Towards a unified, comprehensive and efficient framework for recommendation algorithms},
  author={Wayne Xin Zhao and Shanlei Mu and Yupeng Hou and Zihan Lin and Kaiyuan Li and Yushuo Chen and Yujie Lu and Hui Wang and Changxin Tian and Xingyu Pan and Yingqian Min and Zhichao Feng and Xinyan Fan and Xu Chen and Pengfei Wang and Wendi Ji and Yaliang Li and Xiaoling Wang and Ji-Rong Wen},
  booktitle={{CIKM}},
  year={2021}
}
