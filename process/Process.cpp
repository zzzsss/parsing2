/*
 * Process.cpp
 *
 *  Created on: Dec 24, 2014
 *      Author: zzs
 */

#include "Process.h"

void Process::train()
{
	//2. get corpus
	cout << "2.read-corpus:" << endl;
	training_corpus = read_corpus(CONF_train_file);

	//2.5 if continue/restart --- also init some values
	cout << "2.5:let's see whether to continue" << endl;
	read_restart_conf();

	//3. get dictionary --- whether continue to train
	if(CTL_continue){
		cout << "3.get dict from file "<< CONF_dict_file << endl;
		dict = new Dict(CONF_dict_file);
	}
	else{
		cout << "3.get dict from scratch:" << endl;
		dict = new Dict(CONF_dict_remove,1);
		dict->construct_dictionary(training_corpus);
		dict->write(CONF_dict_file);
	}

	//3.5
	// here only the order1 features
	feat_gen = new FeatureGenO1(dict,CONF_x_window,CONF_add_distance);
	feat_gen->deal_with_corpus(training_corpus);
	if(CONF_pos_filter){
		feat_gen->add_filter(training_corpus);
		feat_gen->write_extra_info(CONF_feature_file);
	}

	//4. get machine
	string mach_cur_name = CONF_mach_name+CONF_mach_cur_suffix;
	if(CTL_continue){
		cout << "4.get mach from file "<< mach_cur_name << endl;
		ifstream ifs;
		ifs.open(mach_cur_name.c_str(),ios::binary);
		mach = Mach::Read(ifs);
		ifs.close();
	}
	else{
		cout << "4.get mach from scratch:" << endl;
		MachConfig mach_config(true);
		each_write_mach_conf();
		//for mach_config
		char *argv[2];
		argv[0] = "nn";
		argv[1] = (char*)CONF_mach_conf_name.c_str();
		mach_config.parse_options(2,argv);
	    mach = mach_config.get_machine();
	    if(mach == 0)
	      Error(mach_config.get_error_string().c_str());
	}

	//5. main training
	cout << "5.start training: " << endl;
	double best_result = 0;
	string mach_best_name = CONF_mach_name+CONF_mach_best_suffix;
	double * dev_results = new double[CONF_NN_ITER];
	for(int i=cur_iter;i<CONF_NN_ITER;i++){
		if(cur_iter>0)
			write_restart_conf();

		nn_train_one_iter();
		cout << "-- Iter done, waiting for test dev:" << endl;
		double this_result = nn_dev_test(CONF_dev_file,CONF_output_file+".dev",CONF_dev_file);
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

		cur_iter++;
		delete_restart_conf();
	}

	//6.results
	cout << "6.training finished with dev results: " << endl;
	for(int i=0;i<CONF_NN_ITER;i++)
		cout << dev_results[i] << " ";
	cout << endl;
}

void Process::test(Mach* m,Dict* d)
{
	cout << "----- Testing -----" << endl;
	mach = m;
	dict = d;
	nn_dev_test(CONF_test_file,CONF_output_file,CONF_gold_file);
}

//**************************************************************************************
// default lrate = mach->lrate_begin / (1.0 + total_n_ex_seen * mach->lrate_mult);
// default wdecay: constant

void Process::set_lrate()
{
#ifdef LRATE_HACK
  lrate = lrate_beg - mach->GetNbForw() * lrate_mult;
#else
  if (CONF_NN_LMULT>0)
	  cur_lrate = CONF_NN_LRATE / (1.0 + mach->GetNbBackw() * CONF_NN_LMULT);		// quadratic decrease
  else
	  cur_lrate = CONF_NN_LRATE; // lrate_beg it will be modified in function of the performance on the development data
#endif
}

//----------------- main training process
void Process::nn_train_one_iter()
{
	time_t now;
	time(&now); //ctime is not rentrant ! use ctime_r() instead if needed
	cout << "##*** Start training for iter " << cur_iter << " at " << ctime(&now) << std::flush;
	each_prepare_data_oneiter();	/*************virtual****************/
	Timer ttrain;		// total training time
	ttrain.start();
	while(1){
		int n_size = mach->GetBsize();
		/*************virtual****************/
		REAL* xinput = each_next_data(&n_size);	//this may change n_size, all memory managed not here
		if(xinput){
			mach->SetDataIn(xinput);
			mach->Forw(n_size);
			//gradient for mach already set when prepare data
			each_get_grad(n_size);	/*************virtual****************/
			mach->Backw(cur_lrate, CONF_NN_WD, n_size);
			set_lrate();
		}
		else
			break;
	}

	ttrain.stop();
	ttrain.disp(" - training time: ");
	cout << "\n";
}

