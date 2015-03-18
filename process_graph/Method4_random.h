/*
 * Method4_random.h
 *
 *  Created on: 14 Jan, 2015
 *      Author: z
 */

#ifndef PARSING2_PROCESS_METHOD4_RANDOM_H_
#define PARSING2_PROCESS_METHOD4_RANDOM_H_

//method4: like method2, but choose random (not all)
#include "Process.h"

class Method4_random: public Process{
private:
	//REAL* data;
	REAL* gradient;
	REAL* temp_data;
	int curr_sentence;
	int all_sentence;

	int curr_num;	//current pointer for one sentence
	int this_num;	//all for one sentence

protected:
	virtual void each_write_mach_conf();
	virtual void each_prepare_data_oneiter();
	virtual REAL* each_next_data(int*);
	virtual void each_get_grad(int);

public:
	Method4_random(string conf):Process(conf){
		//data = 0;
		gradient = 0;
		temp_data = 0;
	}
};



#endif /* PARSING2_PROCESS_METHOD4_RANDOM_H_ */
