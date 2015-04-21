/*
 * Process_parse.cpp
 *
 *  Created on: 2015��3��18��
 *      Author: zzs
 */

#include "Process.h"
#include "../algorithms/Eisner.h"
#include "../algorithms/EisnerO2sib.h"

//-------------------- these two also static methods -----------------------------
//return Score[length][length]
double* Process::get_scores_o1(DependencyInstance* x,parsing_conf* zp,NNInterface * zm,FeatureGen* zf)
{
	int idim = zf->get_xdim();
	int odim = zm->GetOdim();
	//default order1 parsing
	int length = x->forms->size();
	double *tmp_scores = new double[length*length];
	for(int i=0;i<length*length;i++)
		tmp_scores[i] = DOUBLE_LARGENEG;
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
	REAL* mach_y = zm->mach_forward(mach_x,num_pair);
	REAL* assign_y = mach_y;
	for(int ii=0;ii<length;ii++){
		for(int j=0;j<length;j++){
			if(ii!=j){
				int index = get_index2(length,ii,j);
				if(odim > 1){
					double temp = 0;
					for(int c=0;c<odim;c++)
						temp += (*assign_y++)*c;
					tmp_scores[index] = temp;
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
double* Process::get_scores_o2sib(DependencyInstance* x,parsing_conf* zp,NNInterface * zm,FeatureGen* zf,bool* score_o1)
{
	int idim = zf->get_xdim();
	int odim = zm->GetOdim();
	int whether_o1_filter = 0;
	if(score_o1 && zp->CONF_NN_highO_o1filter)
		whether_o1_filter = 1;
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
			if(!whether_o1_filter || !score_o1[get_index2(length,s,t)]){
				zf->fill_one(assign_x,x,s,t,-1);
				assign_x += idim;
				num_togo += 1;
			}
			if(!whether_o1_filter || !score_o1[get_index2(length,t,s)]){
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
				if(!whether_o1_filter || !score_o1[get_index2(length,s,t)] || !score_o1[get_index2(length,s,c)]){
					zf->fill_one(assign_x,x,s,t,c);
					assign_x += idim;
					num_togo += 1;
				}
				if(!whether_o1_filter || !score_o1[get_index2(length,t,s)] || !score_o1[get_index2(length,t,c)]){
					zf->fill_one(assign_x,x,t,s,c);
					assign_x += idim;
					num_togo += 1;
				}
			}
		}
	}
	//forward
	REAL* mach_y = zm->mach_forward(mach_x,num_togo);
	//and assign the scores
#define TMP_GET_ONE(o_dim,a,assign) {\
	if(o_dim>1){\
		{a=0;for(int c=0;c<o_dim;c++) a+=(*assign++)*c;}\
	}\
	else{a=*assign++;}}

	REAL* assign_y = mach_y;
	double answer = 0;
	for(int s=0;s<length;s++){
		for(int t=s+1;t<length;t++){
			//s<->t
			if(!whether_o1_filter || !score_o1[get_index2(length,s,t)]){
				TMP_GET_ONE(odim,answer,assign_y)
				tmp_scores[get_index2_o2sib(length,s,s,t)] = answer;
			}
			if(!whether_o1_filter || !score_o1[get_index2(length,t,s)]){
				TMP_GET_ONE(odim,answer,assign_y)
				tmp_scores[get_index2_o2sib(length,t,t,s)] = answer;
			}
		}
	}
	for(int s=0;s<length;s++){
		for(int c=s+1;c<length;c++){
			for(int t=c+1;t<length;t++){
				//s c t
				if(!whether_o1_filter || !score_o1[get_index2(length,s,t)] || !score_o1[get_index2(length,s,c)]){
					TMP_GET_ONE(odim,answer,assign_y)
					tmp_scores[get_index2_o2sib(length,s,c,t)] = answer;
				}
				if(!whether_o1_filter || !score_o1[get_index2(length,t,s)] || !score_o1[get_index2(length,t,c)]){
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

#include "Process_helper.cpp"	//static functions
//-------------------- parsing non-static methods -----------------------------
vector<int>* Process::parse_o1(DependencyInstance* x)
{
	double *tmp_scores = get_scores_o1(x,parameters,mach,feat_gen);
	if(parameters->CONF_score_prob)
		trans_o1(tmp_scores,x->length());
	vector<int> *ret = decodeProjective(x->length(),tmp_scores);
	delete []tmp_scores;
	return ret;
}

vector<int>* Process::parse_o2sib(DependencyInstance* x,double* score_of_o1)
{
	int length = x->length();
	bool *whether_cut_o1 = 0;
	if(score_of_o1 && parameters->CONF_NN_highO_o1filter){
		whether_cut_o1 = new bool[length*length];
		for(int i=0;i<length*length;i++){
			whether_cut_o1[i] = (score_noprob(score_of_o1[i])) ? true : false;
		}
	}
	double *tmp_scores = get_scores_o2sib(x,parameters,mach,feat_gen,whether_cut_o1);
	delete []whether_cut_o1;
	if(parameters->CONF_score_prob)
		trans_o2sib(tmp_scores,length);
	if(score_of_o1 && parameters->CONF_NN_highO_score_combine){
		if(parameters->CONF_score_prob)
			trans_o1(score_of_o1,length);
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

void Process::check_o1_filter(string m_name,string cutting)
{
	//MUST BE O1 MACH
	cout << "----- Check o1 filter(must be o1-mach)-----" << endl;
	parameters->CONF_NN_highO_o1filter_cut = atof(cutting.c_str());
	dict = new Dict(parameters->CONF_dict_file);
	mach = NNInterface::Read(m_name);
	dev_test_corpus = read_corpus(parameters->CONF_test_file);
	FeatureGenO1* feat_temp_o1 = new FeatureGenO1(dict,parameters->CONF_x_window,
			parameters->CONF_add_distance,parameters->CONF_add_pos,parameters->CONF_add_distance_parent);
	feat_temp_o1->deal_with_corpus(dev_test_corpus);
	if(mach->GetIdim() != feat_temp_o1->get_xdim()){
		cout << "Wrong mach...\n";
		exit(1);
	}
	int token_num = 0;	//token number
	int filter_wrong_count = 0;
	for(int ii=0;ii<dev_test_corpus->size();ii++){
		if(ii%100 == 0)
			cout << filter_wrong_count << "/" << token_num << endl;
		DependencyInstance* x = dev_test_corpus->at(ii);
		int length = x->forms->size();
		token_num += length - 1;
		double* scores_o1 = get_scores_o1(x,parameters,mach,feat_temp_o1);	//same parameters
		for(int i=1;i<length;i++){
			if(score_noprob(scores_o1[get_index2(length,x->heads->at(i),i)]))
				filter_wrong_count++;
			token_num++;
		}
		delete []scores_o1;
	}
	cout << "FINAL:" << filter_wrong_count << "/" << token_num << endl;
}


