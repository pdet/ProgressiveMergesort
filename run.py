import os
import inspect
import urllib

# script directory
SCRIPT_PATH = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
os.chdir(SCRIPT_PATH)


# Compilation
def clean_generated_data():
    os.system('rm -r generated_data')


def compile():
    print("Compiling")
    os.environ['OPT'] = 'true'
    if os.system('cmake -DCMAKE_BUILD_TYPE=Release && make') != 0:
        print("Make Failed")
        exit()

def run_process(codestr):
    import subprocess
    p = subprocess.Popen(codestr.split(' '), stdout=subprocess.PIPE)
    stdout,stderr = p.communicate()
    return stdout


compile()

os.system("rm prog_idx.csv")
prog_idx = run_process("./main")
f = open("prog_idx.csv", "wb")
f.write(prog_idx)
f.close()