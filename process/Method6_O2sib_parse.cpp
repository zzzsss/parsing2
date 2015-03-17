/*
 * Method6_O2sib_parse.cpp
 *
 *  Created on: 2015Äê3ÔÂ10ÈÕ
 *      Author: zzs
 */

#include "Method6_O2sib.h"
extern vector<int>* decodeProjective_o2sib(int length,double* scores);

static double* get_score_from_o1(DependencyInstance* x,FeatureGenO1* feat_temp,Mach* m)
{
	//another specific: tmp_scores[h*length+m]
	int idim = feat_temp->get_xdim();
	int odim = m->GetOdim();
	int length = x->forms->size();
	double *tmp_scores = new double[length*length];
	//construct scores using nn
	int num_pair = length*(length-1);	//2 * (0+(l-1))*l/2
	REAL *mach_x = new REAL[num_pair*idim];
	REAL *mach_y = new REAL[num_pair*odim];
	REAL* assign_x = mach_x;
	for(int i=0;i<length;i++){
		for(int j=0;j<length;j++){
			for(int lr=0;lr<2;lr++){
				if(i!=j){
					feat_temp->fill_one(assign_x,x,i,j);
					assign_x += idim;
				}
			}
		}
	}
	//mach evaluate
	int remain = num_pair;
	int bsize = m->GetBsize();
	REAL* xx = mach_x;
	REAL* yy = mach_y;
	while(remain > 0){
		int n=0;
		if(remain >= bsize)
			n = bsize;
		else
			n = remain;
		remain -= bsize;
		m->SetDataIn(xx);
		m->Forw(n);
		memcpy(yy, m->GetDataOut(), sizeof(REAL)*n);
		yy += n;
		xx += n*idim;
	}
	REAL* assign_y = mach_y;
	for(int i=0;i<length;i++){
		for(int j=0;j<length;j++){
			if(i!=j){
				//special o1 machine
				assign_y++;
				tmp_scores[i*length+j] = *assign_y++;
			}
		}
	}
	delete []mach_x;
	delete []mach_y;
	return tmp_scores;
}

vector<int>* Method6_O2sib::m6_parse_o2sib(DependencyInstance* x)
{
	int idim = feat_gen->get_xdim();
	int odim = mach->GetOdim();
	FeatureGenO2sib* feat_o2 = (FeatureGenO2sib*)feat_gen;	//force it
	// one sentence
	int length = x->forms->size();
	int num_allocated = length*length*length;
	int num_togo = 0;
	double *tmp_scores = new double[length*length*length];
	for(int i=0;i<length*length*length;i++)
		tmp_scores[i] = DOUBLE_LARGENEG;
	REAL *mach_x = new REAL[num_allocated*idim];	//num_allocated is more than needed
	REAL *mach_y = new REAL[num_allocated*odim];
	REAL* assign_x = mach_x;
	for(int s=0;s<length;s++){
		for(int t=s+1;t<length;t++){
			//s<->t
			if(!CONF_pos_filter || feat_o2->allowed_pair(x,s,t,-1)){
				feat_gen->fill_one(assign_x,x,s,t,-1);
				assign_x += idim;
				num_togo += 1;
			}
			if(!CONF_pos_filter || feat_o2->allowed_pair(x,t,s,-1)){
				feat_gen->fill_one(assign_x,x,t,s,-1);
				assign_x += idim;
				num_togo += 1;
			}
		}
	}
	for(int s=0;s<length;s++){
		for(int c=s+1;c<length;c++){
			for(int t=c+1;t<length;t++){
				//s c t
				if(!CONF_pos_filter || feat_o2->allowed_pair(x,s,t,c)){
					feat_gen->fill_one(assign_x,x,s,t,c);
					assign_x += idim;
					num_togo += 1;
				}
				if(!CONF_pos_filter || feat_o2->allowed_pair(x,t,s,c)){
					feat_gen->fill_one(assign_x,x,t,s,c);
					assign_x += idim;
					num_togo += 1;
				}
			}
		}
	}
	//forward through nn
	int remain = num_togo;
	int bsize = mach->GetBsize();
	REAL* xx = mach_x;
	REAL* yy = mach_y;
	while(remain > 0){
		int n=0;
		if(remain >= bsize)
			n = bsize;
		else
			n = remain;
		remain -= bsize;
		mach->SetDataIn(xx);
		mach->Forw(n);
		memcpy(yy, mach->GetDataOut(), odim*sizeof(REAL)*n);
		yy += n*odim;
		xx += n*idim;
	}
	//and assign the scores
#define TMP_GET_MAXINDEX(o_dim,a,assign) {a=0;double temp=*assign;for(int c=0;c<o_dim;c++) if((*assign++)>temp) a=c;}
#define TMP_GET_ONE(o_dim,a,assign) {\
	if(o_dim>1){\
		if(CONF_NN_scoremax){TMP_GET_MAXINDEX(o_dim,a,assign)}\
		else{a=0;for(int c=0;c<o_dim;c++) a+=(*assign++)*c;}\
	}\
	else{a=*assign++;}}

	REAL* assign_y = mach_y;
	double answer = 0;
	for(int s=0;s<length;s++){
		for(int t=s+1;t<length;t++){
			//s<->t
			if(!CONF_pos_filter || feat_o2->allowed_pair(x,s,t,-1)){
				TMP_GET_ONE(odim,answer,assign_y)
				tmp_scores[get_index2_o2sib(length,s,s,t)] = answer;
			}
			if(!CONF_pos_filter || feat_o2->allowed_pair(x,t,s,-1)){
				TMP_GET_ONE(odim,answer,assign_y)
				tmp_scores[get_index2_o2sib(length,t,t,s)] = answer;
			}
		}
	}
	for(int s=0;s<length;s++){
		for(int c=s+1;c<length;c++){
			for(int t=c+1;t<length;t++){
				//s c t
				if(!CONF_pos_filter || feat_o2->allowed_pair(x,s,t,c)){
					TMP_GET_ONE(odim,answer,assign_y)
					tmp_scores[get_index2_o2sib(length,s,c,t)] = answer;
				}
				if(!CONF_pos_filter || feat_o2->allowed_pair(x,t,s,c)){
					TMP_GET_ONE(odim,answer,assign_y)
					tmp_scores[get_index2_o2sib(length,t,c,s)] = answer;
				}
			}
		}
	}

	//combine o1 scores
	if(CONF_NN_O2sib_o1mach.length() > 0){
		FeatureGenO1* feat_temp_o1 = new FeatureGenO1(dict,CONF_x_window,CONF_add_distance,CONF_add_pos);
		ifstream ifs;
		ifs.open(CONF_NN_O2sib_o1mach.c_str(),ios::binary);
		Mach* mach_o1 = Mach::Read(ifs);
		ifs.close();
		double* score_o1 = get_score_from_o1(x,feat_temp_o1,mach_o1);
		for(int i=0;i<length;i++){
			for(int j=0;j<length;j++){
				if(i!=j){
					double score_tmp = score_o1[i*length+j];
					tmp_scores[get_index2_o2sib(length,i,i,j)] += score_tmp;
					//really lazy here
					for(int c=i+1;c<j;c++)
						tmp_scores[get_index2_o2sib(length,i,c,j)] += score_tmp;
					for(int c=j+1;c<i;c++)
						tmp_scores[get_index2_o2sib(length,i,c,j)] += score_tmp;
				}
			}
		}
		delete []score_o1;
	}

	//parse it with o2sib
	vector<int> *ret = decodeProjective_o2sib(length,tmp_scores);
	delete []mach_x;
	delete []mach_y;
	delete []tmp_scores;
	return ret;
}

