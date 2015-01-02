/*
 * main.cpp
 *
 *  Created on: Jan 2, 2015
 *      Author: zzs
 */

//main one
#include "process/Process.h"
#include "process/Method1_allC.h"

//usage: command conf_file ...

int main(int argc,char **argv)
{
	using namespace parsing_conf;
	if(argc < 2){
		Error("Not enough parameters for cmd.");
	}
	init_configurations(string(argv[1]));
	Process *x;
	if(argc == 2){
		//training
		switch(CONF_method){
		case 1:
			x = new Method1_allC();
			x->train();
			break;
		}
	}
	else{

	}
	return 0;
}


