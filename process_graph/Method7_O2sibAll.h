/*
 * Method7_O2sibAll.h
 *
 *  Created on: 2015Äê3ÔÂ15ÈÕ
 *      Author: zzs
 */

#ifndef PROCESS_METHOD7_O2SIBALL_H_
#define PROCESS_METHOD7_O2SIBALL_H_

#include "Method6_O2sib.h"
//method7: also o2sib, but like m1 possible with pos-filters

class Method7_O2sibAll: public Method6_O2sib{
protected:
	REAL* target;
public:
	Method7_O2sibAll(string conf):Method6_O2sib(conf),target(0){}
	virtual void each_prepare_data_oneiter();
	virtual void each_get_grad(int);
	virtual void each_write_mach_conf();
};


#endif /* PROCESS_METHOD7_O2SIBALL_H_ */
