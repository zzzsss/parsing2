/*
 * FeatureGenO2sib.h
 *
 *  Created on: 2015Äê3ÔÂ9ÈÕ
 *      Author: zzs
 */

#ifndef PROCESS_FEATUREGENO2SIB_H_
#define PROCESS_FEATUREGENO2SIB_H_

#include "FeatureGen.h"
class FeatureGenO2sib: public FeatureGen{
public:
	FeatureGenO2sib(Dict* d,int w,int di,int apos);
	virtual int fill_one(REAL*,DependencyInstance*,int head,int mod,int mod_center);
	virtual ~FeatureGenO2sib(){}
};



#endif /* PROCESS_FEATUREGENO2SIB_H_ */
