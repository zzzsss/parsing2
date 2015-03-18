/*
 * Process.h
 *
 *  Created on: 2015��3��18��
 *      Author: zzs
 */

#ifndef PROCESS_GRAPH_PROCESS_H_
#define PROCESS_GRAPH_PROCESS_H_

#include "../parts/Parameters.h"
#include "../parts/FeatureGen.h"
#include "../parts/FeatureGenO1.h"
#include "../parts/FeatureGenO2sib.h"
#include "../parts/Dict.h"
#include "../tools/CONLLReader.h"
#include "../tools/DependencyEvaluator.h"
#include "../cslm/Mach.h"
#include "../cslm/MachConfig.h"
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>
using namespace std;

class Process{
protected:
	//data
	parsing_conf* parameters;
	FeatureGen* feat_gen;
	Dict* dict;
	vector<DependencyInstance*>* training_corpus;
	vector<DependencyInstance*>* dev_test_corpus;

	//read from restart file or from scratch
	REAL cur_lrate;
	int cur_iter;
	int CTL_continue;	//if continue training
	double * dev_results;	//the results of dev-data
	Mach *mach;

	//some procedures
	void set_lrate();					//no schedule, just decrease lrate
	int set_lrate_one_iter();	//lrate schedule
	//init embedings
	virtual void init_embed();
	//restart files
	void read_restart_conf();
	void write_restart_conf();
	void delete_restart_conf();
	void write_conf(int);	//maybe useful

	//help
	static void shuffle_data(REAL* x,REAL* y,int xs,int ys,int xall,int yall,int times);
	static void set_softmax_gradient(const REAL* s_target,const REAL* s_output,REAL* s_gradient,int bsize,int c);
	static void set_pair_gradient(const REAL* s_output,REAL* s_gradient,int bsize);
	static REAL* mach_forward(Mach* m,REAL* assign,int all);
	//parse
	static double* get_scores_o1(DependencyInstance* x,parsing_conf* zp,Mach* zm,FeatureGen* zf);		//double[l*l]
	static double* get_scores_o2sib(DependencyInstance* x,parsing_conf* zp,Mach* zm,FeatureGen* zf);	//double[l*l*l]
	vector<int>* parse_o1(DependencyInstance* x);
	vector<int>* parse_o2sib(DependencyInstance* x,double* score_of_o1=0);

	//train and test
	double nn_dev_test(string to_test,string output,string gold);
	void nn_train_one_iter();
	void nn_train_prepare();

	//virtual functions for different methods
	virtual void each_write_mach_conf(){};
	virtual void each_prepare_data_oneiter()=0;
	virtual REAL* each_next_data(int*)=0;
	virtual void each_get_grad(int)=0;
	virtual vector<int>* each_test_one(DependencyInstance* x){
		//default o1
		return parse_o1(x);
	}
	virtual void each_get_featgen(int if_testing){
		// default only the order1 features
		if(if_testing){
			if(! feat_gen){	//when testing
				feat_gen = new FeatureGenO1(dict,parameters->CONF_x_window,parameters->CONF_add_distance,parameters->CONF_add_pos);
				if(parameters->CONF_pos_filter){
					feat_gen->read_extra_info(parameters->CONF_feature_file);
				}
			}
			feat_gen->deal_with_corpus(dev_test_corpus);
		}
		else{
			feat_gen = new FeatureGenO1(dict,parameters->CONF_x_window,parameters->CONF_add_distance,parameters->CONF_add_pos);
			feat_gen->deal_with_corpus(training_corpus);
			if(parameters->CONF_pos_filter){
				feat_gen->add_filter(training_corpus);
				feat_gen->write_extra_info(parameters->CONF_feature_file);
			}
		}
	}

public:
	Process(string);
	virtual ~Process(){}
	void train();
	void test(Mach*,Dict*);
};


#endif /* PROCESS_GRAPH_PROCESS_H_ */