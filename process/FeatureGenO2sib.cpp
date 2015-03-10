/*
 * FeatureGenO2sib.cpp
 *
 *  Created on: 2015Äê3ÔÂ9ÈÕ
 *      Author: zzs
 */

#include "FeatureGenO2sib.h"

FeatureGenO2sib::FeatureGenO2sib(Dict* d,int w,int di,int apos):FeatureGen(d,w,di,apos)
{
	xdim = 3*w;
	if(apos)
		xdim *= 2;
	if(di)
		xdim += 2;	//2 for distance
}


int FeatureGenO2sib::fill_one(REAL* to_fill,DependencyInstance* ins,int head,int mod,int mod_center)
{
	//head-w,mod_center-w,mod-w,head-pos,mod_center-w,mod-pos,distances
	int backward = window_size/2;	//window_size should be odd...
	int leng = ins->forms->size();
	//-words
	for(int i=head-backward;i<=head+backward;i++){
		if(i<0)
			*to_fill = dictionary->get_index(&dictionary->WORD_START,0);	//must exist
		else if(i>=leng)
			*to_fill = dictionary->get_index(&dictionary->WORD_END,0);	//must exist
		else
			*to_fill = ins->index_forms->at(i);
		to_fill ++;
	}
	if(mod_center<0){
		//use dummy
		if(head < mod)
			*to_fill = dictionary->get_index(&dictionary->WORD_DUMMY_L,0);
		else
			*to_fill = dictionary->get_index(&dictionary->WORD_DUMMY_R,0);
		to_fill ++;
	}
	else{
		for(int i=mod_center-backward;i<=mod_center+backward;i++){
			if(i<0)
				*to_fill = dictionary->get_index(&dictionary->WORD_START,0);	//must exist
			else if(i>=leng)
				*to_fill = dictionary->get_index(&dictionary->WORD_END,0);	//must exist
			else
				*to_fill = ins->index_forms->at(i);
			to_fill ++;
		}
	}
	for(int i=mod-backward;i<=mod+backward;i++){
		if(i<0)
			*to_fill = dictionary->get_index(&dictionary->WORD_START,0);	//must exist
		else if(i>=leng)
			*to_fill = dictionary->get_index(&dictionary->WORD_END,0);	//must exist
		else
			*to_fill = ins->index_forms->at(i);
		to_fill ++;
	}

	if(pos_add){
	//-pos
	for(int i=head-backward;i<=head+backward;i++){
		if(i<0)
			*to_fill = dictionary->get_index(&dictionary->POS_START,0);	//must exist
		else if(i>=leng)
			*to_fill = dictionary->get_index(&dictionary->POS_END,0);	//must exist
		else
			*to_fill = ins->index_pos->at(i);
		to_fill ++;
	}
	if(mod_center<0){
		//use dummy
		if(head < mod)
			*to_fill = dictionary->get_index(&dictionary->POS_DUMMY_L,0);
		else
			*to_fill = dictionary->get_index(&dictionary->POS_DUMMY_R,0);
		to_fill ++;
	}
	else{
		for(int i=mod_center-backward;i<=mod_center+backward;i++){
			if(i<0)
				*to_fill = dictionary->get_index(&dictionary->POS_START,0);	//must exist
			else if(i>=leng)
				*to_fill = dictionary->get_index(&dictionary->POS_END,0);	//must exist
			else
				*to_fill = ins->index_pos->at(i);
			to_fill ++;
		}
	}
	for(int i=mod-backward;i<=mod+backward;i++){
		if(i<0)
			*to_fill = dictionary->get_index(&dictionary->POS_START,0);	//must exist
		else if(i>=leng)
			*to_fill = dictionary->get_index(&dictionary->POS_END,0);	//must exist
		else
			*to_fill = ins->index_pos->at(i);
		to_fill ++;
	}
	}

	//-maybe distance
	if(distance){
		if(mod_center<0){
			*to_fill = dictionary->get_index(&dictionary->DISTANCE_DUMMY,0);
			to_fill ++;
		}
		else{
			*to_fill = dictionary->get_index(head-mod_center);
			to_fill ++;
		}
		*to_fill = dictionary->get_index(head-mod);
		to_fill ++;
	}
	return xdim;
}
