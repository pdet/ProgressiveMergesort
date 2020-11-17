import os
import inspect
import urllib

# script directory
SCRIPT_PATH = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
os.chdir(SCRIPT_PATH)

# clean experiment folder
if (os.path.isdir("results")):
    os.system("rm -r -f results")

os.system("mkdir results")

def compile():
    print("Compiling")
    os.environ['OPT'] = 'true'
    if os.system('cmake -DCMAKE_BUILD_TYPE=Release && make') != 0:
        print("Make Failed")
        exit()

compile()

os.chdir(os.path.join(SCRIPT_PATH,"results"))

class Experiment:
    num_queries = 10000
    column_size = 1000000
    append_size = 10
    append_frequency = 10
    start_updates_after = 1000
    delta = 0.1;
    file_name = ''

    def __init__(self, num_queries, column_size,append_size,append_frequency,start_updates_after,delta):
        self.num_queries = num_queries
        self.column_size = column_size
        self.append_size = append_size
        self.append_frequency = append_frequency
        self.start_updates_after = start_updates_after
        self.delta = delta
        self.file_name = str(num_queries) + '_' + str(column_size) + '_' + str(append_size) + '_' + str(append_frequency) + '_' + str(start_updates_after) + '_' + str(delta)

    def run_process(self,codestr):
        import subprocess
        p = subprocess.Popen(codestr.split(' '), stdout=subprocess.PIPE)
        stdout,stderr = p.communicate()
        return stdout

    def run_one(self,index_type,update_type):
        codestr = "../main --num-queries="+str(self.num_queries) + " --column-size=" + str(self.column_size) + " --index-type=" + str(index_type) + \
        " --update-type=" + str(update_type) + " --delta=" + str(self.delta) + " --append-size=" + str(self.append_size) + " --append-frequency=" + \
        str(self.append_frequency) + " --updates-after=" + str(self.start_updates_after) 
        print (codestr)
        result = self.run_process(codestr)
        f = open(self.file_name+"_" + str(index_type)+"_" + str(update_type) + ".csv","wb")
        f.write(result)
        f.close()

    def run(self):
        self.run_one(1,1) # Cracking Complete
        self.run_one(1,2) # Cracking Gradual
        self.run_one(1,3) # Cracking Ripple
        self.run_one(2,4) # Progressive Mergesort


data_scale = [1,10,100]

for scale in data_scale:

    exp = Experiment(10000,10000000 * scale,10* scale,10,1000,0.1)
    exp.run()

    exp = Experiment(10000,10000000* scale,100* scale,100,1000,0.1)
    exp.run()

    exp = Experiment(10000,10000000* scale,1000* scale,1000,1000,0.1)
    exp.run()

    exp = Experiment(10000,10000000* scale,100* scale,10,1000,0.1)
    exp.run()

    exp = Experiment(10000,10000000* scale,1000* scale,100,1000,0.1)
    exp.run()

    exp = Experiment(10000,10000000* scale,10000* scale,1000,1000,0.1)
    exp.run()
