/*
 * Method8_O2g.cpp
 *
 *  Created on: 2015��4��21��
 *      Author: zzs
 */

#include "Method8_O2g.h"
#include "../algorithms/Eisner.h"
#include <cstdio>
#include <cstdlib>

void Method8_O2g::each_prepare_data_oneiter()
{
	delete []data;
	delete []target;
	delete []gradient;
	//for gradient
	gradient = new REAL[mach->GetWidth()*mach->GetOdim()];
	mach->SetGradOut(gradient);
	FeatureGenO2sib* feat_o2 = (FeatureGenO2sib*)feat_gen;	//force it
	int sentences = training_corpus->size();
	int idim = mach->GetIdim();
	int odim = mach->GetOdim();

	//only one time when o1_filter(decoding o1 is quite expensive)
	static REAL* data_right = 0;
	static REAL* data_wrong = 0;
	static int tmpall_right=0;
	static int tmpall_wrong=0;
	static int tmpall_bad=0;
	int whether_o1_filter = 0;
	if(parameters->CONF_NN_highO_o1mach.length() > 0 && parameters->CONF_NN_highO_o1filter)
		whether_o1_filter = 1;

	//************WE MUST SPECIFY O1_FILTER****************//
	if(!whether_o1_filter){
		cout << "No o1-filter for o2g, too expensive!!" << endl;
		exit(1);
	}
	//************WE MUST SPECIFY O1_FILTER****************//

	if(data_right==0 || !whether_o1_filter){
	//sweep all once and count
	FeatureGenO1* feat_temp_o1 = new FeatureGenO1(dict,parameters->CONF_x_window,
					parameters->CONF_add_distance,parameters->CONF_add_pos,parameters->CONF_add_distance_parent);
	double** all_scores_o1 = new double*[sentences];
	int all_tokens_train=0,all_token_filter_wrong=0;
	for(int i=0;i<sentences;i++){
		all_scores_o1[i] = 0;
		if(whether_o1_filter){
			DependencyInstance* x = training_corpus->at(i);
			all_scores_o1[i] = get_scores_o1(x,parameters,mach_o1,feat_temp_o1);
			double* scores_o1_filter = all_scores_o1[i];
			all_tokens_train += x->length();
			for(int i2=1;i2<x->length();i2++){	//ignore root
				if(score_noprob(scores_o1_filter[get_index2(x->length(),x->heads->at(i2),i2)]))
					all_token_filter_wrong ++;
			}
		}
	}
	if(whether_o1_filter)
		cout << "For o1 filter: all " << all_tokens_train << ";filter wrong " << all_token_filter_wrong << endl;

	for(int i=0;i<sentences;i++){
		DependencyInstance* x = training_corpus->at(i);
		double* scores_o1_filter = all_scores_o1[i];
		int length = x->length();
		//first special (0,0,m)
		for(int m=0;m<length;m++){
			if(x->heads->at(m) == 0)
				tmpall_right++;
			else if(score_noprob(scores_o1_filter[get_index2(length,0,m)]))
				tmpall_bad++;
			else
				tmpall_wrong++;
		}
		//then (g,h,m)
		for(int s=0;s<length;s++){
			for(int t=s+1;t<length;t++){
				int nope_st = score_noprob(scores_o1_filter[get_index2(length,s,t)]);
				int nope_ts = score_noprob(scores_o1_filter[get_index2(length,t,s)]);
				for(int g=0;g<length;g++){
					if(g>=s && g<=t)
						continue;
					int nope_gs = score_noprob(scores_o1_filter[get_index2(length,g,s)]);
					int nope_gt = score_noprob(scores_o1_filter[get_index2(length,g,t)]);
					//s->t
					if(x->heads->at(t)==s && x->heads->at(s)==g)
						tmpall_right++;
					else if(nope_st || nope_gs)
						tmpall_bad++;
					else
						tmpall_wrong++;
					//t->s
					if(x->heads->at(s)==t && x->heads->at(t)==g)
						tmpall_right++;
					else if(nope_ts || nope_gt)
						tmpall_bad++;
					else
						tmpall_wrong++;
				}
			}
		}
	}
	printf("--Stat:%d,%d,%d\n",tmpall_right,tmpall_wrong,tmpall_bad);

	//sweep second time and adding them
	//-allocate
	data_right = new REAL[tmpall_right*idim];
	data_wrong = new REAL[tmpall_wrong*idim];
	REAL* assign_right = data_right;
	REAL* assign_wrong = data_wrong;
	for(int i=0;i<sentences;i++){
		DependencyInstance* x = training_corpus->at(i);
		int length = x->length();
		double* scores_o1_filter = all_scores_o1[i];
		//first special (0,0,m)
		for(int m=0;m<length;m++){
			if(x->heads->at(m) == 0){
				feat_gen->fill_one(assign_right,x,0,m,0);assign_right += idim;
			}
			else if(score_noprob(scores_o1_filter[get_index2(length,0,m)])){}
			else{
				feat_gen->fill_one(assign_wrong,x,0,m,0);assign_wrong += idim;
			}
		}
		//then (g,h,m)
		for(int s=0;s<length;s++){
			for(int t=s+1;t<length;t++){
				int nope_st = score_noprob(scores_o1_filter[get_index2(length,s,t)]);
				int nope_ts = score_noprob(scores_o1_filter[get_index2(length,t,s)]);
				for(int g=0;g<length;g++){
					if(g>=s && g<=t)
						continue;
					int nope_gs = score_noprob(scores_o1_filter[get_index2(length,g,s)]);
					int nope_gt = score_noprob(scores_o1_filter[get_index2(length,g,t)]);
					//s->t
					if(x->heads->at(t)==s && x->heads->at(s)==g){
						feat_gen->fill_one(assign_right,x,s,t,g);assign_right += idim;
					}
					else if(nope_st || nope_gs){}
					else{
						feat_gen->fill_one(assign_wrong,x,s,t,g);assign_wrong += idim;
					}
					//t->s
					if(x->heads->at(s)==t && x->heads->at(t)==g){
						feat_gen->fill_one(assign_right,x,t,s,g);assign_right += idim;
					}
					else if(nope_ts || nope_gt){}
					else{
						feat_gen->fill_one(assign_wrong,x,t,s,g);assign_wrong += idim;
					}
				}
			}
		}
	}

	for(int i=0;i<sentences;i++){
		delete [](all_scores_o1[i]);
	}
	delete []all_scores_o1;
	}

	//then considering CONF_NN_resample and copy them to finish data
	if(parameters->CONF_NN_resample < 1){
		//get part of the wrong ones --- but first shuffle them
		shuffle_data(data_wrong,data_wrong,idim,idim,tmpall_wrong*idim,tmpall_wrong*idim,10);
	}
	int tmp_sumup = tmpall_wrong*parameters->CONF_NN_resample + tmpall_right;
	data = new REAL[tmp_sumup*idim];
	target = new REAL[tmp_sumup];
	memcpy(data,data_right,tmpall_right*idim*sizeof(REAL));
	memcpy(data+tmpall_right*idim,data_wrong,tmpall_wrong*parameters->CONF_NN_resample*idim*sizeof(REAL));
	for(int i=0;i<tmp_sumup;i++){
		if(i<tmpall_right)
			target[i] = 1;
		else
			target[i] = 0;
	}
	shuffle_data(data,target,idim,1,tmp_sumup*idim,tmp_sumup,10);	//final shuffle
	cout << "--Data for this iter(M8:o2g): samples all " << tmpall_right+tmpall_wrong << " resample: " << tmp_sumup << endl;
	current = 0;
	end = tmp_sumup;
	if(!whether_o1_filter){
		delete[] data_right;
		delete[] data_wrong;
	}
}

