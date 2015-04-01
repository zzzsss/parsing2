/*
 * NNInterface.h
 *
 *  Created on: 2015Äê3ÔÂ31ÈÕ
 *      Author: zzs
 */

#ifndef NN_NNINTERFACE_H_
#define NN_NNINTERFACE_H_

#include <vector>
using namespace std;
//machines
#define NN_HNAME_CSLM "HPerf"		//1

#include "../cslm/Tools.h"
#include "../cslm/Mach.h"
#include "../parts/Parameters.h"
#include "../parts/FeatureGen.h"

class NNInterface{
public:
	virtual void SetDataIn(REAL *data)=0;
	virtual void Forw(int)=0;
	virtual void Backw(const float lrate, const float wdecay, int)=0;
	virtual void SetGradOut(REAL *data)=0;
	virtual void Write(string name)=0;
	virtual ulong GetNbBackw()=0;
	virtual int GetIdim()=0;
	virtual int GetOdim()=0;
	virtual int GetBsize()=0;
	virtual REAL* GetDataOut()=0;

	virtual REAL* mach_forward(REAL* assign,int all)=0;	//allocated here
	virtual ~NNInterface(){}
	//tabs
	virtual REAL* get_tab()=0;
	virtual void set_tab(REAL* x)=0;
	virtual void clone_tab(REAL* x,int all)=0;

	static NNInterface* Read(string name);
	static NNInterface* create_one(parsing_conf* p,FeatureGen* f,int outdim);
};




#endif /* NN_NNINTERFACE_H_ */
