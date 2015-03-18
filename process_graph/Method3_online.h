/*
 * Method3_online.h
 *
 *  Created on: 9 Jan, 2015
 *      Author: z
 */

#ifndef PARSING2_PROCESS_METHOD3_ONLINE_H_
#define PARSING2_PROCESS_METHOD3_ONLINE_H_

//method3: error-driven online(use the err function of Method2)
// --- here the changes are huge
#include "Process.h"

class Method3_online: public Process{
private:
	REAL* data;
	REAL* gradient;
	REAL* temp_data;	//buffer
	int curr_sentence;
	int all_sentence;

	int buffer_num;	//number in buffer

	int all_tokens;
	int correct_tokens;

protected:
	virtual void each_write_mach_conf();
	virtual void each_prepare_data_oneiter();
	virtual REAL* each_next_data(int*);
	virtual void each_get_grad(int);

public:
	Method3_online(string conf):Process(conf){
		data = 0;
		gradient = 0;
		temp_data = 0;
	}
};



#endif /* PARSING2_PROCESS_METHOD3_ONLINE_H_ */
