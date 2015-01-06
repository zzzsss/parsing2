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
	filter_map = 0;
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


#define ASSUMING_MAX_POS 500
void FeatureGenO1::add_filter(vector<DependencyInstance*>* c)
{
	filter_map = new IntHashMap();
	int size = c->size();
	//add possible pos pairs, assuming pos index is less than 500 (depend on dictionary)
	for(int i=0;i<size;i++){
		DependencyInstance* x = c->at(i);
		int length = x->length();
		for(int mod=1;mod<length;mod++){	//exclude root
			int head = x->heads->at(mod);
			int adding = x->index_pos->at(head)*ASSUMING_MAX_POS
					+ x->index_pos->at(mod);
			filter_map->insert(pair<int, int>(adding,0));
		}
	}
}

//not virtual, only for Order 1
int FeatureGenO1::allowed_pair(int head,int mod)
{
	int test = head * ASSUMING_MAX_POS + mod;
	if(filter_map->find(test) != filter_map->end())
		return 1;
	else
		return 0;
}

//io
void FeatureGenO1::write_extra_info(string file)
{
	//warning when error
	printf("-Writing feat-file to %s,size %d.\n",file.c_str(),filter_map->size());
	ofstream fout;
	fout.open(file.c_str(),ofstream::out);
	//fout << filter_map->size() << "\n";
	for(IntHashMap::iterator i = filter_map->begin();i!=filter_map->end();i++){
		fout << i->first << endl;
	}
	fout.close();
	printf("-Writing finished.\n");
}

void FeatureGenO1::read_extra_info(string file)
{
	filter_map = new IntHashMap();
	printf("-Reading feat-file from %s.\n",file.c_str());
	ifstream fin;
	fin.open(file.c_str(),ifstream::in);
	while(fin){
		int temp;
		fin >> temp;
		filter_map->insert(pair<int, int>(temp,0));
	}
	printf("-Reading finished,size %d.\n",filter_map->size());
}
