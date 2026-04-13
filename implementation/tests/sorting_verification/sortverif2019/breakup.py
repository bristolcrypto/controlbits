import sys
from pyparsing import StringEnd,Literal,Word,ZeroOrMore,OneOrMore,Optional,Forward,alphas,nums


n = int(sys.argv[1])
inputname = tuple('in_%d_32' % i for i in range(n))
outputname = tuple('out%d' % i for i in range(n))


def group(s):
  def t(x):
    x = list(x)
    if len(x) == 1: return x
    return [[s] + x]
  return t

lparen = Literal('(').suppress()
rparen = Literal(')').suppress()
comma = Literal(',').suppress()
equal = Literal('=').suppress()
name = Word(alphas,alphas+nums+"_")
assignsignedmin = (name + equal + Literal('signedmin').suppress()
               + lparen + name + comma + name + rparen
            ).set_parse_action(group('signedmin'))
assignsignedmax = (name + equal + Literal('signedmax').suppress()
               + lparen + name + comma + name + rparen
            ).set_parse_action(group('signedmax'))
assigncopy = (name + equal + name).set_parse_action(group('copy'))
assignment = assignsignedmin | assignsignedmax | assigncopy
assignments = ZeroOrMore(assignment) + StringEnd()

program = sys.stdin.read()
program = assignments.parse_string(program)
program = list(program)


if n < 2:
  assigned = set()
  for i in range(n): assigned.add(inputname[i])
  for p in program:
    if p[0] == 'copy':
      if not p[2] in assigned: raise Exception('%s used before assigned' % p[2])
      if p[1] in assigned: raise Exception('%s assigned twice' % p[1])
      assigned.add(p[1])
    else:
      raise Exception('only copies allowed for n<2')

  for i in range(n):
    if not outputname[i] in assigned:
      raise Exception('output %s not assigned' % outputname[i])

  sys.exit(0)
      
def find_disjoint_programs():
  # Collect disjoint sorting networks by following the disjoint groups of arguments in the program.

  disjoint_networks = [set(line[1:]) for line in program]

  still_merging = True
  while still_merging:
    still_merging = False
    for dji in disjoint_networks:
      disjoint_networks.remove(dji)
      for djj in disjoint_networks:
        if dji & djj:
          still_merging = True
          dji |= djj
          disjoint_networks.remove(djj)
      disjoint_networks.append(dji)
  
  # Ensure the disjoint sorting networks form an interlaced sorting network. The disjoint networks must
  # cover all inputs and outputs. For the iterative control bits algorithm to work, the input and output
  # indices of the 2^w disjoint networks must be of the form {x | 0 \leq x < n  &&  x \equiv k \mod 2^w}
  # for some k.
  two_to_the_w = len(disjoint_networks)
  seen_moduli = []
  for network in disjoint_networks:
    read_indices  = [int(elem[3:-3]) for elem in network if 'in_' in elem]
    write_indices = [int(elem[3:])   for elem in network if 'out' in elem]

    k = read_indices[0] % two_to_the_w
    read_indices.sort()
    write_indices.sort()
    assert read_indices == write_indices
    assert read_indices == list(range(k, n, two_to_the_w))

    seen_moduli.append(k)

  # Ensure we see all possible k.
  assert sorted(seen_moduli) == list(range(0, two_to_the_w))
  
  # Break up the sorting program into its disjoint parts.
  disjoint_programs = []
  for network in disjoint_networks:
    disjoint_program = [line for line in program if set(line[1:]) & network]
    disjoint_programs.append(disjoint_program)


  # For easy compatibility with the API of djbsort's verifier, 
  # we swap out the `in_0_32`, `in_2_32`, ... with `in_0_32`, `in_1_32`, ...
  # Therefore, we can simply hand our disjoint programs to "decompose.py" without
  # extra faff.
  for disjoint_program in disjoint_programs:
    for line in disjoint_program:
      for i, term in enumerate(line):
        if 'in_' in term:
          idx = int(term[3:-3])
          line[i] = "in_" + str(idx//two_to_the_w) + "_32"

        if 'out' in term:
          idx = int(term[3:])
          line[i] = "out" + str(idx//two_to_the_w)
    
  return disjoint_programs


dps = find_disjoint_programs()
num_disjoint_networks = len(dps)
size_of_each_network  = n // len(dps)


for i, sub_program in enumerate(dps):
  programstring = ""
  for line in sub_program:
    if line[0] == 'copy':
      programstring += f"{line[1]} = {line[2]}\n"
    elif line[0] == 'signedmin':
      programstring += f"{line[1]} = signedmin({line[2]}, {line[3]})\n"
    elif line[0] == 'signedmax':
      programstring += f"{line[1]} = signedmax({line[2]}, {line[3]})\n"

  with open(f"./tmp/disjsortprog_{n}_{num_disjoint_networks}_{i}", "w") as f:
    f.write(programstring)