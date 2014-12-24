/*
 * Process.h
 *
 *  Created on: Dec 24, 2014
 *      Author: zzs
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include "Parameters.h"
#include "../tools/CONLLReader.h"
#include <cstdlib>
using namespace parsing_conf;

//this base class is coupled with configurations
class Process{
protected:
	//corpus
	vector<DependencyInstance*>* training_corpus;
	vector<DependencyInstance*>* dev_test_corpus;
	Dict* dict;

	//read from restart file or from scratch
	REAL cur_lrate;
	int cur_iter;
	int CTL_continue;	//if continue training

	//mach training
	Mach *mach;
	void set_lrate();
	void nn_train_one_iter();
	void nn_dev_test(string to_test,string output,string gold);

	//virtual functions for different methods
	virtual void each_write_mach_conf()=0;

	//restart files
	void read_restart_conf();
	void write_restart_conf();
	void delete_restart_conf();
public:
	void train();
	void test();
	virtual ~Process(){}
};

#endif /* PROCESS_H_ */
