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

class FeatureGen{
	Dict* dictionary;
	int window_size;
	int distance;	//whether add distance feature

protected:
	int xdim;

public:
	int get_xdim(){
		return xdim;
	}
	virtual ~FeatureGen(){}
	virtual int fill_one(int*,DependencyInstance*,int head,int mod)=0;
	FeatureGen(Dict* d,int w,int di):dictionary(d),window_size(w),distance(di),xdim(0){}
};

#endif /* FEATUREGEN_H_ */
