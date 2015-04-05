/*
 * Method10_allC.cpp
 *
 *  Created on: 2015Äê4ÔÂ5ÈÕ
 *      Author: zzs
 */

#include "Method10_allC.h"

void Method10_allC::each_prepare_data_oneiter()
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
	int right_ones = 0;
	data = new REAL[num_pairs*mach->GetIdim()];
	target = new REAL[num_pairs];
	REAL* assign_x = data;
	REAL* assign_y = target;
	FeatureGenO1* feat_o1 = (FeatureGenO1*)feat_gen;	//force it
	for(int i=0;i<sentences;i++){
		DependencyInstance* x = training_corpus->at(i);
		int length = x->length();
		for(int ii=0;ii<length;ii++){
			for(int j=0;j<length;j++){
				if(ii != j){
					//build mach_x
					REAL t = 0;
					//check filter if set
					if(parameters->CONF_pos_filter && feat_gen->has_filter()){
						if(!feat_o1->allowed_pair(x,ii,j))
							continue;
					}
					feat_gen->fill_one(assign_x,x,ii,j);
					if(x->heads->at(j)==ii){
						right_ones++;
						t=1;
					}
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
	gright = 1.0 * (end/2) / (right_ones);
	gwrong = -1.0  * (end/2) / (end-right_ones);
	cout << "--M10, Data for this iter: samples all " << end << " resample: " << (int)(end*parameters->CONF_NN_resample)
			<< "right-gradient: " << gright << "right-gradient: " << gwrong << endl;
	end = (int)(end*parameters->CONF_NN_resample);
}

REAL* Method10_allC::each_next_data(int* size)
{
	if(current >= end)
		return 0;
	if(current + *size > end)
		*size = end - current;
	//!!not adding current here
	return (data+current*mach->GetIdim());
}

void Method10_allC::each_get_grad(int size)
{
	REAL* x = mach->GetDataOut();
	REAL* t = target + current;
	for(int i=0;i<size;i++){
		if(t[i]==1 && x[i]>1 || t[i]==0 && x[i]<-1)
			gradient[i] = 0;
		else
			gradient[i] = ((t[i]==0) ? gwrong : gright);
	}
	//!! here add it
	current += size;
}

