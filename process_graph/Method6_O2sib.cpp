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
	if(((int)parameters->CONF_NN_resample)>=1){
		num_pairs *= (int)parameters->CONF_NN_resample;	//multiple
	}
	int real_num_pairs = 0;
	data = new REAL[num_pairs*mach->GetIdim()];
	REAL* assign_x = data;

	FeatureGenO2sib* feat_o2 = (FeatureGenO2sib*)feat_gen;	//force it
	for(int i=0;i<sentences;i++){
		DependencyInstance* x = training_corpus->at(i);
		int len_t = x->length();
		//add each head
		// really simple implement for multiple ones
#define FILTER_MAX_TRY_TIMES 10
		for(int times=0;times<(int)parameters->CONF_NN_resample;times++){
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
						int guess = h;
						int trying = 0;
						if(!parameters->CONF_pos_filter)
							while((guess=rand()%nodes_left+head_bound+1)==h);
						else
							while(trying++<FILTER_MAX_TRY_TIMES && (guess=rand()%nodes_left+head_bound+1)==h
									&& !feat_o2->allowed_pair(x,guess,c,before));
						if(trying < FILTER_MAX_TRY_TIMES){
							feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
							feat_gen->fill_one(assign_x,x,guess,c,before);assign_x += mach->GetIdim();
							real_num_pairs += 2;
						}
					}
					//2.change child
					nodes_left = child_bound;
					if(nodes_left > 1){
						int guess = c;
						int trying = 0;
						if(!parameters->CONF_pos_filter)
							while((guess=rand()%nodes_left)==c);
						else
							while(trying++<FILTER_MAX_TRY_TIMES && (guess=rand()%nodes_left)==c
									&& !feat_o2->allowed_pair(x,h,guess,before));
						if(trying < FILTER_MAX_TRY_TIMES){
							feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
							feat_gen->fill_one(assign_x,x,h,guess,before);assign_x += mach->GetIdim();
							real_num_pairs += 2;
						}
					}
					//3.change center
					nodes_left = h-c-1;
					if(nodes_left > 1){
						int guess = before;
						int trying = 0;
						if(!parameters->CONF_pos_filter)
							while((guess=rand()%nodes_left+c+1)==before);
						else
							while(trying++<FILTER_MAX_TRY_TIMES && (guess=rand()%nodes_left+c+1)==before
									&& !feat_o2->allowed_pair(x,h,c,guess));
						if(trying < FILTER_MAX_TRY_TIMES){
							feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
							feat_gen->fill_one(assign_x,x,h,c,guess);assign_x += mach->GetIdim();
							real_num_pairs += 2;
						}
					}
					//4.possibly no center
					if(before>0){
						if(!parameters->CONF_pos_filter || feat_o2->allowed_pair(x,h,c,-1)){
							feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
							feat_gen->fill_one(assign_x,x,h,c,-1);assign_x += mach->GetIdim();
							real_num_pairs += 2;
						}
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
						int guess = h;
						int trying = 0;
						if(!parameters->CONF_pos_filter)
							while((guess=rand()%nodes_left)==h);
						else
							while(trying++<FILTER_MAX_TRY_TIMES && (guess=rand()%nodes_left)==h
									&& !feat_o2->allowed_pair(x,guess,c,before));
						if(trying < FILTER_MAX_TRY_TIMES){
							feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
							feat_gen->fill_one(assign_x,x,guess,c,before);assign_x += mach->GetIdim();
							real_num_pairs += 2;
						}
					}
					//2.change child
					nodes_left = len_t - 1 - child_bound;
					if(nodes_left > 1){
						int guess = c;
						int trying = 0;
						if(!parameters->CONF_pos_filter)
							while((guess=rand()%nodes_left+child_bound+1)==c);
						else
							while(trying++<FILTER_MAX_TRY_TIMES && (guess=rand()%nodes_left+child_bound+1)==c
									&& !feat_o2->allowed_pair(x,h,guess,before));
						if(trying < FILTER_MAX_TRY_TIMES){
							feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
							feat_gen->fill_one(assign_x,x,h,guess,before);assign_x += mach->GetIdim();
							real_num_pairs += 2;
						}
					}
					//3.change center
					nodes_left = c-h-1;
					if(nodes_left > 1){
						int guess = before;
						int trying = 0;
						if(!parameters->CONF_pos_filter)
							while((guess=rand()%nodes_left+h+1)==before);
						else
							while(trying++<FILTER_MAX_TRY_TIMES && (guess=rand()%nodes_left+h+1)==before
									&& !feat_o2->allowed_pair(x,h,c,guess));
						if(trying < FILTER_MAX_TRY_TIMES){
							feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
							feat_gen->fill_one(assign_x,x,h,c,guess);assign_x += mach->GetIdim();
							real_num_pairs += 2;
						}
					}
					//4.possibly no center
					if(before>0){
						if(!parameters->CONF_pos_filter || feat_o2->allowed_pair(x,h,c,-1)){
							feat_gen->fill_one(assign_x,x,h,c,before);assign_x += mach->GetIdim();
							feat_gen->fill_one(assign_x,x,h,c,-1);assign_x += mach->GetIdim();
							real_num_pairs += 2;
						}
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
	cout << "--Data for this iter: samples all " << end << " resample: " << (int)(end*parameters->CONF_NN_resample) << endl;
	if(parameters->CONF_NN_resample<1)
		end = (int)(end*parameters->CONF_NN_resample);
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
	if(parameters->CONF_NN_O2sib_o1mach.length() > 0 && parameters->CONF_NN_O2sib_score_combine){
		FeatureGenO1* feat_temp_o1 = new FeatureGenO1(dict,parameters->CONF_x_window,
				parameters->CONF_add_distance,parameters->CONF_add_pos,parameters->CONF_add_distance_parent);
		double* scores_o1 = get_scores_o1(x,parameters,mach_o1,feat_temp_o1);	//same parameters
		ret = parse_o2sib(x,scores_o1);
		delete []scores_o1;
	}
	else{
		ret = parse_o2sib(x);
	}
	return ret;
}

//maybe init embedding from o1 machine
void Method6_O2sib::init_embed()
{
	if(parameters->CONF_NN_O2sib_o1mach.length() > 0 && parameters->CONF_NN_O2sib_embed_init){
		//special structure
		int all = parameters->CONF_NN_we * dict->get_count();
		REAL* embed_from;
		REAL* embed_to;
		{
			MachMulti* m = (MachMulti*)mach_o1;
			m = (MachMulti*)(m->MachGet(0));
			MachTab* mm = (MachTab*)(m->MachGet(0));
			embed_from = mm->GetTabAdr();
		}
		{
			MachMulti* m = (MachMulti*)mach;
			m = (MachMulti*)(m->MachGet(0));
			MachTab* mm = (MachTab*)(m->MachGet(0));
			embed_to = mm->GetTabAdr();
		}
		memcpy(embed_to,embed_from,sizeof(REAL)*all);
	}
	else
		Process::init_embed();
}
