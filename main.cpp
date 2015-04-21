/*
 * main.cpp
 *
 *  Created on: Jan 2, 2015
 *      Author: zzs
 */

//main one
#include <cstdlib>
#include "process_graph/Process.h"
#include "process_graph/Method1_allC.h"
#include "process_graph/Method2_pairs.h"
#include "process_graph/Method6_O2sib.h"
#include "process_graph/Method7_O2sibAll.h"
#include "process_graph/Method8_O2g.h"
#include "parts/Parameters.h"

//usage: command conf_file ...

int main(int argc,char **argv)
{
	if(argc < 2){
		Error("Not enough parameters for cmd.");
	}
	string conf(argv[1]);
	parsing_conf par(conf);
	srand(par.CONF_random_seed);
	Process *x;
	switch(par.CONF_method){
	case 1:
		x = new Method1_allC(conf);
		break;
	case 2:
		x = new Method2_pairs(conf);
	case 6:
		x = new Method6_O2sib(conf);
		break;
	case 7:
		x = new Method7_O2sibAll(conf);
		break;
	case 8:
		x = new Method8_O2g(conf);
		break;
	default:
		x = 0;
		break;
	}
	if(argc == 2){
		//training
		x->train();
		if(par.CONF_test_file.length()>0 && par.CONF_gold_file.length()>0){
			//test
			string mach_best_name = par.CONF_mach_name+par.CONF_mach_best_suffix;
			x->test(mach_best_name);
		}
	}
	else if(argc == 3){
		//only testing
		x->test(string(argv[2]));
	}
	else if(argc == 4){
		x->check_o1_filter(string(argv[2]),string(argv[3]));
	}
	return 0;
}


