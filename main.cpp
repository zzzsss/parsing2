/*
 * main.cpp
 *
 *  Created on: Jan 2, 2015
 *      Author: zzs
 */

//main one
#include <cstdlib>
#include "process/Process.h"
#include "process/Method1_allC.h"
#include "process/Method2_pairs.h"
#include "process/Method3_online.h"
#include "process/Method4_random.h"
#include "process/Method5_localMax.h"
#include "process/Method6_O2sib.h"
#include "process/Method7_O2sibAll.h"

//usage: command conf_file ...

int main(int argc,char **argv)
{
	using namespace parsing_conf;
	srand(CONF_random_seed);
	if(argc < 2){
		Error("Not enough parameters for cmd.");
	}
	init_configurations(string(argv[1]));
	Process *x;
	switch(CONF_method){
	case 1:
		x = new Method1_allC();
		break;
	case 2:
		x = new Method2_pairs();
		break;
	case 3:
		x= new Method3_online();
		break;
	case 4:
		x = new Method4_random();
		break;
	case 5:
		x = new Method5_localMax();
		break;
	case 6:
		x = new Method6_O2sib();
		break;
	case 7:
		x = new Method7_O2sibAll();
		break;
	}
	if(argc == 2){
		//training
		x->train();
		if(CONF_test_file.length()>0 && CONF_gold_file.length()>0){
			//test
			Dict* temp_d = new Dict(CONF_dict_file);
			ifstream ifs;
			string mach_best_name = CONF_mach_name+CONF_mach_best_suffix;
			ifs.open(mach_best_name.c_str(),ios::binary);
			Mach* temp_m = Mach::Read(ifs);
			x->test(temp_m,temp_d);
		}
	}
	else{
		//only testing
		Dict* temp_d = new Dict(CONF_dict_file);
		ifstream ifs;
		string mach_best_name = string(argv[2]);	//test conf mach_name
		ifs.open(mach_best_name.c_str(),ios::binary);
		Mach* temp_m = Mach::Read(ifs);
		x->test(temp_m,temp_d);
	}
	return 0;
}


