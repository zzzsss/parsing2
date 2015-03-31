/*
 * Method8_O2sibWitho1.h
 *
 *  Created on: 2015.3.24
 *      Author: zzs
 */

#ifndef PROCESS_GRAPH_METHOD8_O2SIBWITHO1_H_
#define PROCESS_GRAPH_METHOD8_O2SIBWITHO1_H_

#include "Method7_O2sibAll.h"
#include "Method1_allC.h"
//method8: also o2sib, but like m1 possible with pos-filters and togther with o1-training
//-those two shares the embeding and use the same dict
class Method8_O2sibWitho1: public Method7_O2sibAll{
private:
	void m8_nn_train_one_iter();
protected:
	Method1_allC* m1;
	//re-write the main procedure
	virtual vector<int>* each_test_one(DependencyInstance* x);
public:
	Method8_O2sibWitho1(string conf):Method7_O2sibAll(conf){
		m1 = new Method1_allC(conf);
		m1->parameters->CONF_mach_conf_name += ".o1";
		m1->parameters->CONF_mach_name += ".o1";
		m1->parameters->CONF_NN_resample = 1;	//force it to be 1
		m1->parameters->CONF_pos_filter = 0;	//no-filter
	}
	virtual void train();
	virtual void test(string m_name);
};



#endif /* PROCESS_GRAPH_METHOD8_O2SIBWITHO1_H_ */
