/*
 * Process_parse.cpp
 *
 *  Created on: 2015Äê3ÔÂ9ÈÕ
 *      Author: zzs
 */

#include "Process.h"

vector<int>* Process::parse_o1(DependencyInstance* x)
{
	int idim = feat_gen->get_xdim();
	int odim = mach->GetOdim();
	//default order1 parsing
	int length = x->forms->size();
	double *tmp_scores = new double[length*length*2];
	//construct scores using nn
	int num_pair = length*(length-1);	//2 * (0+(l-1))*l/2
	REAL *mach_x = new REAL[num_pair*idim];
	REAL *mach_y = new REAL[num_pair*odim];
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
		memcpy(yy, mach->GetDataOut(), odim*sizeof(REAL)*n);
		yy += n*odim;
		xx += n*idim;
	}
	REAL* assign_y = mach_y;
	FeatureGenO1* feat_o1 = (FeatureGenO1*)feat_gen;	//force it
	for(int ii=0;ii<length;ii++){
		for(int j=ii+1;j<length;j++){
			for(int lr=0;lr<2;lr++){
				int index = get_index2(length,ii,j,lr);
				//if filter --- this is the easy way(but waste computation)
				if(CONF_pos_filter){
					int head = ii, modif = j;
					if(lr==E_LEFT){
						head = j;
						modif = ii;
					}
					if(!feat_o1->allowed_pair(x->index_pos->at(head),x->index_pos->at(modif))){
						tmp_scores[index] = DOUBLE_LARGENEG;	//is this neg enough??
						//skip forward
						for(int c=0;c<odim;c++)
							assign_y++;
						continue;
					}
				}
				//important ...
				double temp = 0;
				if(odim > 1){
					for(int c=0;c<odim;c++)
						temp += (*assign_y++)*c;
					tmp_scores[index] = temp;
				}
				else
					tmp_scores[index] = *assign_y++;
			}
		}
	}
	vector<int> *ret = decodeProjective(length,tmp_scores);
	delete []mach_x;
	delete []mach_y;
	delete []tmp_scores;
	return ret;
}
