/*
 * common.h
 *
 *  Created on: Dec 19, 2014
 *      Author: zzs
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <iostream>
#include <vector>
#include <string>
#include "tools/CONLLReader.h"
#include "tools/DependencyInstance.h"
#include "dict/Dict.h"
using namespace std;

//step0: get_input
vector<DependencyInstance*>* get_input(string file);



#endif /* COMMON_H_ */