REAL* Method8_O2g::each_next_data(int* size)
{
	//size must be even number, if no universal rays, it should be that...
	if(current >= end)
		return 0;
	if(current + *size > end)
		*size = end - current;
	//!!not adding current here
	return (data+current*mach->GetIdim());
}

void Method8_O2g::each_get_grad(int size)
{
	set_softmax_gradient(target+current,mach->GetDataOut(),gradient,size,mach->GetOdim());
	//!! here add it
	current += size;
}

vector<int>* Method8_O2g::each_test_one(DependencyInstance* x)
{
	vector<int>* ret;
	//combine o1 scores
	if(parameters->CONF_NN_highO_o1mach.length() > 0 &&
			(parameters->CONF_NN_highO_score_combine || parameters->CONF_NN_highO_o1filter)){
		FeatureGenO1* feat_temp_o1 = new FeatureGenO1(dict,parameters->CONF_x_window,
				parameters->CONF_add_distance,parameters->CONF_add_pos,parameters->CONF_add_distance_parent);
		double* scores_o1 = get_scores_o1(x,parameters,mach_o1,feat_temp_o1);	//same parameters
		ret = parse_o2g(x,scores_o1);
		delete []scores_o1;
		delete feat_temp_o1;
	}
	else{
		ret = parse_o2g(x);
	}
	return ret;
}


