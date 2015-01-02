/*
 * FeatureGebO1.cpp
 *
 *  Created on: Dec 19, 2014
 *      Author: zzs
 */

#include "FeatureGenO1.h"

FeatureGenO1::FeatureGenO1(Dict* d,int w,int di):FeatureGen(d,w,di)
{
	xdim = (2+2)*w + ((di>0)?1:0);
}

int FeatureGenO1::fill_one(REAL* to_fill,DependencyInstance* ins,int head,int mod)
{
	//head-w,mod-w,head-pos,mod-pos,distance
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
	for(int i=mod-backward;i<=mod+backward;i++){
		if(i<0)
			*to_fill = dictionary->get_index(&dictionary->WORD_START,0);	//must exist
		else if(i>=leng)
			*to_fill = dictionary->get_index(&dictionary->WORD_END,0);	//must exist
		else
			*to_fill = ins->index_forms->at(i);
		to_fill ++;
	}
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
	for(int i=mod-backward;i<=mod+backward;i++){
		if(i<0)
			*to_fill = dictionary->get_index(&dictionary->POS_START,0);	//must exist
		else if(i>=leng)
			*to_fill = dictionary->get_index(&dictionary->POS_END,0);	//must exist
		else
			*to_fill = ins->index_pos->at(i);
		to_fill ++;
	}
	//-maybe distance
	if(distance){
		*to_fill = dictionary->get_index(head-mod);
	}
	return xdim;
}

void FeatureGenO1::deal_with_corpus(vector<DependencyInstance*>* c)
{
	if(c->at(0)->index_forms)
		return;
	int size = c->size();
	for(int i=0;i<size;i++){
		DependencyInstance* x = c->at(i);
		int length = x->length();
		x->index_forms = new vector<int>();
		x->index_pos = new vector<int>();
		for(int ii=0;ii<length;ii++){
			(x->index_forms)->push_back(dictionary->get_index(x->forms->at(ii),x->postags->at(ii)));
			(x->index_pos)->push_back(dictionary->get_index(x->postags->at(ii),0));
		}
	}
}
