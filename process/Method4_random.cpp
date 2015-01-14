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
void Method4_random::each_prepare_data_oneiter()
{
	//only shuffle the corpus(here we ignore resample)
	//simple method --- once one sentence

	if(temp_data == 0){
		temp_data = new REAL[1000*mach->GetIdim()];	//1000 should be enough
		gradient = new REAL[mach->GetBsize()*mach->GetOdim()];
		mach->SetGradOut(gradient);
		cout << "--Ok, ready." << endl;
	}

		//nothing
		this_num = curr_num = 0;
		curr_sentence = 0;
		all_sentence = training_corpus->size();

}

REAL* Method4_random::each_next_data(int* size)
{
	//later iters --- depend on vars of Method3
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

		for(int i=1;i<x->length();i++){
			int guess = i;
			int right = x->heads->at(i);
			while(guess==i || guess==right)
				guess = rand()%(x->length());	//waht if only one token, nope?? --- yes

			if(1){
				//no filter here
				feat_gen->fill_one(assign_x,x,right,i);
				assign_x += mach->GetIdim();
				feat_gen->fill_one(assign_x,x,guess,i);
				assign_x += mach->GetIdim();
				this_num += 2;
			}
		}
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