double Process::nn_dev_test(string to_test,string output,string gold)
{
	//also assuming test-file itself is gold file(this must be true with dev file)
	dev_test_corpus = read_corpus(to_test);
	if(! feat_gen){	//when testing
		feat_gen = new FeatureGenO1(dict,CONF_x_window,CONF_add_distance);
		if(CONF_pos_filter){
			feat_gen->read_extra_info(CONF_feature_file);
		}
	}
	feat_gen->deal_with_corpus(dev_test_corpus);

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


//restart conf files and also set sth
void Process::read_restart_conf()
{
	ifstream ifs(CONF_restart_file.c_str());
	if(!ifs){
		CTL_continue = 0;
		cur_iter = 0;
		cur_lrate = CONF_NN_LRATE;
	}
	else{
		CTL_continue = 1;
		ifs >> cur_iter >> cur_lrate;
	}
	printf("-- %d %d %g",CTL_continue,cur_iter,(double)cur_lrate);
	ifs.close();
	cout << endl;
}

void Process::write_restart_conf()
{
	ofstream ofs(CONF_restart_file.c_str());
	ofs << cur_iter << " " << cur_lrate << "\n";
	ofs.close();
}

void Process::delete_restart_conf()
{
	string cmd = "rm ";
	cmd += CONF_restart_file;
	cmd += ";";
	system(cmd.c_str());
}

vector<int>* Process::parse_o1(DependencyInstance* x)
{
	int idim = feat_gen->get_xdim();
	int odim = mach->GetOdim();
	//default order1 parsing
	int length = x->forms->size();
	double *tmp_scores = new double[length*length*2];
	//construct scores using nn
	int num_pair = length*(length-1);	//2 * (0+(l-1))*l/2
	REAL *mach_x = new REAL[num_pair*idim];
	REAL *mach_y = new REAL[num_pair*odim];
	REAL* assign_x = mach_x;
	for(int ii=0;ii<length;ii++){
		for(int j=ii+1;j<length;j++){
			for(int lr=0;lr<2;lr++){
				//build mach_x
				if(lr==E_RIGHT)
					feat_gen->fill_one(assign_x,x,ii,j);
				else
					feat_gen->fill_one(assign_x,x,j,ii);
				assign_x += idim;
			}
		}
	}
	//mach evaluate
	int remain = num_pair;
	int bsize = mach->GetBsize();
	REAL* xx = mach_x;
	REAL* yy = mach_y;
	while(remain > 0){
		int n=0;
		if(remain >= bsize)
			n = bsize;
		else
			n = remain;
		remain -= bsize;
		mach->SetDataIn(xx);
		mach->Forw(n);
		memcpy(yy, mach->GetDataOut(), odim*sizeof(REAL)*n);
		yy += n*odim;
		xx += n*idim;
	}
	REAL* assign_y = mach_y;
	FeatureGenO1* feat_o1 = (FeatureGenO1*)feat_gen;	//force it
	for(int ii=0;ii<length;ii++){
		for(int j=ii+1;j<length;j++){
			for(int lr=0;lr<2;lr++){
				int index = get_index2(length,ii,j,lr);
				//if filter --- this is the easy way(but waste computation)
				if(CONF_pos_filter){
					int head = ii, modif = j;
					if(lr==E_LEFT){
						head = j;
						modif = ii;
					}
					if(!feat_o1->allowed_pair(x->index_pos->at(head),x->index_pos->at(modif))){
						tmp_scores[index] = DOUBLE_LARGENEG;	//is this neg enough??
						continue;
					}
				}
				//important ...
				double temp = 0;
				if(odim > 1){
					for(int c=0;c<odim;c++)
						temp += (*assign_y++)*c;
					tmp_scores[index] = temp;
				}
				else
					tmp_scores[index] = *assign_y++;
			}
		}
	}
	vector<int> *ret = decodeProjective(length,tmp_scores);
	delete []mach_x;
	delete []mach_y;
	return ret;
}
