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

int FeatureGenO1::fill_one(int*,DependencyInstance*,int head,int mod)
{
	return 0;
}
