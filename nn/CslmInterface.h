/*
 * CslmInterface.h
 *
 *  Created on: 2015Äê3ÔÂ31ÈÕ
 *      Author: zzs
 */

#ifndef NN_CSLMINTERFACE_H_
#define NN_CSLMINTERFACE_H_

#include "NNInterface.h"
#include "../cslm/Mach.h"
#include "../cslm/MachConfig.h"
#include <fstream>
#include <cstring>
#include <cstdlib>
using namespace std;

#define CSLM_MACHINE_DESCRIBE_SUFFIX ".precalc"	//for pre-calculation

class CslmInterface: public NNInterface{
protected:
	Mach *mach;
	//for pre-calculation
	int embed_dim;
	int embed_layer_num;
	int dict_num;
	int second_layer_dim;
	REAL* pre_calc_table;	//pre-calculation table
	void mach_split_share();
public:
	virtual void SetDataIn(REAL *data)		{mach->SetDataIn(data);}
	virtual REAL* GetDataOut()			{return mach->GetDataOut();}
	virtual int GetIdim()				{return mach->GetIdim();}
	virtual int GetOdim()				{return mach->GetOdim();}
	virtual int GetWidth()				{return mach->GetBsize();}
	virtual void SetWidth()				{nnError(NNERROR_NotImplemented);}

	virtual void Forw(int x)				{mach->Forw(x);}
	virtual void Backw(const float lrate, const float wdecay, int s){mach->Backw(lrate,wdecay,s);}
	virtual void SetGradOut(REAL *data)		{mach->SetGradOut(data);}
	virtual ulong GetNbBackw()			{return mach->GetNbBackw();}

	virtual void Backw_store(const float lrate, const float wdecay, int bs){nnError(NNERROR_NotImplemented);}
	virtual void Backw_update(){nnError(NNERROR_NotImplemented);}

	virtual REAL* mach_forward(REAL* assign,int all);	//allocated here
	virtual void Write(string name);

	virtual REAL* get_tab(){
		MachMulti* m = (MachMulti*)mach;
		m = (MachMulti*)(m->MachGet(0));
		MachTab* mm = (MachTab*)(m->MachGet(0));
		REAL* tab = mm->GetTabAdr();
		return tab;
	}
	virtual void set_tab(REAL* x){
		MachMulti* m = (MachMulti*)mach;
		m = (MachMulti*)(m->MachGet(0));
		MachTab* mm = (MachTab*)(m->MachGet(0));
		mm->SetTabAdr(x);
	}
	virtual void clone_tab(REAL* x,int all){
		MachMulti* m = (MachMulti*)mach;
		m = (MachMulti*)(m->MachGet(0));
		MachTab* mm = (MachTab*)(m->MachGet(0));
		REAL* tab = mm->GetTabAdr();
		memcpy(tab,x,sizeof(REAL)*all);
	}

	virtual void pre_calc();
	virtual void DEBUG_pre_calc();

	CslmInterface(Mach* m,int e_dim,int e_num,int d_num,int s_dim,REAL* t=0){
		mach = m;
		embed_dim = e_dim;
		embed_layer_num = e_num;
		dict_num = d_num;
		second_layer_dim = s_dim;
		pre_calc_table = t;
	}
	static CslmInterface* Read(string name);
	static CslmInterface* create_one(parsing_conf* p,FeatureGen* f,int outdim);
};



#endif /* NN_CSLMINTERFACE_H_ */
