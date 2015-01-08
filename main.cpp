/*
 * main.cpp
 *
 *  Created on: Jan 2, 2015
 *      Author: zzs
 */

//main one
#include "process/Process.h"
#include "process/Method1_allC.h"
#include "process/Method2_pairs.h"

//usage: command conf_file ...

int main(int argc,char **argv)
{
	using namespace parsing_conf;
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


