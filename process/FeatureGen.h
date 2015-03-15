/*
 * FeatureGen.h
 *
 *  Created on: Dec 19, 2014
 *      Author: zzs
 */

#ifndef FEATUREGEN_H_
#define FEATUREGEN_H_
#include "Dict.h"
#include "../tools/DependencyInstance.h"
#include "../cslm/Mach.h"

class FeatureGen{
protected:
	Dict* dictionary;
	int window_size;
	int pos_add;	//whether add pos
	int distance;	//whether add distance feature

protected:
	int xdim;
	IntHashMap* filter_map;
	static const int ASSUMING_MAX_POS = 512;	//pos always in the first 500 of the dict
	static const int ASSUMING_MAX_POS_SHIFT = 9;

public:
	int get_xdim(){
		return xdim;
	}
	virtual ~FeatureGen(){}
	virtual int fill_one(REAL*,DependencyInstance*,int head,int mod,int mod_center=-1)=0;
	virtual void deal_with_corpus(vector<DependencyInstance*>*);
	FeatureGen(Dict* d,int w,int di,int apos):dictionary(d),window_size(w),distance(di),xdim(0),pos_add(apos),filter_map(0){}

	//for extra information(1.filter )
	virtual void add_filter(vector<DependencyInstance*>*){}

	void read_extra_info(string f);
	void write_extra_info(string f);
};

#endif /* FEATUREGEN_H_ */
