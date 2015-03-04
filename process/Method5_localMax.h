/*
 * Method4_random.h
 *
 *  Created on: 14 Jan, 2015
 *      Author: z
 */

#ifndef PARSING2_PROCESS_METHOD5_LOCALMAX_H_
#define PARSING2_PROCESS_METHOD5_LOCALMAX_H_

//method5: like online version, but choose local max as pairs
// kind of like m4 and m3
#include "Process.h"

class Method5_localMax: public Process{
private:
	//REAL* data;
	REAL* gradient;
	REAL* temp_data;
	int curr_sentence;
	int all_sentence;

	int curr_num;	//current pointer for one sentence
	int this_num;	//all for one sentence

	REAL** scores_table;
	void get_scores_once(REAL**,DependencyInstance*);

protected:
	virtual void each_write_mach_conf();
	virtual void each_prepare_data_oneiter();
	virtual REAL* each_next_data(int*);
	virtual void each_get_grad(int);

public:
	Method5_localMax(){
		//data = 0;
		gradient = 0;
		temp_data = 0;
	}
};

#endif
