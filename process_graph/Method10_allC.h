/*
 * Method10_allC.h
 *
 *  Created on: 2015Äê4ÔÂ5ÈÕ
 *      Author: zzs
 */

#ifndef PROCESS_GRAPH_METHOD10_ALLC_H_
#define PROCESS_GRAPH_METHOD10_ALLC_H_

//method 10: some slightly changes from m1
#include "Process.h"

class Method10_allC: public Process{
private:
	REAL* data;
	REAL* target;
	int current;
	int end;
	REAL* gradient;

	REAL gright,gwrong;

protected:
	virtual int each_get_mach_outdim(){return 1;}
	virtual void each_prepare_data_oneiter();
	virtual REAL* each_next_data(int*);
	virtual void each_get_grad(int);

public:
	Method10_allC(string conf):Process(conf){
		data = 0;
		target = 0;
		gradient = 0;
		end = current = 0;
	}
};



#endif /* PROCESS_GRAPH_METHOD10_ALLC_H_ */
