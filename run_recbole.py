# @Time   : 2020/7/20
# @Author : sahanlei Mu
# @Email  : slmu@ruc.edu.cn

# UPDATE
# @Time   : 2020/10/3, 2020/10/1
# @Author : Yupeng Hou, Zihan Lin
# @Email  : houyupeng@ruc.edu.cn, zhlin@ruc.edu.cn


import argparse
from cProfile import run
import itertools

from recbole.quick_start import run_recbole

if __name__ == '__main__':
    import sys,os
    os.chdir(sys.path[0])

    parser = argparse.ArgumentParser()
    parser.add_argument('--model', '-m', type=str, default='FOCF', help='name of models')
    parser.add_argument('--dataset', '-d', type=str, default='ml-1M', help='name of datasets')
    parser.add_argument('--config_files', type=str, default='FOCF.yaml', help='config files')

    args, _ = parser.parse_known_args()

    config_file_list = args.config_files.strip().split(' ') if args.config_files else None
    run_recbole(model=args.model, dataset=args.dataset, config_file_list=config_file_list)
    

    # config_dict = {}
    # filter_mode_list = ['sm','cm']
    # for filter_mode in filter_mode_list:
    #     config_dict['filter_mode'] = filter_mode
    #     run_recbole(model=args.model, dataset=args.dataset, config_file_list=config_file_list, config_dict=config_dict)
    # fair_weights = [10]
    # fair_objective_list = ['nonparity'] 
    # for fair_objective in fair_objective_list:
    #     config_dict['fair_objective'] = fair_objective
    #     for fair_weight in fair_weights:
    #         config_dict['fair_weight'] = fair_weight
    #         run_recbole(model=args.model, dataset=args.dataset, config_file_list=config_file_list, config_dict=config_dict)

