/*
 * FeatureGebO1.h
 *
 *  Created on: Dec 19, 2014
 *      Author: zzs
 */

#ifndef FEATUREGEBO1_H_
#define FEATUREGEBO1_H_
#include "FeatureGen.h"

class FeatureGenO1: public FeatureGen{
public:
	FeatureGenO1(Dict* d,int w,int di);
	virtual int fill_one(REAL*,DependencyInstance*,int head,int mod);
	virtual void deal_with_corpus(vector<DependencyInstance*>*);
	virtual ~FeatureGenO1(){}
};

#endif /* FEATUREGEBO1_H_ */
