/*
 * FeatureGen.cpp
 *
 *  Created on: 2015Äê3ÔÂ9ÈÕ
 *      Author: zzs
 */

#include "FeatureGen.h"

void FeatureGen::deal_with_corpus(vector<DependencyInstance*>* c)
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


