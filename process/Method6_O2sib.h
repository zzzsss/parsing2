/*
 * Method6_O2sib.h
 *
 *  Created on: 2015Äê3ÔÂ9ÈÕ
 *      Author: zzs
 */
#ifndef PARSING2_PROCESS_METHOD6_H_
#define PARSING2_PROCESS_METHOD6_H_

#include "Process.h"

//Method 6 : order2, siblings
//also pairwise like M2~5
class Method6_O2sib: public Process{
protected:
	REAL* data;
	int current;
	int end;
	REAL* gradient;

	virtual void each_write_mach_conf();
	virtual void each_prepare_data_oneiter();
	virtual REAL* each_next_data(int*);
	virtual void each_get_grad(int);

	vector<int>* m6_parse_o2sib(DependencyInstance* x);
public:
	Method6_O2sib(){
		current = end = 0;
		data = 0;
		gradient = 0;
	}
	virtual void each_get_featgen(int if_testing){
		if(if_testing){
			if(! feat_gen)	//when testing
				feat_gen = new FeatureGenO2sib(dict,CONF_x_window,CONF_add_distance,CONF_add_pos);
			if(CONF_pos_filter){
				feat_gen->read_extra_info(CONF_feature_file);
			}
			feat_gen->deal_with_corpus(dev_test_corpus);
		}
		else{
			feat_gen = new FeatureGenO2sib(dict,CONF_x_window,CONF_add_distance,CONF_add_pos);
			feat_gen->deal_with_corpus(training_corpus);
			if(CONF_pos_filter){
				feat_gen->add_filter(training_corpus);
				feat_gen->write_extra_info(CONF_feature_file);
			}
		}
	}
	virtual vector<int>* each_test_one(DependencyInstance* x){
		return m6_parse_o2sib(x);
	}
};

// ----------- for decoding o2sib ----------------
// -- just like ../tools/Eisner.h

#define Negative_Infinity_O2sib -1e100
#define E_LEFT_O2sib 0
#define E_RIGHT_O2sib 1
#define E_INCOM_O2sib 0
#define E_COM_O2sib 1
#define E_SIB_O2sib 2

//the index explanation --- C[len][len][2][3]
inline int get_index_o2sib(int len,int s,int t,int lr,int c)
{
	int key = s;
	key = key * len + t;
	key = key * 2 + lr;
	key = key * 3 + c;	//here *3
	return key;
}

//here different from Eisner.h::get_index2 (no direction -> it depends on h and m)
inline int get_index2_o2sib(int len,int h,int c,int m)
{
	//get for scores: S[h][c][m] (S[h][-][m]=>S[h][h][m])
	int key = h;
	key = key * len + c;
	key = key * len + m;
	return key;
}

#endif
