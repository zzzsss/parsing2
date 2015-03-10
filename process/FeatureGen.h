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

public:
	int get_xdim(){
		return xdim;
	}
	virtual ~FeatureGen(){}
	virtual int fill_one(REAL*,DependencyInstance*,int head,int mod,int mod_center)=0;
	virtual void deal_with_corpus(vector<DependencyInstance*>*);
	FeatureGen(Dict* d,int w,int di,int apos):dictionary(d),window_size(w),distance(di),xdim(0),pos_add(apos){}

	//for extra information(1.filter )
	virtual void add_filter(vector<DependencyInstance*>*){};
	virtual void read_extra_info(string f){};
	virtual void write_extra_info(string f){};
};

#endif /* FEATUREGEN_H_ */
