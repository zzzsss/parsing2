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
#include "../cslm/MachTab.h"
#include "../cslm/Mach.h"
#include "../cslm/MachMulti.h"
#include <fstream>
#include <cstring>
using namespace std;

class CslmInterface: public NNInterface{
protected:
	Mach *mach;
public:
	virtual void SetDataIn(REAL *data)		{mach->SetDataIn(data);}
	virtual void Forw(int x)				{mach->Forw(x);}
	virtual void Backw(const float lrate, const float wdecay, int s){mach->Backw(lrate,wdecay,s);}
	virtual void SetGradOut(REAL *data)		{mach->SetGradOut(data);}
	virtual void Write(string name){
		ofstream fs;
		fs.open(name.c_str(),ios::binary);
		mach->Write(fs);
		fs.close();
	}
	virtual ulong GetNbBackw()			{return mach->GetNbBackw();}
	virtual int GetIdim()				{return mach->GetIdim();}
	virtual int GetOdim()				{return mach->GetOdim();}
	virtual int GetBsize()				{return mach->GetBsize();}
	virtual REAL* GetDataOut()			{return mach->GetDataOut();}

	virtual REAL* mach_forward(REAL* assign,int all);	//allocated here
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

	CslmInterface(Mach* m){
		mach = m;
	}
	static CslmInterface* Read(string name){
		ifstream ifs;
		ifs.open(name.c_str(),ios::binary);
		Mach* m = Mach::Read(ifs);
		ifs.close();
		if(m)
			return new CslmInterface(m);
		else
			return 0;
	}
	static CslmInterface* create_one(parsing_conf* p,int dict_count,int xdim,int outdim);
};



#endif /* NN_CSLMINTERFACE_H_ */
