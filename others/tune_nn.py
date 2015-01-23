import control_grid
from os import system,popen
import random
import re

class nn_task(control_grid.specific_task):
    def execute(self,iden):
        iden_all = dict()
        for i in iden:
            s = (i.split(":"))[0]
            iden_all[s] = (i.split(":"))[1]
        #the dir
        system("mkdir ../t_%s" % hash(iden))
        #write conf file
        conf_f = open("../t_%s/conf" % hash(iden),"w")
        template_f = open(control_grid.file_template,"r");
        for line in template_f:
            x = line.split()
            if(len(x)>0):
                if x[0] in iden_all.keys():
                    conf_f.write("%s %s\n"%(x[0],iden_all[x[0]]))
                else:
                    conf_f.write(line)
        conf_f.close()
        template_f.close()
        #execute 
        system("cd ../t_%s;../ztrain conf >> log.test" % hash(iden))
        #system("cd ../t_%s;echo zzzzz %s %s %s %s %s %s > log.test;sleep %s" % (hash(iden),1,1,1,1,1,random.random(),random.randint(5,10))) #onlyfortesing
        test_iter = 6  #iter6, which is the 7th iter
        log_file = "../t_%s/log.test" % hash(iden);
        #ret = popen("cat %s | grep '^zzzzz'" % log_file).read()
        #res = float((ret.split())[1+test_iter])
        
        ##new test
        pattern_iter = re.compile(r"Start training for iter %s" % test_iter)
        pattern_res = re.compile(r"Unlabeled Accuracy No Punc:[ \t]*([0-9]*.[0-9]*)")
        res = -1
        ff = open(log_file,"r")
        in_iter = False
        for line in ff:
            if(in_iter):
                x_res=pattern_res.search(line)
                if(x_res):
                    res = (float)(x_res.group(1))
                    break
            else:
                if(pattern_iter.search(line)):
                    in_iter = True                    
        ff.close()
        print("Finish one of %s with %g" % (iden,res))
        return res
        	
    

if __name__ == "__main__":
    control_grid.ct_init()
    control_grid.ct_main(nn_task())
