/*
 * main.cpp
 *
 *  Created on: Dec 19, 2014
 *      Author: zzs
 */

#include "common.h"

int main()
{
	Dict d(1);
	vector<DependencyInstance*>* data = get_input("./wsj_train.txt.08f");
	d.construct_dictionary(data);
}

