/*
 * Method7_O2sibAll.cpp
 *
 *  Created on: 2015Äê3ÔÂ15ÈÕ
 *      Author: zzs
 */

#include "Method7_O2sibAll.h"
#include "../algorithms/Eisner.h"
#include <cstdio>

void Method7_O2sibAll::each_prepare_data_oneiter()
{
	delete []data;
	delete []target;
	delete []gradient;
	//for gradient
	gradient = new REAL[mach->GetBsize()*mach->GetOdim()];
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
	if(parameters->CONF_NN_O2sib_o1mach.length() > 0 && parameters->CONF_NN_o2sib_o1filter)
		whether_o1_filter = 1;

	if(data_right==0 || !whether_o1_filter){
	//sweep all once and count
	FeatureGenO1* feat_temp_o1 = new FeatureGenO1(dict,parameters->CONF_x_window,
					parameters->CONF_add_distance,parameters->CONF_add_pos,parameters->CONF_add_distance_parent);
	double** all_scores_o1 = new double*[sentences];
	for(int i=0;i<sentences;i++){
		all_scores_o1[i] = 0;
		if(whether_o1_filter)
			all_scores_o1[i] = get_scores_o1(training_corpus->at(i),parameters,mach_o1,feat_temp_o1);
	}
	int tmp2_right=0,tmp2_wrong=0,tmp2_bad=0;
	int tmp3_right=0,tmp3_wrong=0,tmp3_bad=0;
	for(int i=0;i<sentences;i++){
		DependencyInstance* x = training_corpus->at(i);
		double* scores_o1_filter = all_scores_o1[i];
		int length = x->length();
		for(int h=0;h<length;h++){
			//left
			int first_m = -1;
			for(int m=h-1;m>=0;m--){
				double score_hm = whether_o1_filter ? scores_o1_filter[get_index2(length,h,m)] : 0;
				// h - m
				if(x->heads->at(m)==h && first_m<0){
					tmp2_right++;
					first_m = m;
				}
				else if(parameters->CONF_pos_filter && !feat_o2->allowed_pair(x,h,m,-1))
					tmp2_bad++;
				else if(whether_o1_filter &&  score_noprob(score_hm))
					tmp2_bad++;
				else
					tmp2_wrong++;
				//h m out-one
				int first_out = -1;
				for(int one=m-1;one>=0;one--){
					if(x->heads->at(m)==h && x->heads->at(one)==h && first_out<0){
						tmp3_right++;
						first_out = one;
					}
					else if(whether_o1_filter && (score_noprob(score_hm)||score_noprob(scores_o1_filter[get_index2(length,h,one)])))
						tmp2_bad++;
					else if(parameters->CONF_pos_filter && !feat_o2->allowed_pair(x,h,one,m))
						tmp3_bad++;
					else
						tmp3_wrong++;
				}
			}
			//right
			first_m = -1;
			for(int m=h+1;m<length;m++){
				double score_hm = whether_o1_filter ? scores_o1_filter[get_index2(length,h,m)] : 0;
				// h - m
				if(x->heads->at(m)==h && first_m<0){
					tmp2_right++;
					first_m = m;
				}
				else if(parameters->CONF_pos_filter && !feat_o2->allowed_pair(x,h,m,-1))
					tmp2_bad++;
				else if(whether_o1_filter &&  score_noprob(score_hm))
					tmp2_bad++;
				else
					tmp2_wrong++;
				//h m out-one
				int first_out = -1;
				for(int one=m+1;one<length;one++){
					if(x->heads->at(m)==h && x->heads->at(one)==h && first_out<0){
						tmp3_right++;
						first_out = one;
					}
					else if(parameters->CONF_pos_filter && !feat_o2->allowed_pair(x,h,one,m))
						tmp3_bad++;
					else if(whether_o1_filter && (score_noprob(score_hm)||score_noprob(scores_o1_filter[get_index2(length,h,one)])))
						tmp2_bad++;
					else
						tmp3_wrong++;
				}
			}
		}
	}
	tmpall_right=tmp2_right+tmp3_right;
	tmpall_wrong=tmp2_wrong+tmp3_wrong;
	tmpall_bad=tmp2_bad+tmp3_bad;
	printf("--Stat<all,2,3>:right(%d,%d,%d),wrong(%d,%d,%d),bad(%d,%d,%d)\n",tmpall_right,tmp2_right,tmp3_right,
			tmpall_wrong,tmp2_wrong,tmp3_wrong,tmpall_bad,tmp2_bad,tmp3_bad);

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
		for(int h=0;h<length;h++){
			//left
			int first_m = -1;
			for(int m=h-1;m>=0;m--){
				double score_hm = whether_o1_filter ? scores_o1_filter[get_index2(length,h,m)] : 0;
				// h - m
				if(x->heads->at(m)==h && first_m<0){
					feat_gen->fill_one(assign_right,x,h,m,-1);assign_right += idim;
					first_m = m;
				}
				else if(parameters->CONF_pos_filter && !feat_o2->allowed_pair(x,h,m,-1)){}
				else if(whether_o1_filter &&  score_noprob(score_hm)){}
				else{
					feat_gen->fill_one(assign_wrong,x,h,m,-1);assign_wrong += idim;
				}
				//h m out-one
				int first_out = -1;
				for(int one=m-1;one>=0;one--){
					if(x->heads->at(m)==h && x->heads->at(one)==h && first_out<0){
						feat_gen->fill_one(assign_right,x,h,one,m);assign_right += idim;
						first_out = one;
					}
					else if(parameters->CONF_pos_filter && !feat_o2->allowed_pair(x,h,one,m)){}
					else if(whether_o1_filter && (score_noprob(score_hm)||score_noprob(scores_o1_filter[get_index2(length,h,one)]))){}
					else{
						feat_gen->fill_one(assign_wrong,x,h,one,m);assign_wrong += idim;
					}
				}
			}
			//right
			first_m = -1;
			for(int m=h+1;m<length;m++){
				double score_hm = whether_o1_filter ? scores_o1_filter[get_index2(length,h,m)] : 0;
				// h - m
				if(x->heads->at(m)==h && first_m<0){
					feat_gen->fill_one(assign_right,x,h,m,-1);assign_right += idim;
					first_m = m;
				}
				else if(parameters->CONF_pos_filter && !feat_o2->allowed_pair(x,h,m,-1)){}
				else if(whether_o1_filter &&  score_noprob(score_hm)){}
				else{
					feat_gen->fill_one(assign_wrong,x,h,m,-1);assign_wrong += idim;
				}
				//h m out-one
				int first_out = -1;
				for(int one=m+1;one<length;one++){
					if(x->heads->at(m)==h && x->heads->at(one)==h && first_out<0){
						feat_gen->fill_one(assign_right,x,h,one,m);assign_right += idim;
						first_out = one;
					}
					else if(parameters->CONF_pos_filter && !feat_o2->allowed_pair(x,h,one,m)){}
					else if(whether_o1_filter && (score_noprob(score_hm)||score_noprob(scores_o1_filter[get_index2(length,h,one)]))){}
					else{
						feat_gen->fill_one(assign_wrong,x,h,one,m);assign_wrong += idim;
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
	cout << "--Data for this iter: samples all " << tmpall_right+tmpall_wrong << " resample: " << tmp_sumup << endl;
	current = 0;
	end = tmp_sumup;
	if(!whether_o1_filter){
		delete[] data_right;
		delete[] data_wrong;
	}
}

void Method7_O2sibAll::each_get_grad(int size)
{
	set_softmax_gradient(target+current,mach->GetDataOut(),gradient,size,mach->GetOdim());
	//!! here add it
	current += size;
}
