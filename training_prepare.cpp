/*
 * training_prepare.cpp
 *
 *  Created on: Dec 19, 2014
 *      Author: zzs
 */
#include <iostream>
#include <vector>
#include <string>
#include "tools/CONLLReader.h"
#include "tools/DependencyInstance.h"
using namespace std;

//step0: get_input
vector<DependencyInstance*>* get_input(string file)
{
	vector<DependencyInstance*>* ret = new vector<DependencyInstance*>();
	CONLLReader* reader = new CONLLReader();
	reader->startReading(file.c_str());
	DependencyInstance* x = reader->getNext();
	while(x != NULL){
		ret->push_back(x);
		x = reader->getNext();
	}
	return ret;
}



