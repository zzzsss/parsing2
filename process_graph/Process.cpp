/*
 * Process.cpp
 *
 *  Created on: 2015Äê3ÔÂ18ÈÕ
 *      Author: zzs
 */

#include "Process.h"

Process::Process(string conf)
{
	//1.conf
	cout << "1.configuration:" << endl;
	parameters = new parsing_conf(conf);
	feat_gen = 0;
}

//init --- right after construction, but here use some virtual functions
void Process::nn_train_prepare()
{
	dev_results = new double[parameters->CONF_NN_ITER];
	//2. get training corpus --- configured for training
	cout << "2.read-corpus:" << endl;
	training_corpus = read_corpus(parameters->CONF_train_file);
	dev_test_corpus = 0;
	//2.5 if continue/restart --- also init some values ---- CAN ONLY CONTINUE WITH ONE MACH METHOD
	cout << "2.5:let's see whether to continue" << endl;
	read_restart_conf();
	//3. get dictionary --- whether continue to train
	if(CTL_continue){
		cout << "3.get dict from file "<< parameters->CONF_dict_file << endl;
		dict = new Dict(parameters->CONF_dict_file);
	}
	else{
		cout << "3.get dict from scratch:" << endl;
		dict = new Dict(parameters->CONF_dict_remove,parameters->CONF_add_distance,parameters->CONF_oov_backoff);
		dict->construct_dictionary(training_corpus);
		dict->write(parameters->CONF_dict_file);
	}
	//3.5 get the feature generator
	each_get_featgen(0);			/*************virtual****************/
	//4. get machine
	string mach_cur_name = parameters->CONF_mach_name+parameters->CONF_mach_cur_suffix;
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
		each_write_mach_conf();		/*************virtual****************/
		//for mach_config
		char *argv[2];
		argv[0] = "nn";
		argv[1] = (char*)parameters->CONF_mach_conf_name.c_str();
		mach_config.parse_options(2,argv);
	    mach = mach_config.get_machine();
	    if(mach == 0)
	    	Error(mach_config.get_error_string().c_str());
	    //if init embed
	    init_embed();
	}
	cout << "-Prepare over..." << endl;
}


//restart conf files and also set sth
void Process::read_restart_conf()
{
	ifstream ifs(parameters->CONF_restart_file.c_str());
	if(!ifs){
		CTL_continue = 0;
		cur_iter = 0;
		cur_lrate = parameters->CONF_NN_LRATE;
	}
	else{
		CTL_continue = 1;
		ifs >> cur_iter >> cur_lrate;
		for(int i=0;i<cur_iter;i++)
			ifs >> dev_results[i];
	}
	printf("-- %d %d %g",CTL_continue,cur_iter,(double)cur_lrate);
	ifs.close();
	cout << endl;
}

void Process::write_restart_conf()
{
	ofstream ofs(parameters->CONF_restart_file.c_str());
	ofs << cur_iter << " " << cur_lrate << "\n";
	for(int i=0;i<cur_iter;i++)
		ofs << dev_results[i] << "\n";
	ofs.close();
}

void Process::delete_restart_conf()
{
	string cmd = "rm ";
	cmd += parameters->CONF_restart_file;
	cmd += ";";
	system(cmd.c_str());
}

//**************************************************************************************
// default lrate = mach->lrate_begin / (1.0 + total_n_ex_seen * mach->lrate_mult);
// default wdecay: constant

void Process::set_lrate()
{
#ifdef LRATE_HACK
  lrate = lrate_beg - mach->GetNbForw() * lrate_mult;
#else
  if (parameters->CONF_NN_LMULT>0)
	  cur_lrate = parameters->CONF_NN_LRATE / (1.0 + mach->GetNbBackw() * parameters->CONF_NN_LMULT);		// quadratic decrease
  //else cur_lrate = CONF_NN_LRATE; // lrate_beg it will be modified in function of the performance on the development data, no change here
#endif
}

