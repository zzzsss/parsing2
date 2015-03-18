/*
 * Method6_O2sib.h
 *
 *  Created on: 2015Äê3ÔÂ9ÈÕ
 *      Author: zzs
 */
#ifndef PARSING2_PROCESS_METHOD6_H_
#define PARSING2_PROCESS_METHOD6_H_

#include "Process.h"

//Method 6 : order2, siblings
//also pairwise like M2~5
class Method6_O2sib: public Process{
protected:
	REAL* data;
	int current;
	int end;
	REAL* gradient;

	virtual void each_write_mach_conf();
	virtual void each_prepare_data_oneiter();
	virtual REAL* each_next_data(int*);
	virtual void each_get_grad(int);
public:
	Method6_O2sib(string conf):Process(conf){
		current = end = 0;
		data = 0;
		gradient = 0;
	}
	virtual void each_get_featgen(int if_testing){
		if(if_testing){
			if(! feat_gen)	//when testing
				feat_gen = new FeatureGenO2sib(dict,parameters->CONF_x_window,parameters->CONF_add_distance,parameters->CONF_add_pos);
			if(parameters->CONF_pos_filter){
				feat_gen->read_extra_info(parameters->CONF_feature_file);
			}
			feat_gen->deal_with_corpus(dev_test_corpus);
		}
		else{
			feat_gen = new FeatureGenO2sib(dict,parameters->CONF_x_window,parameters->CONF_add_distance,parameters->CONF_add_pos);
			feat_gen->deal_with_corpus(training_corpus);
			if(parameters->CONF_pos_filter){
				feat_gen->add_filter(training_corpus);
				feat_gen->write_extra_info(parameters->CONF_feature_file);
			}
		}
	}
	virtual vector<int>* each_test_one(DependencyInstance* x);
};



#endif
