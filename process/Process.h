/*
 * Process.h
 *
 *  Created on: Dec 24, 2014
 *      Author: zzs
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include "Parameters.h"
#include "FeatureGen.h"
#include "FeatureGenO1.h"
#include "../tools/Eisner.h"
#include "../tools/CONLLReader.h"
#include "../tools/DependencyEvaluator.h"
#include "../cslm/Mach.h"
#include "../cslm/MachConfig.h"
#include <cstdlib>
#include <cstring>
using namespace parsing_conf;

//this base class is coupled with configurations
class Process{
protected:
	//corpus
	vector<DependencyInstance*>* training_corpus;
	vector<DependencyInstance*>* dev_test_corpus;
	FeatureGen* feat_gen;
	Dict* dict;

	//read from restart file or from scratch
	REAL cur_lrate;
	int cur_iter;
	int CTL_continue;	//if continue training

	//mach training
	Mach *mach;
	void set_lrate();
	void nn_train_one_iter();
	double nn_dev_test(string to_test,string output,string gold);

	//virtual functions for different methods
	virtual void each_write_mach_conf()=0;
	virtual void each_prepare_data_oneiter()=0;
	virtual REAL* each_next_data(int*)=0;
	virtual void each_get_grad(int)=0;

	virtual vector<int>* each_test_one(DependencyInstance* x){
		//for now
		return parse_o1(x);
	}

	//restart files
	void read_restart_conf();
	void write_restart_conf();
	void delete_restart_conf();
public:
	void train();
	void test(Mach*,Dict*);
	virtual ~Process(){}
	Process(){
		feat_gen = 0;
		mach = 0;
		dict = 0;
	}

	//help
	static void shuffle_data(REAL* x,REAL* y,int xs,int ys,int xall,int yall,int times);
	static void set_softmax_gradient(const REAL* s_target,const REAL* s_output,REAL* s_gradient,int bsize,int c);

protected:
	//useful functions
	//parsing
	vector<int>* parse_o1(DependencyInstance*);
	//write nn_conf
	void write_conf(int c);
};

#endif /* PROCESS_H_ */
