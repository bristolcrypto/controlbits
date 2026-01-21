from os import listdir, getcwd, path, remove
import subprocess

portable_sorting_algorithms = ["portable4", "bitonic"]
iterative_algorithms = ["cbiterative", "cbiteravx", "cbiterative_unrolled"]
recursive_algorithms = ["cbrecursion"]

# # Change -mtune to match your architecture.
portable_no_optimisations = ["gcc -Wall -fPIC -fwrapv".split(),
                             "clang -Wall -fPIC -fwrapv -Qunused-arguments".split()]

libmce_portable = ["gcc -Wall -fPIC -fwrapv -O2".split(),
                   "clang -Wall -fPIC -fwrapv -Qunused-arguments -O2".split()]

libmce_avx = ["gcc -Wall -fPIC -fwrapv -O2 -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mbmi -mbmi2 -mpopcnt -mavx2 -mtune=meteorlake".split(),
              "clang -Wall -fPIC -fwrapv -Qunused-arguments -O2 -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mbmi -mbmi2 -mpopcnt -mavx2 -mtune=meteorlake".split()]


def readfile(filename):
  with open(filename,'r') as f:
    return f.read()

def writefile(filename, string):
  with open(filename,'w') as f:
    f.write(string)

def print_quartiles(timing_data: list[int]):
  rounds = len(timing_data)
  return f"Q1: {timing_data[rounds//4]}, Q2: {timing_data[rounds//2]}, Q3: {timing_data[3*rounds//4]}, IQR: {timing_data[3*rounds//4] - timing_data[rounds//4]}, IQR/Q2: {(timing_data[3*rounds//4] - timing_data[rounds//4]) / timing_data[rounds//2]}"

def get_quartiles(timing_data: list[int]):
  """ Returns Q1, Q2, Q3, IQR, IQR/Q2"""
  rounds = len(timing_data)
  return timing_data[rounds//4], timing_data[rounds//2], timing_data[3*rounds//4], timing_data[3*rounds//4] - timing_data[rounds//4], (timing_data[3*rounds//4] - timing_data[rounds//4]) / timing_data[rounds//2]

def get_median(timing_data: list[int]):
  rounds = len(timing_data)
  return timing_data[rounds//2]



def compile_and_run(core, compiler_cmd: list[str], test_to_run: str, cb_alg: str, sort_alg: str, sort_type: str, wd: str, m: int, rounds: int) -> tuple[str,list]:
  """This function compiles a single combination choice of control bit and sorting algorithm with
  the compiler command chosen in the working directory wd. It returns a list of the form: 
                    ["compiler_cbalg_sortalg", [ timing data ]]
  """
  name = f"{compiler_cmd[0]}_{cb_alg}_{sort_alg}"
  compiler_cmd += ["int8_optblocker.o", "int16_optblocker.o", "int32_optblocker.o", test_to_run, "-o", name]
  # Set the director.h file.
  writefile("./director.h", f"""
#define CHOOSE_{sort_type.upper()}
#define CHOOSE_{cb_alg.upper()}
#define CHOOSE_{sort_alg.upper()}
#define ROUNDS {rounds}
#define M {m}
#define N {1 << m}
""")

  # Compile
  compiler_process = subprocess.Popen(compiler_cmd, cwd=wd, stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
  out, err = compiler_process.communicate()
  if err or b'error' in out:
    print(f"Compilation unsuccessful of: {' '.join(compiler_cmd)}")
    raise ChildProcessError
  
  # Run
  exe_process = subprocess.Popen(["taskset", "--cpu-list", f"{core}", f"./{name}"], cwd=wd, stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
  out, err = exe_process.communicate()
  if err or b'Aborted' in out:
    print(f"Error running binary ./{name}.\n Cmd: {' '.join(compiler_cmd)}")
    raise ChildProcessError
  remove(f"./{name}")

  # Upon success, the process prints a sorted list of TSC counts.
  result = out.decode().splitlines()[0]
  result = [int(time_count) for time_count in out.split()]

  return (name, result)




def try_get_core_without_simultaneous_multithreading():
  
  cpuinfo = readfile("/proc/cpuinfo")
  # Get all the cores.
  cores = []
  for line in cpuinfo.splitlines():
    # Get CPU id
    if "processor" in line:
      cores.append(line.split(':')[1].strip())


  # Check CPUs isn't doing simultaneous multithreading
  for core in cores:
    thread_pairs = readfile(f"/sys/devices/system/cpu/cpu{core}/topology/thread_siblings_list").strip()

    # If the core has no sibling thread
    if core == thread_pairs:
      return core
  
  # If none of the cores are single threaded
  return cores[0]


def check_no_turbo():
  turbo = readfile("/sys/devices/system/cpu/intel_pstate/no_turbo").strip()
  if turbo == "0":
    print(""" === WARNING === Benchmarks run with no_turbo = 0. Expect unreproducible results. 
  Before running the benchmarks run the following command as sudo:
    echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo
  
          """)
    return False
  else:
    return True
  

def check_underclocking_disabled(core):
  underclocking = readfile(f"/sys/devices/system/cpu/cpu{core}/cpufreq/scaling_governor").strip()
  if underclocking != "performance":
    print(f""" === WARNING === Benchmarks run with underclocking enabled on core {core}. Expect unreproducible results.
  Before running the benchmarks run the following command as sudo:
    echo performance > /sys/devices/system/cpu/cpu{core}/cpufreq/scaling_governor

          """)
    return False
  else:
    return True
  
def check_core_isnt_simultaneous_multithreading(core):
  thread_pairs = readfile(f"/sys/devices/system/cpu/cpu{core}/topology/thread_siblings_list").strip()
  if core != thread_pairs:
    print(""" === WARNING === Benchmarks run on core with simultaneous multithreading. Expect unreproducible results.""")
    return False
  else:
    print(f"Our benchmark scripts will use core {core} to avoid simultaneous multithreading.")
    return True
  

def set_up_cryptoint_optblockers(working_directory) :
  compiler_commands = [["gcc", "-fwrapv", "-c", "../src/cryptoint/int8_optblocker.c"],
                       ["gcc", "-fwrapv", "-c", "../src/cryptoint/int16_optblocker.c"],
                       ["gcc", "-fwrapv", "-c", "../src/cryptoint/int32_optblocker.c"]]
  for compiler_cmd in compiler_commands:
    compiler_process = subprocess.Popen(compiler_cmd[::], cwd=working_directory, stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    out, err = compiler_process.communicate()
    if err or b'error' in out:
      print(f"Compilation unsuccessful of: {' '.join(compiler_cmd)}")
      raise ChildProcessError
  
def clean_up_cryptoint_optblockers(working_directory):
  remove(f"./int8_optblocker.o")
  remove(f"./int16_optblocker.o")
  remove(f"./int32_optblocker.o")