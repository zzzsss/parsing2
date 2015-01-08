/*
 * Method1_allC.cpp
 *
 *  Created on: Dec 25, 2014
 *      Author: zzs
 */

#include "Method1_allC.h"

void Method1_allC::each_write_mach_conf()
{
	write_conf(2);
}

void Method1_allC::each_prepare_data_oneiter()
{
	delete []data;
	delete []target;
	delete []gradient;

	//for gradient
	gradient = new REAL[mach->GetBsize()*mach->GetOdim()];
	mach->SetGradOut(gradient);

	//prepare all
	//-- first all
	int num_pairs = 0;
	int sentences = training_corpus->size();
	for(int i=0;i<sentences;i++){
		int length = training_corpus->at(i)->length();
		num_pairs += length*(length-1);
	}
	//-- generate all
	int real_num_pairs = 0;
	data = new REAL[num_pairs*mach->GetIdim()];
	target = new REAL[num_pairs];
	REAL* assign_x = data;
	REAL* assign_y = target;
	FeatureGenO1* feat_o1 = (FeatureGenO1*)feat_gen;	//force it
	for(int i=0;i<sentences;i++){
		DependencyInstance* x = training_corpus->at(i);
		int length = x->length();
		for(int ii=0;ii<length;ii++){
			for(int j=ii+1;j<length;j++){
				for(int lr=0;lr<2;lr++){
					//build mach_x
					REAL t = 0;
					int head = ii, mod = j;
					if(lr==E_LEFT){
						head = j;
						mod = ii;
					}
					//check filter if set
					if(CONF_pos_filter){
						if(!feat_o1->allowed_pair(x->index_pos->at(head),x->index_pos->at(mod)))
							continue;
					}
					feat_gen->fill_one(assign_x,x,head,mod);
					if(x->heads->at(mod)==head)
						t=1;

					*assign_y = t;
					assign_x += mach->GetIdim();
					assign_y += 1;
					real_num_pairs++;
				}
			}
		}
	}
	current = 0;
	end = real_num_pairs;
	//shuffle
	shuffle_data(data,target,mach->GetIdim(),1,real_num_pairs*mach->GetIdim(),real_num_pairs,10);
	//sample
	cout << "--Data for this iter: samples all " << end << " resample: " << (int)(end*CONF_NN_resample) << endl;
	end = (int)(end*CONF_NN_resample);
}

REAL* Method1_allC::each_next_data(int* size)
{
	if(current >= end)
		return 0;
	if(current + *size > end)
		*size = end - current;
	//!!not adding current here
	return (data+current*mach->GetIdim());
}

void Method1_allC::each_get_grad(int size)
{
	set_softmax_gradient(target+current,mach->GetDataOut(),gradient,size,mach->GetOdim());
	//!! here add it
	current += size;
}
