/*
 * CslmInterface.cpp
 *
 *  Created on: 2015Äê3ÔÂ31ÈÕ
 *      Author: zzs
 */

#include "CslmInterface.h"

REAL* CslmInterface::mach_forward(REAL* assign,int all)
{
	Mach* m = mach;
	int idim = m->GetIdim();
	int odim = m->GetOdim();
	int remain = all;
	int bsize = m->GetBsize();
	REAL* xx = assign;
	REAL* mach_y = new REAL[all*odim];
	REAL* yy = mach_y;
	while(remain > 0){
		int n=0;
		if(remain >= bsize)
			n = bsize;
		else
			n = remain;
		remain -= bsize;
		m->SetDataIn(xx);
		m->Forw(n);
		memcpy(yy, m->GetDataOut(), odim*sizeof(REAL)*n);
		yy += n*odim;
		xx += n*idim;
	}
	return mach_y;
}

const char* cslm_activations[] = {"Tanh","Nope"};
#define WRITE_CONF_ONE(a1,a2) \
	fout << cslm_activations[parameters->CONF_NN_act] << " = " << (int)(a1) << "x" << (int)(a2) << " fanio-init-weights=1.0\n";
static inline void write_conf_no_split(parsing_conf* parameters,int dict_count,int xdim,int outdim)
{
	//--first write conf
	ofstream fout(parameters->CONF_mach_conf_name.c_str());
	fout << "block-size = " << parameters->CONF_NN_BS << "\n";
	if(parameters->CONF_NN_drop>0)
		fout << "drop-out = " << parameters->CONF_NN_drop << "\n";
	int width = xdim*parameters->CONF_NN_we;
	//1.projection layer
	fout << "[machine]\n" << "Sequential = \n" << "Parallel = \n";
	for(int i=0;i<xdim;i++)
		fout << "Tab = " << dict_count << "x" << parameters->CONF_NN_we << "\n";
	fout << "#End\n";
	//2.hidden layers
	for(int i=0;i<parameters->CONF_NN_plus_layers;i++){
		WRITE_CONF_ONE(width,parameters->CONF_NN_h_size[i]);
		width = parameters->CONF_NN_h_size[i];
	}
	//3.output layer
	if(outdim>1){
		//output multiclass-class(0 or 1)
		fout << "Softmax = " << width << "x" << outdim << " fanio-init-weights=1.0\n";
	}
	else{
		//linear output score
		fout << "Linear = " << width << "x" << 1 << " fanio-init-weights=1.0\n";
	}
	fout << "#End\n";
	fout.close();
}
static inline void write_conf_split(parsing_conf* parameters,int dict_count,int xdim,int outdim,int split_num)
{
	//here only support CONF_NN_h_size and must have symmetric input
	ofstream fout(parameters->CONF_mach_conf_name.c_str());
	fout << "block-size = " << parameters->CONF_NN_BS << "\n";
	if(parameters->CONF_NN_drop>0)
		fout << "drop-out = " << parameters->CONF_NN_drop << "\n";
	int width = xdim*parameters->CONF_NN_we;
	//1.projection layer
	fout << "[machine]\n" << "Sequential = \n" << "Parallel = \n";
	for(int i=0;i<xdim;i++)
		fout << "Tab = " << dict_count << "x" << parameters->CONF_NN_we << "\n";
	fout << "#End\n";
	//2.then
	//hidden layer1 -- split
	{
		int each_1 = width / split_num;
		int each_2 = parameters->CONF_NN_h_size[0] / split_num;		//should be divided
		fout << "Parallel = \n";
		for(int i=0;i<split_num;i++)
			WRITE_CONF_ONE(each_1,each_2);
		fout << "#End\n";
	}
	//more hidden layers??
	for(int i=0;i<parameters->CONF_NN_plus_layers-1;i++){
		WRITE_CONF_ONE(parameters->CONF_NN_h_size[i],parameters->CONF_NN_h_size[i+1]);
	}
	width = parameters->CONF_NN_h_size[parameters->CONF_NN_plus_layers-1];
	//3.output
	if(outdim>1)
		fout << "Softmax = " << width << "x" << outdim << " fanio-init-weights=1.0\n";
	else
		fout << "Linear = " << width << "x" << 1 << " fanio-init-weights=1.0\n";
	fout << "#End\n";
	fout.close();
}
void CslmInterface::mach_split_share()
{
	//specified sharing first layer(after projection)
	//	--- don't care memory problems because we never delete(fortunately memory is enough and deleting is at the last...)
	MachMulti* m = (MachMulti*)mach;	//seq
	m = (MachMulti*)(m->MachGet(1));	//par2
	MachLin* mm = (MachLin*)(m->MachGet(0));	//lin
	REAL* ww = mm->w;
	REAL* bb = mm->b;
	for(int i=1;i<m->MachGetNb();i++){
		((MachLin*)(m->MachGet(i)))->w = ww;
		((MachLin*)(m->MachGet(i)))->b = bb;
	}
}

//specified init
CslmInterface* CslmInterface::create_one(parsing_conf* parameters,FeatureGen* f,int outdim)
{
	//1.conf file
	if(parameters->CONF_NN_split)
		write_conf_split(parameters,f->get_dict()->get_count(),f->get_xdim(),outdim,f->get_order()+1);
	else
		write_conf_no_split(parameters,f->get_dict()->get_count(),f->get_xdim(),outdim);
	//2. get machine
	MachConfig mach_config(true);
	//for mach_config
	char *argv[2];
	argv[0] = "nn";
	argv[1] = (char*)parameters->CONF_mach_conf_name.c_str();
	mach_config.parse_options(2,argv);
    Mach* mach = mach_config.get_machine();
    if(mach == 0)
    	Error(mach_config.get_error_string().c_str());
    CslmInterface* ret = new CslmInterface(mach);
    //3.split??
    if(parameters->CONF_NN_split && parameters->CONF_NN_split_share)
    	ret->mach_split_share();
    return ret;
}