int Process::set_lrate_one_iter()
{
	if(parameters->CONF_NN_LMULT<0 && cur_iter>0){
		//special schedule in (-1,0)
		if(parameters->CONF_NN_LMULT > -1){
			if(dev_results[cur_iter] < dev_results[cur_iter-1])
				cur_lrate *= (-1 * parameters->CONF_NN_LMULT);
		}
	}
	return 1;
}

#define WRITE_CONF_ONE(a1,a2) \
	fout << parameters->CONF_NN_act << " = " << (int)(a1) << "x" << (int)(a2) << " fanio-init-weights=1.0\n";

void Process::write_conf(int c)
{
	ofstream fout(parameters->CONF_mach_conf_name.c_str());
	fout << "block-size = " << parameters->CONF_NN_BS << "\n";
	if(parameters->CONF_NN_drop>0)
		fout << "drop-out = " << parameters->CONF_NN_drop << "\n";
	int xdim = feat_gen->get_xdim();
	int width = xdim*parameters->CONF_NN_we;
	//projection layer
	fout << "[machine]\n" << "Sequential = \n" << "Parallel = \n";
	for(int i=0;i<xdim;i++)
		fout << "Tab = " << dict->get_count() << "x" << parameters->CONF_NN_we << "\n";
	fout << "#End\n";
	if(parameters->CONF_NN_h_size==0){
		//hidden layer1
		if(parameters->CONF_NN_hidden_size_portion <= 1){
			WRITE_CONF_ONE(width,width*parameters->CONF_NN_hidden_size_portion);
			//fout << "Tanh = " << width << "x" << (int)(width*CONF_NN_hidden_size_portion) << " fanio-init-weights=1.0\n";
			width = (int)(width*parameters->CONF_NN_hidden_size_portion);
			//more hidden layers??
			for(int i=0;i<parameters->CONF_NN_plus_layers;i++){
				WRITE_CONF_ONE(width,width*parameters->CONF_NN_hidden_size_portion);
				//fout << "Tanh = " << width << "x" << (int)(width*CONF_NN_hidden_size_portion) << " fanio-init-weights=1.0\n";
				width = (int)(width*parameters->CONF_NN_hidden_size_portion);
			}
		}
		else{
			WRITE_CONF_ONE(width,parameters->CONF_NN_hidden_size_portion);
			//fout << "Tanh = " << width << "x" << (int)(CONF_NN_hidden_size_portion) << " fanio-init-weights=1.0\n";
			width = parameters->CONF_NN_hidden_size_portion;
			for(int i=0;i<parameters->CONF_NN_plus_layers;i++)
				WRITE_CONF_ONE(width,width);
				//fout << "Tanh = " << width << "x" << width << " fanio-init-weights=1.0\n";
		}
	}
	else{
		//hidden layer1
		WRITE_CONF_ONE(width,parameters->CONF_NN_h_size[0]);
		//fout << "Tanh = " << width << "x" << CONF_NN_h_size[0] << " fanio-init-weights=1.0\n";
		//more hidden layers??
		for(int i=0;i<parameters->CONF_NN_plus_layers;i++){
			WRITE_CONF_ONE(parameters->CONF_NN_h_size[i],parameters->CONF_NN_h_size[i+1]);
			//fout << "Tanh = " << CONF_NN_h_size[i] << "x" << CONF_NN_h_size[i+1] << " fanio-init-weights=1.0\n";
		}
		width = parameters->CONF_NN_h_size[parameters->CONF_NN_plus_layers];
	}
	if(c>1){
		//output multiclass-class(0 or 1)
		fout << "Softmax = " << width << "x" << c << " fanio-init-weights=1.0\n";
	}
	else{
		//linear output score
		fout << "Linear = " << width << "x" << 1 << " fanio-init-weights=1.0\n";
	}
	fout << "#End\n";
	fout.close();
}
