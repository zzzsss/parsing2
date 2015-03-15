/*
 * Method7_O2sibAll.cpp
 *
 *  Created on: 2015Äê3ÔÂ15ÈÕ
 *      Author: zzs
 */

#include "Method7_O2sibAll.h"
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

}

void Method7_O2sibAll::each_get_grad(int size)
{
	set_softmax_gradient(target+current,mach->GetDataOut(),gradient,size,mach->GetOdim());
	//!! here add it
	current += size;
}
