/*
 * Method4_random.cpp
 *
 *  Created on: 14 Jan, 2015
 *      Author: z
 */

#include "Method4_random.h"
#include <cstdio>

void Method4_random::each_write_mach_conf()
{
	//only one output --- the score
	write_conf(1);
}

#define GET_MAX_ONE(a,b) (((a)>(b))?(a):(b))
#define GET_MIN_ONE(a,b) (((a)>(b))?(b):(a))
void Method4_random::each_prepare_data_oneiter()
{
	//only shuffle the corpus(here we ignore resample)
	//simple method --- once one sentence

	if(temp_data == 0){
		temp_data = new REAL[100000*mach->GetIdim()];	//100000 should be enough??
		gradient = new REAL[mach->GetBsize()*mach->GetOdim()];
		mach->SetGradOut(gradient);
		cout << "--Ok, ready for random style1." << endl;
	}

		//nothing
		this_num = curr_num = 0;
		curr_sentence = 0;
		all_sentence = training_corpus->size();

}

REAL* Method4_random::each_next_data(int* size)
{
	while(this_num <= 0){
		//get new sentence
		if(curr_sentence >= all_sentence){
			return 0;
		}
		DependencyInstance* x = training_corpus->at(curr_sentence);
		curr_sentence++;
		this_num = curr_num = 0;

		REAL* assign_x = temp_data;

		if(x->length() <= 2)
			continue;

		//multiple use of CONF_NN_resample
		int one_len = x->length();
		int one_sample = (one_len-2)*parameters->CONF_NN_resample;
		if(parameters->CONF_NN_resample > 0.9999)//well...
			one_sample = (int)(parameters->CONF_NN_resample+0.01);
		else if(one_sample==0)
			one_sample = 1;	//at least one
		one_sample = GET_MIN_ONE(one_sample,one_len-2);

		bool *one_already = new bool[one_len];
		bool *one_already_child = new bool[one_len];
		for(int i=1;i<one_len;i++){
			int guess = i;
			int right = x->heads->at(i);

			int one_already_child_left = -1;	//exclude the parent itself
			for(int j=0;j<one_len;j++){
				one_already[j] = false;
				if(x->heads->at(j) == right)
					one_already_child[j] = true;
				else{
					one_already_child[j] = false;
					one_already_child_left++;
				}
			}
			one_already[i] = one_already[right] = true;
			one_already_child[i] = one_already_child[right] = true;
			int one_sample_get = 0;

			while(one_sample_get < one_sample ){
				one_sample_get++;

				guess = rand()%(one_len);
				while(one_already[guess])
					guess = rand()%(one_len);	//what if only one token, nope?? --- yes
				//no filter here
				feat_gen->fill_one(assign_x,x,right,i);
				assign_x += mach->GetIdim();
				feat_gen->fill_one(assign_x,x,guess,i);
				assign_x += mach->GetIdim();
				this_num += 2;
				one_already[guess] = true;

				//change child and keep parent
				if(parameters->CONF_NN_example && one_already_child_left>0){
					guess = rand()%(one_len);
					while(one_already_child[guess])
						guess = rand()%(one_len);
					//no filter here
					feat_gen->fill_one(assign_x,x,right,i);
					assign_x += mach->GetIdim();
					feat_gen->fill_one(assign_x,x,right,guess);
					assign_x += mach->GetIdim();
					this_num += 2;
					one_already_child[guess] = true;
					one_already_child_left--;
				}
			}
		}
		delete []one_already;
		delete []one_already_child;
		break;
	}
	//get data
	if(curr_num + *size >= this_num){	//maybe most will hit here unless quite long sentences
		*size = this_num - curr_num;
		this_num = -1;
		return (temp_data+curr_num*mach->GetIdim());
	}
	else{
		curr_num += *size;
		return (temp_data+(curr_num-*size)*mach->GetIdim());
	}
}

void Method4_random::each_get_grad(int size)
{
	set_pair_gradient(mach->GetDataOut(),gradient,size);
}

