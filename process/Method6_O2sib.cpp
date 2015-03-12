/*
 * Method6_O2sib.cpp
 *
 *  Created on: 2015Äê3ÔÂ9ÈÕ
 *      Author: zzs
 */

#include "Method6_O2sib.h"

void Method6_O2sib::each_write_mach_conf()
{
	//only one output --- the score
	write_conf(1);
}

#define GET_MAX_ONE(a,b) (((a)>(b))?(a):(b))
#define GET_MIN_ONE(a,b) (((a)>(b))?(b):(a))
void Method6_O2sib::each_prepare_data_oneiter()
{
	delete []data;
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
		num_pairs += 2*length*4;	//change the 3 items
	}
	if(((int)CONF_NN_resample)>=1){
		num_pairs *= (int)CONF_NN_resample;	//multiple
	}
	int real_num_pairs = 0;
	data = new REAL[num_pairs*mach->GetIdim()];
	REAL* assign_x = data;
	for(int i=0;i<sentences;i++){
		DependencyInstance* x = training_corpus->at(i);
		int len_t = x->length();
		//add each head
		// really simple implement for multiple ones
		for(int times=0;times<(int)CONF_NN_resample;times++){
		for(int h=0;h<len_t;h++){
			int before = -1;
			for(int c=h-1;c>=0;c--){
				if(x->heads->at(c) == h){
					int head_bound = GET_MAX_ONE(c,before);
					int child_bound = h;
					if(before>0)
						child_bound = before;
					//start
					int nodes_left = 0;
					//1.change head
					nodes_left = len_t - 1 - head_bound;
					if(nodes_left > 1){
						int guess = h;while((guess=rand()%nodes_left+head_bound+1)==h);
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,guess,c,before);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					//2.change child
					nodes_left = child_bound;
					if(nodes_left > 1){
						int guess = c;while((guess=rand()%nodes_left)==c);
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,h,guess,before);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					//3.change center
					nodes_left = h-c-1;
					if(nodes_left > 1){
						int guess = before;while((guess=rand()%nodes_left+c+1)==before);
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,h,c,guess);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					//4.possibly no center
					if(before>0){
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,h,c,-1);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					before = c;
				}
			}
			before = -1;
			for(int c=h+1;c<len_t;c++){
				if(x->heads->at(c) == h){
					int child_bound = GET_MAX_ONE(h,before);
					int head_bound = c;
					if(before>0)
						head_bound = before;
					//start
					int nodes_left = 0;
					//1.change head
					nodes_left = head_bound;
					if(nodes_left > 1){
						int guess = h;while((guess=rand()%nodes_left)==h);
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,guess,c,before);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					//2.change child
					nodes_left = len_t - 1 - child_bound;
					if(nodes_left > 1){
						int guess = c;while((guess=rand()%nodes_left+child_bound+1)==c);
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,h,guess,before);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					//3.change center
					nodes_left = c-h-1;
					if(nodes_left > 1){
						int guess = before;while((guess=rand()%nodes_left+h+1)==before);
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,h,c,guess);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					//4.possibly no center
					if(before>0){
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,h,c,-1);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					before = c;
				}
			}
		}
		}
	}

	current = 0;
	end = real_num_pairs;
	//shuffle --- make sure shuffle 2 at the same time(here really lazy to write another shuffle,so ...)
	shuffle_data(data,data,2*mach->GetIdim(),2*mach->GetIdim(),
			real_num_pairs*mach->GetIdim(),real_num_pairs*mach->GetIdim(),10);
	//sample
	cout << "--Data for this iter: samples all " << end << " resample: " << (int)(end*CONF_NN_resample) << endl;
	if(CONF_NN_resample<1)
		end = (int)(end*CONF_NN_resample);
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

/*		************** THE OLD ONE
void Method6_O2sib::each_prepare_data_oneiter()
{
	delete []data;
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
		num_pairs += 2*length*3;	//change the 3 items
	}
	int real_num_pairs = 0;
	data = new REAL[num_pairs*mach->GetIdim()];
	REAL* assign_x = data;
	for(int i=0;i<sentences;i++){
		DependencyInstance* x = training_corpus->at(i);
		int len_t = x->length();
		//add each head
		for(int h=0;h<len_t;h++){
			int before = -1;
			for(int c=h-1;c>=0;c--){
				if(x->heads->at(c) == h){
					int head_bound = GET_MAX_ONE(c,before);
					int child_bound = h;
					if(before>0)
						child_bound = before;
					//start
					int nodes_left = 0;
					//1.change head
					nodes_left = len_t - 1 - head_bound;
					if(nodes_left > 1){
						int guess = h;while((guess=rand()%nodes_left+head_bound+1)==h);
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,guess,c,before);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					//2.change child
					nodes_left = child_bound;
					if(nodes_left > 1){
						int guess = c;while((guess=rand()%nodes_left)==c);
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,h,guess,before);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					//3.change center
					nodes_left = h-c-1;
					if(nodes_left > 1){
						int guess = before;while((guess=rand()%nodes_left+c+1)==before);
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,h,c,guess);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					before = c;
				}
			}
			before = -1;
			for(int c=h+1;c<len_t;c++){
				if(x->heads->at(c) == h){
					int child_bound = GET_MAX_ONE(h,before);
					int head_bound = c;
					if(before>0)
						head_bound = before;
					//start
					int nodes_left = 0;
					//1.change head
					nodes_left = head_bound;
					if(nodes_left > 1){
						int guess = h;while((guess=rand()%nodes_left)==h);
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,guess,c,before);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					//2.change child
					nodes_left = len_t - 1 - child_bound;
					if(nodes_left > 1){
						int guess = c;while((guess=rand()%nodes_left+child_bound+1)==c);
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,h,guess,before);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					//3.change center
					nodes_left = c-h-1;
					if(nodes_left > 1){
						int guess = before;while((guess=rand()%nodes_left+h+1)==before);
						feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
						feat_gen->fill_one(assign_x,x,h,c,guess);assign_x += mach->GetIdim();
						real_num_pairs += 2;
					}
					before = c;
				}
			}
		}
	}

	current = 0;
	end = real_num_pairs;
	//shuffle --- make sure shuffle 2 at the same time(here really lazy to write another shuffle,so ...)
	shuffle_data(data,data,2*mach->GetIdim(),2*mach->GetIdim(),
			real_num_pairs*mach->GetIdim(),real_num_pairs*mach->GetIdim(),10);
	//sample
	cout << "--Data for this iter: samples all " << end << " resample: " << (int)(end*CONF_NN_resample) << endl;
	if(CONF_NN_resample<1)
		end = (int)(end*CONF_NN_resample);
}
*/
