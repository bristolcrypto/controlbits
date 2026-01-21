""" This script collects the data for Table 1 where all algorithms
    are compiled with -O2 flags.
"""

from os import getcwd
import helpers

# Parameters to choose 
m = 13
n = (1 << m)
rounds = 63
core = helpers.try_get_core_without_simultaneous_multithreading()

cwd = getcwd()
checkfor = "controlbits/implementation/benchmarks"
if cwd[-len(checkfor):] != checkfor:
  print("Error. You must run this script from the benchmarks directory.")
  quit()

if any([not helpers.check_no_turbo(), not helpers.check_underclocking_disabled(core), not helpers.check_core_isnt_simultaneous_multithreading(core)]):
  carry_on = input("Continue? y/n \n")
  if 'y' not in carry_on.lower():
    quit()

# These are the 'portable' compilers from libmceliece
compilers = ["gcc -Wall -fPIC -fwrapv -O2".split(),
             "clang -Wall -fPIC -fwrapv -Qunused-arguments -O2".split()]

tests_parameters = [("cbrecursion", "portable4", "standard_sorting"  ),     # Row 1
                    ("cbiterative", "portable4", "standard_sorting"  ),     # Row 1
                    ("cbiterative", "portable4", "interlaced_sorting")]     # Row 2

helpers.set_up_cryptoint_optblockers(cwd)
results = []
for cb_alg, sort_alg, sort_type in tests_parameters:
  temp = []
  for compiler_cmd in compilers:
    print(f"Getting results for ({cb_alg}, {sort_alg}, {sort_type}) compiled with {compiler_cmd[0]}.")
    temp.append(helpers.compile_and_run(core, compiler_cmd[::], "calculate_cbs.c", cb_alg, sort_alg, sort_type, cwd, m, rounds))
  
  # Store minimum over gcc and clang
  results.append(min(temp, key= lambda result: helpers.get_median(result[1])))


helpers.clean_up_cryptoint_optblockers(cwd)


print("\nPrinting raw results: ")
for result in results:
  print("  ", result[0], helpers.print_quartiles(result[1]))
print("\n")


# We compare to cbrecursion with portable4
comparison_point = helpers.get_median(results[0][1]) 
clockcycles = [helpers.get_median(result[1]) for result in results[0:3]]
comparisons = [round(clockcycles / comparison_point, 2) for clockcycles in clockcycles]


print(f"""\\begin{{table}}[h]
  \\centering
  \\caption{{\\textbf{{Performance of portable implementations.}} Latency (in median clock cycles) required to calculate control bits for a permutation of $2^{{{m}}}$ integers, taken over $n={rounds}$ samples, with an acceleration factor relative to \\cbrecursion{{}} with \\portable{{}}. Both are compiled with \\texttt{{-O2}} optimisations.}}\\label{{tab:portable_results}}
  \\begin{{tabular}}{{|c|rr|rr|}}\\hline
    Sorting         & \\multicolumn{{4}}{{c|}}{{Control Bit Implementation}} \\\\\\cline{{2-5}}
    Implementation  & \\multicolumn{{2}}{{c|}}{{\\cbrecursion{{}}}} & \\multicolumn{{2}}{{c|}}{{\\cbiterative{{}}}} \\\\\\hline
    \\portable{{}}  & {clockcycles[0]}   & ${comparisons[0]:.2f}\\times$ & {clockcycles[1]} & ${comparisons[1]:.2f}\\times$  \\\\\\cline{{2-5}}
    \\iportable{{}} &         \\multicolumn{{2}}{{c|}}{{N/A}}            & {clockcycles[2]} & ${comparisons[2]:.2f}\\times$  \\\\\\hline
  \\end{{tabular}}
\\end{{table}}
""")
