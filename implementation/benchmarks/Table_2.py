""" This script collects the data for table 2 where all algorithms
    are compiled with -O2 -mavx2 flags. Please set the following variable
    to match your architecture."""


microarch = "meteorlake"


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

# Compiler options copied from libmceliece x86 and set to -mtune=meteorlake
compilers = [f"gcc -Wall -fPIC -fwrapv -O2 -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mbmi -mbmi2 -mpopcnt -mavx2 -mtune={microarch}".split(),
             f"clang -Wall -fPIC -fwrapv -Qunused-arguments -O2 -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mbmi -mbmi2 -mpopcnt -mavx2 -mtune={microarch}".split()]

tests_parameters = [("cbrecursion", "ibitonic"   , "standard_sorting"  ),  # -+-
                    ("cbiterative", "ibitonic"   , "standard_sorting"  ),  #  |   row 1
                    ("cbiteravx"  , "ibitonic"   , "standard_sorting"  ),  # -+-
                    ("cbrecursion", "djbsort2026", "standard_sorting"  ),  # -+-
                    ("cbiterative", "djbsort2026", "standard_sorting"  ),  #  |   row 2
                    ("cbiteravx"  , "djbsort2026", "standard_sorting"  ),  # -+-
                                                                           # -+-
                    ("cbiterative", "ibitonic"   , "interlaced_sorting"),  #  |   row 3
                    ("cbiteravx"  , "ibitonic"   , "interlaced_sorting"),  # -+-  
                                                                           # -+- 
                    ("cbiterative", "ipermsort"  , "interlaced_sorting"),  #  |   row 4    
                    ("cbiteravx"  , "ipermsort"  , "interlaced_sorting")]  # -+-    

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

# We compare to cbrecursion with djbsort
comparison_point = helpers.get_median(results[3][1]) 

clocksrow1 = [helpers.get_median(result[1]) for result in results[ 0: 3]]
clocksrow2 = [helpers.get_median(result[1]) for result in results[ 3: 6]]
clocksrow3 = [helpers.get_median(result[1]) for result in results[ 6: 8]]
clocksrow4 = [helpers.get_median(result[1]) for result in results[ 8:10]]


compsrow1 = [round(clockcycles / comparison_point, 2) for clockcycles in clocksrow1]
compsrow2 = [round(clockcycles / comparison_point, 2) for clockcycles in clocksrow2]
compsrow3 = [round(clockcycles / comparison_point, 2) for clockcycles in clocksrow3]
compsrow4 = [round(clockcycles / comparison_point, 2) for clockcycles in clocksrow4]


print(f"""
\\begin{{table}}[h]
  \\centering
  \\caption{{\\textbf{{Performance of AVX2-enabled implementations.}} Latency (in median clock cycles) required to calculate control bits for a permutation of $2^{{{m}}}$ integers, taken over $n={rounds}$ samples, with an acceleration factor relative to \\cbrecursion{{}} with \\djbsort{{}}.  Both are compiled with \\texttt{{-O2}}, \\texttt{{-mavx2}}, \\texttt{{-mtune=meteorlake}} optimisations.}}\\label{{tab:avx_results}}
  \\begin{{tabular}}{{|c|rr|rr|rr|}}\\hline
    Sorting & \\multicolumn{{6}}{{c|}}{{Control Bit Implementation}} \\\\\\cline{{2-7}}
    Implementation & \\multicolumn{{2}}{{c|}}{{\\cbrecursion{{}}}} & \\multicolumn{{2}}{{c|}}{{\\cbiterative{{}}}} & \\multicolumn{{2}}{{c|}}{{\\cbiteravx{{}}}} \\\\\\hline
    \\bitonic{{}}    & {clocksrow1[0]} & ${compsrow1[0]:.2f}\\times$  & {clocksrow1[1]} & ${compsrow1[1]:.2f}\\times$ & {clocksrow1[2]} & ${compsrow1[2]:.2f}\\times$  \\\\\\cline{{2-7}}
    \\djbsort{{}}    & {clocksrow2[0]} & ${compsrow2[0]:.2f}\\times$  & {clocksrow2[1]} & ${compsrow2[1]:.2f}\\times$ & {clocksrow2[2]} & ${compsrow2[2]:.2f}\\times$  \\\\\\cline{{2-7}}
    \\ibitonic{{}}   & \\multicolumn{{2}}{{c|}}{{N/A}}                & {clocksrow3[0]} & ${compsrow3[0]:.2f}\\times$ & {clocksrow3[1]} & ${compsrow3[1]:.2f}\\times$  \\\\\\cline{{2-7}}
    \\permsort{{}}   & \\multicolumn{{2}}{{c|}}{{N/A}}                & {clocksrow4[0]} & ${compsrow4[0]:.2f}\\times$ & {clocksrow4[1]} & ${compsrow4[1]:.2f}\\times$  \\\\\\hline
    \\end{{tabular}}
\\end{{table}}
""")