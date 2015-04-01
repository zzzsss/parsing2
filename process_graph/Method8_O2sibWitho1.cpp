/*
 * Method8_O2sibWitho1.cpp
 *
 *  Created on: 2015.3.24
 *      Author: zzs
 */

#include "Method8_O2sibWitho1.h"

const string M8_SPECIAL_O1_PREDIX = ".o1";
//rewrite the whole training --> the easiest way but such bad design...
void Method8_O2sibWitho1::train()
{
	//1.first prepare the basic and o2-machine
	Process::nn_train_prepare();
	m1->training_corpus = training_corpus;
	m1->dict = dict;
	m1->each_get_featgen(0);
	m1->mach = NNInterface::create_one(m1->parameters,m1->feat_gen,m1->each_get_mach_outdim());
	//2.force shared embedding --- m1 already ok
	mach->set_tab(m1->mach->get_tab());
    cout << "-Prepare for o1 ok..." << endl;

    //start training both of them
	cout << "5.start training: " << endl;
	double best_result = 0;
	int best_iter = -1;
	string mach_cur_name = parameters->CONF_mach_name+parameters->CONF_mach_cur_suffix;
	string mach_best_name = parameters->CONF_mach_name+parameters->CONF_mach_best_suffix;
	string mach_cur_name_o1 = mach_cur_name+M8_SPECIAL_O1_PREDIX;	//special machine name written
	string mach_best_name_o1 = mach_best_name+M8_SPECIAL_O1_PREDIX;
	for(int i=cur_iter;i<parameters->CONF_NN_ITER;i++){
		m8_nn_train_one_iter();
		cout << "-- Iter done, waiting for test dev:" << endl;
		double this_result = nn_dev_test(parameters->CONF_dev_file,parameters->CONF_output_file+".dev",parameters->CONF_dev_file);
		dev_results[cur_iter] = this_result;
		//write curr mach
		mach->Write(mach_cur_name);
		m1->mach->Write(mach_cur_name_o1);
		//possible write best mach
		if(this_result > best_result){
			cout << "-- get better result, write to " << mach_best_name << endl;
			best_result = this_result;
			best_iter = cur_iter;
			mach->Write(mach_best_name);
			m1->mach->Write(mach_best_name_o1);
		}
		//lrate schedule
		set_lrate_one_iter();
		cur_iter++;
	}

	//6.results
	cout << "6.training finished with dev results: best " << best_result << "|" << best_iter << endl;
	cout << "zzzzz ";
	for(int i=0;i<parameters->CONF_NN_ITER;i++)
		cout << dev_results[i] << " ";
	cout << endl;
}

vector<int>* Method8_O2sibWitho1::each_test_one(DependencyInstance* x)
{
	vector<int>* ret;
	//combine o1 scores
	double* scores_o1 = get_scores_o1(x,parameters,m1->mach,m1->feat_gen);	//same parameters
	ret = parse_o2sib(x,scores_o1);
	delete [] scores_o1;
	return ret;
}

void Method8_O2sibWitho1::m8_nn_train_one_iter()
{
	time_t now;
	time(&now); //ctime is not rentrant ! use ctime_r() instead if needed
	cout << "##*** Start training for iter " << cur_iter << " at " << ctime(&now)
			<< "with lrate " << cur_lrate << std::flush;
	//prepare
	m1->each_prepare_data_oneiter();
	each_prepare_data_oneiter();
	while(1){
		int finish_num = 0;
		//o1
		{
		int n_size = m1->mach->GetBsize();
		REAL* xinput = m1->each_next_data(&n_size);	//this may change n_size, all memory managed not here
		if(xinput){
			m1->mach->SetDataIn(xinput);
			m1->mach->Forw(n_size);
			//gradient for mach already set when prepare data
			m1->each_get_grad(n_size);	/*************virtual****************/
			m1->mach->Backw(cur_lrate, parameters->CONF_NN_WD, n_size);
			m1->set_lrate();
		}
		else
			finish_num++;
		}
		//o2
		{
		int n_size = mach->GetBsize();
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
			finish_num++;
		}
		if(finish_num==2)
			break;
	}
}

void Method8_O2sibWitho1::test(string m_name)
{
	cout << "----- Testing -----" << endl;
	Dict* temp_d = new Dict(parameters->CONF_dict_file);
	NNInterface* temp_m = NNInterface::Read(m_name);
	mach = temp_m;
	dict = temp_d;
	m1->feat_gen = new FeatureGenO1(dict,parameters->CONF_x_window,parameters->CONF_add_distance,
			parameters->CONF_add_pos,parameters->CONF_add_distance_parent);
	{
		NNInterface* temp_m = NNInterface::Read(m_name+M8_SPECIAL_O1_PREDIX);
		m1->mach = temp_m;
	}
	nn_dev_test(parameters->CONF_test_file,parameters->CONF_output_file,parameters->CONF_gold_file);
}

