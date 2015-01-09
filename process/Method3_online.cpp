/*
 * Method3_online.cpp
 *
 *  Created on: 9 Jan, 2015
 *      Author: z
 */

#include "Method3_online.h"

void Method3_online::each_write_mach_conf()
{
	//only one output --- the score
	write_conf(1);
}

#define GET_MAX_ONE(a,b) (((a)>(b))?(a):(b))
void Method3_online::each_prepare_data_oneiter()
{
	//only shuffle the corpus(here we ignore resample)
	//simple method --- once one sentence

	if(temp_data == 0){
		gradient = new REAL[mach->GetBsize()*mach->GetOdim()];
		mach->SetGradOut(gradient);
		max_num = 0;
		int num_pairs = 0;
		int sentences = training_corpus->size();
		for(int i=0;i<sentences;i++){
			int length = training_corpus->at(i)->length();
			//length-1
			num_pairs += (length-1)*2;
			max_num = GET_MAX_ONE(max_num,(length-1)*2);
		}
		temp_data = new REAL[max_num*mach->GetIdim()];
		cout << "Preparing data once, full will be " << num_pairs << endl;
	}

	//shuffle
	cout << "--shuffle data " << 10 << " times.";
	curr_sentence = 0;
	all_sentence = training_corpus->size();
	for(int i=0;i<10;i++){	//shuffle 10 times
		for(int t=0;t<all_sentence;t++){
			int which = t+(rand()%(all_sentence-t));
			if(which==t)
				continue;
			DependencyInstance* temp = training_corpus->at(t);
			(*training_corpus)[t] = training_corpus->at(which);
			(*training_corpus)[which] = temp;
		}
	}
	cout << " -- Done." << endl;

	//ready to start
	curr_num = 0;
	this_num = -1;
}

REAL* Method3_online::each_next_data(int* size)
{
	if(this_num <= 0){
		//get new sentence
		if(curr_sentence >= all_sentence)
			return 0;
		DependencyInstance* x = training_corpus->at(curr_sentence);
		curr_sentence++;
		this_num = curr_num = 0;
		//decode that
		vector<int>* res = each_test_one(x);
		REAL* assign_x = temp_data;
		for(int i=1;i<x->length();i++){
			int guess = res->at(i);
			int right = x->heads->at(i);
			if(guess != right){
				//no filter here
				feat_gen->fill_one(assign_x,x,right,i);
				assign_x += mach->GetIdim();
				feat_gen->fill_one(assign_x,x,guess,i);
				assign_x += mach->GetIdim();
				this_num += 2;
			}
		}
		delete res;
		//if lucky to get all right --- use tail recursion
		if(this_num==0)
			return each_next_data(size);
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

void Method3_online::each_get_grad(int size)
{
	set_pair_gradient(mach->GetDataOut(),gradient,size);
}


