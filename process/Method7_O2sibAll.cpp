/*
 * Method7_O2sibAll.cpp
 *
 *  Created on: 2015Äê3ÔÂ15ÈÕ
 *      Author: zzs
 */

#include "Method7_O2sibAll.h"
#include <cstdio>

void Method7_O2sibAll::each_write_mach_conf()
{
	write_conf(2);
}

void Method7_O2sibAll::each_prepare_data_oneiter()
{
	delete []data;
	delete []target;
	delete []gradient;
	//for gradient
	gradient = new REAL[mach->GetBsize()*mach->GetOdim()];
	mach->SetGradOut(gradient);
	FeatureGenO2sib* feat_o2 = (FeatureGenO2sib*)feat_gen;	//force it

	//sweep all once and count
	int tmp2_right=0,tmp2_wrong=0,tmp2_bad=0;
	int tmp3_right=0,tmp3_wrong=0,tmp3_bad=0;
	int sentences = training_corpus->size();
	for(int i=0;i<sentences;i++){
		DependencyInstance* x = training_corpus->at(i);
		int length = x->length();
		for(int h=0;h<length;h++){
			//left
			int first_m = -1;
			for(int m=h-1;m>=0;m--){
				// h - m
				if(x->heads->at(m)==h && first_m<0){
					tmp2_right++;
					first_m = m;
				}
				else if(CONF_pos_filter && !feat_o2->allowed_pair(x,h,m,-1))
					tmp2_bad++;
				else
					tmp2_wrong++;
				//h m out-one
				int first_out = -1;
				for(int one=m-1;one>=0;one--){
					if(x->heads->at(m)==h && x->heads->at(one)==h && first_out<0){
						tmp3_right++;
						first_m = m;
					}
					else if(CONF_pos_filter && !feat_o2->allowed_pair(x,h,one,m))
						tmp3_bad++;
					else
						tmp3_wrong++;
				}
			}
			//right
			first_m = -1;
			for(int m=h+1;m<length;m++){
				// h - m
				if(x->heads->at(m)==h && first_m<0){
					tmp2_right++;
					first_m = m;
				}
				else if(CONF_pos_filter && !feat_o2->allowed_pair(x,h,m,-1))
					tmp2_bad++;
				else
					tmp2_wrong++;
				//h m out-one
				int first_out = -1;
				for(int one=m+1;one<length;one++){
					if(x->heads->at(m)==h && x->heads->at(one)==h && first_out<0){
						tmp3_right++;
						first_m = m;
					}
					else if(CONF_pos_filter && !feat_o2->allowed_pair(x,h,one,m))
						tmp3_bad++;
					else
						tmp3_wrong++;
				}
			}
		}
	}
	int tmpall_right=tmp2_right+tmp3_right;
	int tmpall_wrong=tmp2_wrong+tmp3_wrong;
	int tmpall_bad=tmp2_bad+tmp3_bad;
	printf("--Stat<all,2,3>:right(%d,%d,%d),wrong(%d,%d,%d),bad(%d,%d,%d)\n",tmpall_right,tmp2_right,tmp3_right,
			tmpall_wrong,tmp2_wrong,tmp3_wrong,tmpall_bad,tmp2_bad,tmp3_bad);

	//sweep second time and adding them
	//-allocate
	int idim = mach->GetIdim();
	int odim = mach->GetOdim();
	REAL* data_right = new REAL[tmpall_right*idim];
	REAL* data_wrong = new REAL[tmpall_wrong*idim];
	REAL* assign_right = data_right;
	REAL* assign_wrong = data_wrong;
	for(int i=0;i<sentences;i++){
		DependencyInstance* x = training_corpus->at(i);
		int length = x->length();
		for(int h=0;h<length;h++){
			//left
			int first_m = -1;
			for(int m=h-1;m>=0;m--){
				// h - m
				if(x->heads->at(m)==h && first_m<0){
					feat_gen->fill_one(assign_right,x,h,m,-1);assign_right += idim;
					first_m = m;
				}
				else if(CONF_pos_filter && !feat_o2->allowed_pair(x,h,m,-1)){}
				else{
					feat_gen->fill_one(assign_wrong,x,h,m,-1);assign_wrong += idim;
				}
				//h m out-one
				int first_out = -1;
				for(int one=m-1;one>=0;one--){
					if(x->heads->at(m)==h && x->heads->at(one)==h && first_out<0){
						feat_gen->fill_one(assign_right,x,h,one,m);assign_right += idim;
						first_m = m;
					}
					else if(CONF_pos_filter && !feat_o2->allowed_pair(x,h,one,m)){}
					else{
						feat_gen->fill_one(assign_wrong,x,h,one,m);assign_wrong += idim;
					}
				}
			}
			//right
			first_m = -1;
			for(int m=h+1;m<length;m++){
				// h - m
				if(x->heads->at(m)==h && first_m<0){
					feat_gen->fill_one(assign_right,x,h,m,-1);assign_right += idim;
					first_m = m;
				}
				else if(CONF_pos_filter && !feat_o2->allowed_pair(x,h,m,-1)){}
				else{
					feat_gen->fill_one(assign_wrong,x,h,m,-1);assign_wrong += idim;
				}
				//h m out-one
				int first_out = -1;
				for(int one=m+1;one<length;one++){
					if(x->heads->at(m)==h && x->heads->at(one)==h && first_out<0){
						feat_gen->fill_one(assign_right,x,h,one,m);assign_right += idim;
						first_m = m;
					}
					else if(CONF_pos_filter && !feat_o2->allowed_pair(x,h,one,m)){}
					else{
						feat_gen->fill_one(assign_wrong,x,h,one,m);assign_wrong += idim;
					}
				}
			}
		}
	}

	//then considering CONF_NN_resample and copy them to finish data
	if(CONF_NN_resample < 1){
		//get part of the wrong ones --- but first shuffle them
		shuffle_data(data_wrong,data_wrong,idim,idim,tmpall_wrong*idim,tmpall_wrong*idim,10);
	}
	int tmp_sumup = tmpall_wrong*CONF_NN_resample + tmpall_right;
	data = new REAL[tmp_sumup*idim];
	target = new REAL[tmp_sumup];
	memcpy(data,data_right,tmpall_right*idim*sizeof(REAL));
	memcpy(data+tmpall_right*idim,data_wrong,tmpall_wrong*CONF_NN_resample*idim*sizeof(REAL));
	for(int i=0;i<tmp_sumup;i++){
		if(i<tmpall_right)
			target[i] = 1;
		else
			target[i] = 0;
	}
	delete []data_right;
	delete []data_wrong;
	shuffle_data(data,target,idim,1,tmp_sumup*idim,tmp_sumup,10);	//final shuffle
	cout << "--Data for this iter: samples all " << tmpall_right+tmpall_wrong << " resample: " << tmp_sumup << endl;
	current = 0;
	end = tmp_sumup;
}

void Method7_O2sibAll::each_get_grad(int size)
{
	set_softmax_gradient(target+current,mach->GetDataOut(),gradient,size,mach->GetOdim());
	//!! here add it
	current += size;
}
