import PermNetwork.CBIterative.Controlbits
import PermNetwork.CBRecursive.Controlbits
import PermNetwork.ComparatorNetwork.Basic

/-!
# PermNetwork

Root module of the library. It gathers the three independent developments described in the project
README:

* `CBRecursive` — a formalisation of Bernstein's recursive control-bit formulae, extended with a
  proof that interpreting control bits as a permutation is a left inverse to computing them;
* `CBIterative` — the Robson–Kelly iterative variant of the control-bit formulae;
* `ComparatorNetwork` — an early model of comparator (sorting) networks and their behaviour under
  permutation of channels.

Importing this file pulls in all three.
-/
