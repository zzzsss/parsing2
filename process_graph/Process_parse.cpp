/*
 * Process_parse.cpp
 *
 *  Created on: 2015Äê3ÔÂ18ÈÕ
 *      Author: zzs
 */

#include "Process.h"
#include "../algorithms/Eisner.h"
#include "../algorithms/EisnerO2sib.h"

//-------------------- these two also static methods -----------------------------
//return Score[length][length]
double* Process::get_scores_o1(DependencyInstance* x,parsing_conf* zp,Mach* zm,FeatureGen* zf)
{
	int idim = zf->get_xdim();
	int odim = zm->GetOdim();
	//default order1 parsing
	int length = x->forms->size();
	double *tmp_scores = new double[length*length];
	//construct scores using nn
	int num_pair = length*(length-1);	//2 * (0+(l-1))*l/2
	REAL *mach_x = new REAL[num_pair*idim];
	REAL* assign_x = mach_x;
	for(int ii=0;ii<length;ii++){
		for(int j=0;j<length;j++){
			//ii -> j
			if(ii != j){
				zf->fill_one(assign_x,x,ii,j);
				assign_x += idim;
			}
		}
	}
	REAL* mach_y = mach_forward(zm,mach_x,num_pair);
	REAL* assign_y = mach_y;
	for(int ii=0;ii<length;ii++){
		for(int j=0;j<length;j++){
			if(ii!=j){
				int index = get_index2(length,ii,j);
				//if filter --- this is the easy way(but waste computation)
				if(zp->CONF_pos_filter && zf->has_filter()){
					if(!zf->allowed_pair(x,ii,j)){
						tmp_scores[index] = DOUBLE_LARGENEG;	//is this neg enough??
						//skip forward
						for(int c=0;c<odim;c++)
							assign_y++;
						continue;
					}
				}
				if(odim > 1){
					if(zp->CONF_NN_scoremax){
						double temp = *assign_y;
						double ans = 0;
						for(int c=0;c<odim;c++)
							if(*assign_y++ > temp)
								ans = c;
						tmp_scores[index] = ans;
					}
					else{
						double temp = 0;
						for(int c=0;c<odim;c++)
							temp += (*assign_y++)*c;
						tmp_scores[index] = temp;
					}
				}
				else
					tmp_scores[index] = *assign_y++;
			}
		}
	}
	delete []mach_x;
	delete []mach_y;
	return tmp_scores;
}

//return Score[length][length][length]
double* Process::get_scores_o2sib(DependencyInstance* x,parsing_conf* zp,Mach* zm,FeatureGen* zf)
{
	int idim = zf->get_xdim();
	int odim = zm->GetOdim();
	// one sentence
	int length = x->forms->size();
	int num_allocated = length*length*length;
	int num_togo = 0;
	double *tmp_scores = new double[length*length*length];
	for(int i=0;i<length*length*length;i++)
		tmp_scores[i] = DOUBLE_LARGENEG;
	REAL *mach_x = new REAL[num_allocated*idim];	//num_allocated is more than needed
	REAL* assign_x = mach_x;
	for(int s=0;s<length;s++){
		for(int t=s+1;t<length;t++){
			//s<->t
			if(!zp->CONF_pos_filter || zf->allowed_pair(x,s,t,-1)){
				zf->fill_one(assign_x,x,s,t,-1);
				assign_x += idim;
				num_togo += 1;
			}
			if(!zp->CONF_pos_filter || zf->allowed_pair(x,t,s,-1)){
				zf->fill_one(assign_x,x,t,s,-1);
				assign_x += idim;
				num_togo += 1;
			}
		}
	}
	for(int s=0;s<length;s++){
		for(int c=s+1;c<length;c++){
			for(int t=c+1;t<length;t++){
				//s c t
				if(!zp->CONF_pos_filter || zf->allowed_pair(x,s,t,c)){
					zf->fill_one(assign_x,x,s,t,c);
					assign_x += idim;
					num_togo += 1;
				}
				if(!zp->CONF_pos_filter || zf->allowed_pair(x,t,s,c)){
					zf->fill_one(assign_x,x,t,s,c);
					assign_x += idim;
					num_togo += 1;
				}
			}
		}
	}
	//forward
	REAL* mach_y = mach_forward(zm,mach_x,num_togo);
	//and assign the scores
#define TMP_GET_MAXINDEX(o_dim,a,assign) {a=0;double temp=*assign;for(int c=0;c<o_dim;c++) if((*assign++)>temp) a=c;}
#define TMP_GET_ONE(o_dim,a,assign) {\
	if(o_dim>1){\
		if(zp->CONF_NN_scoremax){TMP_GET_MAXINDEX(o_dim,a,assign)}\
		else{a=0;for(int c=0;c<o_dim;c++) a+=(*assign++)*c;}\
	}\
	else{a=*assign++;}}

	REAL* assign_y = mach_y;
	double answer = 0;
	for(int s=0;s<length;s++){
		for(int t=s+1;t<length;t++){
			//s<->t
			if(!zp->CONF_pos_filter || zf->allowed_pair(x,s,t,-1)){
				TMP_GET_ONE(odim,answer,assign_y)
				tmp_scores[get_index2_o2sib(length,s,s,t)] = answer;
			}
			if(!zp->CONF_pos_filter || zf->allowed_pair(x,t,s,-1)){
				TMP_GET_ONE(odim,answer,assign_y)
				tmp_scores[get_index2_o2sib(length,t,t,s)] = answer;
			}
		}
	}
	for(int s=0;s<length;s++){
		for(int c=s+1;c<length;c++){
			for(int t=c+1;t<length;t++){
				//s c t
				if(!zp->CONF_pos_filter || zf->allowed_pair(x,s,t,c)){
					TMP_GET_ONE(odim,answer,assign_y)
					tmp_scores[get_index2_o2sib(length,s,c,t)] = answer;
				}
				if(!zp->CONF_pos_filter || zf->allowed_pair(x,t,s,c)){
					TMP_GET_ONE(odim,answer,assign_y)
					tmp_scores[get_index2_o2sib(length,t,c,s)] = answer;
				}
			}
		}
	}
	delete []mach_x;
	delete []mach_y;
	return tmp_scores;
}


//-------------------- parsing non-static methods -----------------------------
vector<int>* Process::parse_o1(DependencyInstance* x)
{
	double *tmp_scores = get_scores_o1(x,parameters,mach,feat_gen);
	vector<int> *ret = decodeProjective(x->length(),tmp_scores);
	delete []tmp_scores;
	return ret;
}

vector<int>* Process::parse_o2sib(DependencyInstance* x,double* score_of_o1)
{
	int length = x->length();
	double *tmp_scores = get_scores_o2sib(x,parameters,mach,feat_gen);
	if(score_of_o1){
		for(int i=0;i<length;i++){
			for(int j=0;j<length;j++){
				if(i!=j){
					double score_tmp = score_of_o1[get_index2(length,i,j)];
					tmp_scores[get_index2_o2sib(length,i,i,j)] += score_tmp;
					//really lazy here: (i,j)
					for(int c=i+1;c<j;c++)
						tmp_scores[get_index2_o2sib(length,i,c,j)] += score_tmp;
					for(int c=j+1;c<i;c++)
						tmp_scores[get_index2_o2sib(length,i,c,j)] += score_tmp;
				}
			}
		}
	}
	vector<int> *ret = decodeProjective_o2sib(length,tmp_scores);
	delete []tmp_scores;
	return ret;
}



