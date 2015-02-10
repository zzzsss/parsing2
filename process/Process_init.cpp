/*
 * Process_init.cpp
 *
 *  Created on: 10 Feb, 2015
 *      Author: z
 */

#include "Process.h"
#include "../cslm/MachTab.h"
#include "../cslm/Mach.h"
#include "../cslm/MachMulti.h"
#include "HashMap.h"
#include <fstream>
#include <cstring>

void Process::init_embed()
{
	vector<string*>* real_wl = dict->get_real_words();
	//special mach structures
	MachMulti* m = (MachMulti*)mach;
	m = (MachMulti*)(m->MachGet(0));
	MachTab* mm = (MachTab*)(m->MachGet(0));
	REAL* to_assign = mm->GetTabAdr();

	//temps
	HashMap* t_maps = new HashMap(INIT_EM_MAX_SIZE);
	REAL * t_embeds = new REAL[CONF_NN_we*INIT_EM_MAX_SIZE];
	vector<string*>* t_words = new vector<string*>();

	cout << "--Init embedding from " << CONF_NN_WL << " and " << CONF_NN_EM << "\n";
	ifstream fwl,fem;
	fwl.open(CONF_NN_WL.c_str());
	fem.open(CONF_NN_EM.c_str());
	REAL* em_assign = t_embeds;
	int em_assign_num = 0;
	if(!fwl || !fem)
		Error("Failed when opening embedding file.");
	while(fwl){
		if(!fem)
			Error("No match with embedding files.");
		string one_word;
		fwl >> one_word;
		for(int i=0;i<CONF_NN_we;i++,em_assign++){
			fem >> *em_assign;
			*em_assign = *em_assign * CONF_NN_ISCALE;
		}
		string* one_word_p = new string(one_word);
		t_maps->insert(pair<string*, int>(one_word_p,em_assign_num++));
		t_words->push_back(one_word_p);
	}
	fwl.close();
	fem.close();

	//start
	int n_all = real_wl->size();
	int n_check = 0;
	int n_icheck = 0;
	for(int i=0;i<n_all;i++){
		string* one_str = real_wl->at(i);
		int one_index = dict->get_word_index(one_str);	//must be there
		//serach here
		HashMap::iterator iter = t_maps->find(one_str);
		if(iter == t_maps->end()){
			//tolower
			string one_str_temp = (*one_str);
			for(int j=0;j<one_str_temp.size();j++)
				one_str_temp[j] = tolower(one_str_temp[j]);
			//find again
			iter = t_maps->find(&one_str_temp);
			if(iter != t_maps->end())
				n_icheck++;
		}
		else
			n_check++;

		if(iter != t_maps->end()){
			//init
			int the_index = iter->second;
			memcpy(to_assign+one_index*CONF_NN_we,t_embeds+the_index*CONF_NN_we,CONF_NN_we*sizeof(REAL));
		}
	}

	//clear
	delete [] t_embeds;
	delete t_maps;
	for(int i=0;i<t_words->size();i++)
		delete t_words->at(i);
	delete t_words;
	cout << "-- Done, with " << n_all << "/" << n_check << "/" << n_icheck << '\n';
}


