/*
 * Dict.h
 *
 *  Created on: Dec 19, 2014
 *      Author: zzs
 */

#ifndef DICT_H_
#define DICT_H_
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <algorithm>
#include <fstream>
#include "HashMap.h"
#include "../tools/DependencyInstance.h"
using namespace std;

#define CONS_dict_map_size 100000
#define CONS_distance_max 10

//the dictionary
class Dict{

	HashMap* maps;	//feature maps
	int dict_num;
	vector<string*>* real_word_list;
	//
	int distance_max;

	//used only when building
	int statistic_info;	//whether gather statistic_info
	int remove_single;	//remove rare word -- backoff to pos -- indicate the number

public:
	//symbols
	static string POS_START,POS_END,POS_UNK;
	static string WORD_START,WORD_END,WORD_UNK;
	static string WORD_BACKOFF_POS_PREFIX;
	//for o2
	// -- this is: h-dl-c(right-arc) or h-dr-c(left-arc)
	static string WORD_DUMMY_L,WORD_DUMMY_R,POS_DUMMY_L,POS_DUMMY_R;
	static string DISTANCE_DUMMY;

	string* get_distance_str(int n,int way=1);
	int get_index(string* word,string* backoff_pos);	//word or pos
	int get_index(int distance);						//distance
	int get_word_index(string* word);	//for outer use

	void write(string file);
	static Dict* read(string file);

	int get_count(){return dict_num;}
	vector<string*>* get_real_words() {return real_word_list;}

	Dict(string file);
	Dict(int remove,int stat=1,int dist=CONS_distance_max);
	~Dict(){
		delete maps;	//leak some memory, but that's alright
	}
	void construct_dictionary(vector<DependencyInstance*>*);
};


#endif /* DICT_H_ */
