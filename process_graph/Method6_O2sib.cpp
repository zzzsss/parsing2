/*
 * Method6_O2sib.cpp
 *
 *  Created on: 2015Äê3ÔÂ9ÈÕ
 *      Author: zzs
 */

#include "Method6_O2sib.h"

#define GET_MAX_ONE(a,b) (((a)>(b))?(a):(b))
#define GET_MIN_ONE(a,b) (((a)>(b))?(b):(a))
void Method6_O2sib::each_prepare_data_oneiter()
{
	//NOPE--no-use
	//M6 just just exist for m7...
}

REAL* Method6_O2sib::each_next_data(int* size)
{
	//size must be even number, if no universal rays, it should be that...
	if(current >= end)
		return 0;
	if(current + *size > end)
		*size = end - current;
	//!!not adding current here
	return (data+current*mach->GetIdim());
}

void Method6_O2sib::each_get_grad(int size)
{
	set_pair_gradient(mach->GetDataOut(),gradient,size);
	//!! here add it
	current += size;
}

vector<int>* Method6_O2sib::each_test_one(DependencyInstance* x)
{
	vector<int>* ret;
	//combine o1 scores
	if(parameters->CONF_NN_highO_o1mach.length() > 0 &&
			(parameters->CONF_NN_highO_score_combine || parameters->CONF_NN_highO_o1filter)){
		FeatureGenO1* feat_temp_o1 = new FeatureGenO1(dict,parameters->CONF_x_window,
				parameters->CONF_add_distance,parameters->CONF_add_pos,parameters->CONF_add_distance_parent);
		double* scores_o1 = get_scores_o1(x,parameters,mach_o1,feat_temp_o1);	//same parameters
		ret = parse_o2sib(x,scores_o1);
		delete []scores_o1;
		delete feat_temp_o1;
	}
	else{
		ret = parse_o2sib(x);
	}
	return ret;
}

//maybe init embedding from o1 machine
void Method6_O2sib::init_embed()
{
	if(parameters->CONF_NN_highO_o1mach.length() > 0 && parameters->CONF_NN_highO_embed_init){
		//special structure
		int all = parameters->CONF_NN_we * dict->get_count();
		mach->clone_tab(mach_o1->get_tab(),all);
	}
	else
		Process::init_embed();
}
