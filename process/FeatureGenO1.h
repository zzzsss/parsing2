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
private:
	IntHashMap* filter_map;
public:
	FeatureGenO1(Dict* d,int w,int di);
	virtual int fill_one(REAL*,DependencyInstance*,int head,int mod);
	virtual void deal_with_corpus(vector<DependencyInstance*>*);
	virtual ~FeatureGenO1(){}

	//for extra information(1.filter )
	virtual void add_filter(vector<DependencyInstance*>*);
	int allowed_pair(int head,int modif);
	virtual void read_extra_info(string f);
	virtual void write_extra_info(string f);
};

#endif /* FEATUREGEBO1_H_ */
