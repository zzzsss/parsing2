/*
 * Process_train.cpp
 *
 *  Created on: 2015��3��18��
 *      Author: zzs
 */

#include "Process.h"

void Process::train()
{
	nn_train_prepare();
	//5. main training
	cout << "5.start training: " << endl;
	double best_result = 0;
	string mach_cur_name = parameters->CONF_mach_name+parameters->CONF_mach_cur_suffix;
	string mach_best_name = parameters->CONF_mach_name+parameters->CONF_mach_best_suffix;
	for(int i=cur_iter;i<parameters->CONF_NN_ITER;i++){
		if(cur_iter>0)
			write_restart_conf();
		nn_train_one_iter();
		cout << "-- Iter done, waiting for test dev:" << endl;
		double this_result = nn_dev_test(parameters->CONF_dev_file,parameters->CONF_output_file+".dev",parameters->CONF_dev_file);
		dev_results[cur_iter] = this_result;
		//write curr mach
		ofstream fs;
		fs.open(mach_cur_name.c_str(),ios::binary);
		mach->Write(fs);
		fs.close();
		//possible write best mach
		if(this_result > best_result){
			cout << "-- get better result, write to " << mach_best_name << endl;
			best_result = this_result;
			fs.open(mach_best_name.c_str(),ios::binary);
			mach->Write(fs);
			fs.close();
		}
		//lrate schedule
		set_lrate_one_iter();
		if(cur_iter>0)
			delete_restart_conf();
		cur_iter++;
	}

	//6.results
	cout << "6.training finished with dev results: " << endl;
	cout << "zzzzz ";
	for(int i=0;i<parameters->CONF_NN_ITER;i++)
		cout << dev_results[i] << " ";
	cout << endl;
}

void Process::test(Mach* m,Dict* d)
{
	cout << "----- Testing -----" << endl;
	mach = m;
	dict = d;
	nn_dev_test(parameters->CONF_test_file,parameters->CONF_output_file,parameters->CONF_gold_file);
}

double Process::nn_dev_test(string to_test,string output,string gold)
{
	time_t now;
	time(&now); //ctime is not rentrant ! use ctime_r() instead if needed
	cout << "#--Test at " << ctime(&now) << std::flush;
	//also assuming test-file itself is gold file(this must be true with dev file)
	dev_test_corpus = read_corpus(to_test);
	each_get_featgen(1);	/*************virtual****************/
	int token_num = 0;	//token number
	int miss_count = 0;
	for(int i=0;i<dev_test_corpus->size();i++){
		DependencyInstance* t = dev_test_corpus->at(i);
		int length = t->forms->size();
		token_num += length - 1;
		vector<int>* ret = each_test_one(t);		/*************virtual****************/
		for(int i2=1;i2<length;i2++){	//ignore root
			if((*ret)[i2] != (*(t->heads))[i2])
				miss_count ++;
		}
		delete t->heads;
		t->heads = ret;
	}
	write_corpus(dev_test_corpus,output);
	string ttt;
	double rate = (double)(token_num-miss_count) / token_num;
	cout << "Evaluate:" << (token_num-miss_count) << "/" << token_num
			<< "(" << rate << ")" << endl;
	DependencyEvaluator::evaluate(gold,output,ttt,false);

	//clear
	for(int i=0;i<dev_test_corpus->size();i++){
		delete dev_test_corpus->at(i);
	}
	delete dev_test_corpus;
	return rate;
}

//----------------- main training process
void Process::nn_train_one_iter()
{
	time_t now;
	time(&now); //ctime is not rentrant ! use ctime_r() instead if needed
	cout << "##*** Start training for iter " << cur_iter << " at " << ctime(&now)
			<< "with lrate " << cur_lrate << std::flush;
	each_prepare_data_oneiter();	/*************virtual****************/
	while(1){
		int n_size = mach->GetBsize();
		/*************virtual****************/
		REAL* xinput = each_next_data(&n_size);	//this may change n_size, all memory managed not here
		if(xinput){
			mach->SetDataIn(xinput);
			mach->Forw(n_size);
			//gradient for mach already set when prepare data
			each_get_grad(n_size);	/*************virtual****************/
			mach->Backw(cur_lrate, parameters->CONF_NN_WD, n_size);
			set_lrate();
		}
		else
			break;
	}
}
