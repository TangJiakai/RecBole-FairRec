import argparse
import optuna
import logging

from recbole.quick_start import run_recbole


def objective(trail, args, config_dict):
    # config_dict['fair_weight'] = trail.suggest_loguniform('fair_weight', 1e-4, 10.0)
    config_dict['learning_rate'] = trail.suggest_loguniform('learning_rate', 1e-6, 1.0)

    config_file_list = args.config_files.strip().split(' ') if args.config_files else None
    result = run_recbole(args.model, args.dataset, config_file_list, config_dict)

    return result['best_valid_score']


def find_hparams(args, config_dict):
    optuna.logging.get_logger("optuna").addHandler(logging.StreamHandler(sys.stdout))
    study_name = args.dataset
    storage_name = 'sqlite:///{}.db'.format(study_name)
    study = optuna.create_study(study_name="FairGR_LightGCN", storage=storage_name, direction='maximize', load_if_exists=True)
    
    study.optimize(lambda trail: objective(trail, args, config_dict), n_trials=20)
    best_param_file_path = f'log/BS_FairGR/{args.dataset}_best_hparams.log'
    with open(best_param_file_path, 'w') as f:
        f.write('best params\n')
        f.write(study.best_params)
        f.write('best valid score\n')
        f.write(study.best_value)

if __name__ == '__main__':
    import sys,os
    os.chdir(sys.path[0])

    parser = argparse.ArgumentParser()
    parser.add_argument('--model', '-m', type=str, default='BS_FairGR', help='name of models')
    parser.add_argument('--dataset', '-d', type=str, default='FairGR_LightGCN_100k', help='name of datasets')
    parser.add_argument('--checkpoint_dir', '-c', type=str, default='saved_FairGR_LightGCN_100k', help='checkpoint_dir')
    parser.add_argument('--config_files', type=str, default='BS_FairGR.yaml', help='config files')

    args, _ = parser.parse_known_args()

    # config_file_list = args.config_files.strip().split(' ') if args.config_files else None
    # run_recbole(model=args.model, dataset=args.dataset, config_file_list=config_file_list)

    config_dict = {'filter_mode':'sm'}
    find_hparams(args, config_dict)
   
    # filter_mode_list = ['sm','cm']
    # for filter_mode in filter_mode_list:
    #     config_dict[] = filter_mode
    #     run_recbole(model=args.model, dataset=args.dataset, config_file_list=config_file_list, config_dict=config_dict)
    
