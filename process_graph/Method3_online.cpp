/*
 * Method3_online.cpp
 *
 *  Created on: 9 Jan, 2015
 *      Author: z
 */

#include "Method3_online.h"
#include <cstdio>

#define GET_MAX_ONE(a,b) (((a)>(b))?(a):(b))
void Method3_online::each_prepare_data_oneiter()
{
	//*** In order to fill the bsize, multiple sentence one time
	if(data == 0){
		temp_data = new REAL[1000*mach->GetIdim()];	//1000 should be enough
		data = new REAL[1000*mach->GetIdim()];	//1000 should be enough, in fact should be bsize
		gradient = new REAL[mach->GetBsize()*mach->GetOdim()];
		mach->SetGradOut(gradient);
		cout << "--Ok, ready." << endl;
	}

		//nothing
		buffer_num = 0;
		curr_sentence = 0;
		all_sentence = training_corpus->size();
		all_tokens = correct_tokens = 0;
}

REAL* Method3_online::each_next_data(int* size)
{
	//easy way --- every-time copy from buffer
	int once_size = mach->GetIdim();
	while(buffer_num < *size){
		//get new sentence
		if(curr_sentence >= all_sentence)
			break;
		DependencyInstance* x = training_corpus->at(curr_sentence);
		curr_sentence++;
		//decode that
		vector<int>* res = each_test_one(x);
		REAL* assign_x = temp_data+buffer_num*once_size;
		for(int i=1;i<x->length();i++){
			all_tokens++;
			int guess = res->at(i);
			int right = x->heads->at(i);
			if(guess != right){
				//no filter here
				feat_gen->fill_one(assign_x,x,right,i);
				assign_x += once_size;
				feat_gen->fill_one(assign_x,x,guess,i);
				assign_x += once_size;
				buffer_num += 2;
			}
			else
				correct_tokens++;
		}
		delete res;
	}
	if(buffer_num == 0){
		//stat for training corpus
		printf("-For this:%d/%d - (%g),forward %d samples.\n",correct_tokens,all_tokens,
				(double)(correct_tokens)/all_tokens,2*(all_tokens-correct_tokens));
		return 0;
	}

	//at lease return sth
	memcpy(data,temp_data,(*size)*once_size*sizeof(REAL));
	if(buffer_num <= *size){	//the one dance
		*size = buffer_num;
		buffer_num = 0;
	}
	else{
		memcpy(temp_data,temp_data+(*size)*once_size,(buffer_num-*size)*once_size*sizeof(REAL));
		buffer_num -= *size;
	}
	return data;
}

void Method3_online::each_get_grad(int size)
{
	set_pair_gradient(mach->GetDataOut(),gradient,size);
}

