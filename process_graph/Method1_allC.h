/*
 * Method1_allC.h
 *
 *  Created on: Dec 25, 2014
 *      Author: zzs
 */

#ifndef METHOD1_ALLC_H_
#define METHOD1_ALLC_H_

//method 1: the original old method
//-- all pairs and 0-1 cross entropy
#include "Process.h"

class Method1_allC: public Process{
	friend class Method8_O2sibWitho1;
private:
	REAL* data;
	REAL* target;
	int current;
	int end;
	REAL* gradient;

protected:
	virtual void each_write_mach_conf();
	virtual void each_prepare_data_oneiter();
	virtual REAL* each_next_data(int*);
	virtual void each_get_grad(int);

public:
	Method1_allC(string conf):Process(conf){
		data = 0;
		target = 0;
		gradient = 0;
		end = current = 0;
	}
};



#endif /* METHOD1_ALLC_H_ */
