/*
 * Parameters.cpp
 *
 *  Created on: Dec 24, 2014
 *      Author: zzs
 */

#include "Parameters.h"
#include <fstream>
#include <cstdio>

namespace parsing_conf{
//1.0
int CONF_method;	//which method
//1.1-files
string CONF_train_file;	//the training file
string CONF_dev_file;	//testing files
string CONF_test_file;	//testing files
string CONF_output_file="output.txt";
string CONF_gold_file;	//golden files

//1.2-other files
string CONF_dict_file="vocab.dict";		//for dictionary
string CONF_mach_name="nn.mach";		//mach name
string CONF_mach_conf_name="nn.conf";	//nn.conf
string CONF_mach_cur_suffix=".curr";
string CONF_mach_best_suffix=".best";
string CONF_restart_file="nn.restart";		//recording the training iters
string CONF_feature_file="nn.feat";

//1.3-for nn
double CONF_NN_LRATE=0.045;
int CONF_NN_ITER=20;
double CONF_NN_LMULT=1e-9;
double CONF_NN_WD=3e-5;
double CONF_NN_hidden_size_portion=0.2;	//how much is hidden size
int CONF_NN_we=50;						//word-embedding size
int CONF_NN_plus_layers=0;				//plus number of layers(plus from base)
double CONF_NN_resample=1.0;				//re-sample rate
int CONF_NN_BS=128;						//block-size

//1.4-for parsing basis
int CONF_x_window=5;	//word and pos window size
int CONF_add_distance=1;	//whether add distance
int CONF_dict_remove=0;	//remove words appears only once
int CONF_pos_filter=0;		//add filters, with pairs seen before

//others
void init_configurations(string conf_file)
{
#define DATA_LINE_LEN 10000
	ifstream fin(conf_file.c_str());
	cout << "Dealing configure file '" << conf_file << "'" << endl;
	fin >> CONF_method;
	while(!fin.eof()){
		string buf;
		char line[DATA_LINE_LEN];
		fin >> buf;
		if (buf=="") continue; // HACK
		if (buf[0]=='#') {fin.getline(line, DATA_LINE_LEN); continue;} // skip comments
		//1.1
		if(buf=="train")		fin >> CONF_train_file;
		else if(buf=="dev")		fin >> CONF_dev_file;
		else if(buf=="test")	fin >> CONF_test_file;
		else if(buf=="output")	fin >> CONF_output_file;
		else if(buf=="gold")	fin >> CONF_gold_file;
		//1.2
		//1.3
		else if(buf=="nn_lrate") fin >> CONF_NN_LRATE;
		else if(buf=="nn_iters") fin >> CONF_NN_ITER;
		else if(buf=="nn_lmult") fin >> CONF_NN_LMULT;
		else if(buf=="nn_wd")	 fin >> CONF_NN_WD;
		else if(buf=="nn_h_por") fin >> CONF_NN_hidden_size_portion;
		else if(buf=="nn_we")	fin >> CONF_NN_we;
		else if(buf=="nn_plusl") fin >> CONF_NN_plus_layers;
		else if(buf=="nn_resample") fin >> CONF_NN_resample;
		else if(buf=="nn_bs") 	 fin >> CONF_NN_BS;
		//1.4
		else if(buf=="f_xwin") 	fin >> CONF_x_window;
		else if(buf=="f_distance") fin >> CONF_add_distance;
		else if(buf=="f_removes") fin >> CONF_dict_remove;
		else if(buf=="f_filter") fin >> CONF_pos_filter;
		else
			cout << "Unknown conf " << buf << endl;
	}

	//the configurations
	printf("The configurations:\n");
	printf("Data files: %s,%s,%s,%s,%s\n",CONF_train_file.c_str(),CONF_dev_file.c_str(),
			CONF_test_file.c_str(),CONF_output_file.c_str(),CONF_gold_file.c_str());
	printf("NN: lrate(%g),iters(%d),lmult(%g),wdecay(%g),hidden_por(%g),word_esize(%d),plus_layers(%d),resample(%g),bsize(%d)",
			CONF_NN_LRATE,CONF_NN_ITER,CONF_NN_LMULT,CONF_NN_WD,CONF_NN_hidden_size_portion,CONF_NN_we,CONF_NN_plus_layers,
			CONF_NN_resample,CONF_NN_BS);
	printf("Feature: xwindow(%d),distance(%d),removes(%d),filter(%d)\n",
			CONF_x_window,CONF_add_distance,CONF_dict_remove,CONF_pos_filter);
}

};
