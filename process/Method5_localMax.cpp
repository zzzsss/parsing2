/*
 * Method5_loaclMax.cpp
 *
 *  Created on: 2015Äê3ÔÂ4ÈÕ
 *      Author: zzs
 */

#include "Method5_localMax.h"

void Method5_localMax::each_write_mach_conf()
{
	//only one output --- the score
	write_conf(1);
}

void Method5_localMax::each_prepare_data_oneiter()
{
	//simple method --- once one sentence

	if(temp_data == 0){
		const int MAX_LEN_HERE = 1000;
		scores_table = new REAL*[MAX_LEN_HERE];
		for(int i=0;i<MAX_LEN_HERE;i++)
			scores_table[i] = new REAL[MAX_LEN_HERE];

		temp_data = new REAL[100000*mach->GetIdim()];	//100000 should be enough??
		gradient = new REAL[mach->GetBsize()*mach->GetOdim()];
		mach->SetGradOut(gradient);
		cout << "--Ok, ready for m5." << endl;
	}
		this_num = curr_num = 0;
		curr_sentence = 0;
		all_sentence = training_corpus->size();
}

REAL* Method5_localMax::each_next_data(int* size)
{
	//find the max-score false-dependency --> this is just one sample each
	while(this_num <= 0){
		//get new sentence
		if(curr_sentence >= all_sentence){
			return 0;
		}
		DependencyInstance* x = training_corpus->at(curr_sentence);
		curr_sentence++;
		this_num = curr_num = 0;
		REAL* assign_x = temp_data;
		int one_len = x->length();
		if(one_len <= 2)
			continue;
		//forward to get scores and get training samples
		get_scores_once(scores_table,x);
		for(int i=1;i<one_len;i++){
			int right = x->heads->at(i);
			REAL here_max = Negative_Infinity;
			//step1 : change parent
			int here_one = -1;
			for(int h=0;h<one_len;h++){
				if(h==right || h==i)
					continue;
				if(scores_table[h][i] > here_max){
					here_max = scores_table[h][i];
					here_one = h;
				}
			}
			if(here_one >= 0){
				//no filter here
				feat_gen->fill_one(assign_x,x,right,i);
				assign_x += mach->GetIdim();
				feat_gen->fill_one(assign_x,x,here_one,i);
				assign_x += mach->GetIdim();
				this_num += 2;
			}
			//step2: change child
			if(CONF_NN_example){
				for(int c=0;c<one_len;c++){
					if(x->heads->at(c)==right || c==right || c==i)
						continue;
					if(scores_table[right][c] > here_max){
						here_max = scores_table[right][c];
						here_one = c;
					}
				}
				if(here_one >= 0){
					//no filter here
					feat_gen->fill_one(assign_x,x,right,i);
					assign_x += mach->GetIdim();
					feat_gen->fill_one(assign_x,x,right,here_one);
					assign_x += mach->GetIdim();
					this_num += 2;
				}
			}
		}
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

void Method5_localMax::each_get_grad(int size)
{
	set_pair_gradient(mach->GetDataOut(),gradient,size);
}

//same as the part of parse_o1, but avoid sub-procedure the original code, so get the part here
void Method5_localMax::get_scores_once(REAL** table,DependencyInstance* x)
{
	int idim = feat_gen->get_xdim();
	int length = x->forms->size();
	double *tmp_scores = new double[length*length*2];
	//construct scores using nn
	int num_pair = length*(length-1);	//2 * (0+(l-1))*l/2
	REAL *mach_x = new REAL[num_pair*idim];
	REAL *mach_y = new REAL[num_pair];
	REAL* assign_x = mach_x;
	for(int ii=0;ii<length;ii++){
		for(int j=ii+1;j<length;j++){
			for(int lr=0;lr<2;lr++){
				//build mach_x
				if(lr==E_RIGHT)
					feat_gen->fill_one(assign_x,x,ii,j);
				else
					feat_gen->fill_one(assign_x,x,j,ii);
				assign_x += idim;
			}
		}
	}
	//mach evaluate
	int remain = num_pair;
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
		memcpy(yy, mach->GetDataOut(), sizeof(REAL)*n);
		yy += n;
		xx += n*idim;
	}
	REAL* assign_y = mach_y;
	for(int ii=0;ii<length;ii++){
		for(int j=ii+1;j<length;j++){
			for(int lr=0;lr<2;lr++){
				//build mach_x
				if(lr==E_RIGHT)
					table[ii][j] = *assign_y;
				else
					table[j][ii] = *assign_y;
				assign_y++;
			}
		}
	}
	delete []mach_x;
	delete []mach_y;
	delete []tmp_scores;
}
