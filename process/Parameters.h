/*
 * common.h
 *
 *  Created on: Dec 19, 2014
 *      Author: zzs
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <iostream>
#include <vector>
#include <string>
using namespace std;

//well, maybe bad design, but convenient, ???
namespace parsing_conf{
//1.0
extern int CONF_method;	//which method
//1.1-files
extern string CONF_train_file;	//the training file
extern string CONF_dev_file;	//testing files
extern string CONF_test_file;	//testing files
extern string CONF_output_file;
extern string CONF_gold_file;	//golden files

//1.2-other files
extern string CONF_dict_file;		//for dictionary
extern string CONF_mach_name;		//mach name
extern string CONF_mach_conf_name;	//nn.conf
extern string CONF_mach_cur_suffix;
extern string CONF_mach_best_suffix;
extern string CONF_restart_file;		//recording the training iters
extern string CONF_feature_file;

//1.3-for nn
extern double CONF_NN_LRATE;
extern int CONF_NN_ITER;
extern double CONF_NN_LMULT;
extern double CONF_NN_WD;
extern double CONF_NN_hidden_size_portion;	//how much is hidden size
extern int CONF_NN_we;						//word-embedding size
extern int CONF_NN_plus_layers;				//plus number of layers(plus from base)
extern double CONF_NN_resample;				//re-sample rate
extern int CONF_NN_BS;						//block-size

//1.4-for parsing basis
extern int CONF_x_window;	//word and pos window size
extern int CONF_add_distance;	//whether add distance
extern int CONF_dict_remove;	//remove words appears only once
extern int CONF_pos_filter;		//add filters, with pairs seen before

//others
extern void init_configurations(string);

};

#define DOUBLE_LARGENEG -10000000.0		//maybe it is enough

#endif /* COMMON_H_ */
