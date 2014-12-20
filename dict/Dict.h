/*
 * Dict.h
 *
 *  Created on: Dec 19, 2014
 *      Author: zzs
 */

#ifndef DICT_H_
#define DICT_H_
#include <iostream>
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
	//
	int distance_max;

	//used only when building
	int statistic_info;	//whether gather statistic_info
	int remove_single;	//remove single word -- backoff to pos

public:
	//symbols
	static string POS_START,POS_END,POS_UNK;
	static string WORD_START,WORD_END,WORD_UNK;
	static string WORD_BACKOFF_POS_PREFIX;

	string* get_distance_str(int n);
	int get_index(string* word,string* backoff_pos);	//word or pos
	int get_index(int distance);						//distance
	int get_word_index(string* word);	//for outer use

	void write(string file);
	static Dict* read(string file);

	Dict(string file);
	Dict(int remove,int stat=1,int dist=CONS_distance_max);
	~Dict(){
		delete maps;	//leak some memory, but that's alright
	}
	void construct_dictionary(vector<DependencyInstance*>*);
};


#endif /* DICT_H_ */
