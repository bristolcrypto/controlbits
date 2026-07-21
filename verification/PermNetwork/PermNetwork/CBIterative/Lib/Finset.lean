import PermNetwork.CBIterative.Lib.Nat
import Mathlib.Data.Fintype.EquivFin

/-!
# Numbers agreeing on their low bits

`bitMatchUnder i x` is the set of numbers below `2 ^ n` that agree with `x` on the low `i` bits.
There are exactly `2 ^ (n - i)` of them (`card_bitMatchUnder`), witnessed by the explicit
equivalence `equivBitMatchUnder` with `Fin (2 ^ (n - i))`. These "bit-matching" classes index the
sub-blocks on which one layer of the control-bit network acts.
-/

namespace Finset

open Nat

/-- The numbers below `2 ^ n` whose low `i` bits agree with those of `x`. -/
def bitMatchUnder {n : ℕ} (i : ℕ) (x : Fin (2 ^ n)) :=
  (Finset.range (2 ^ n)).filter (fun y => ∀ k < i, y.testBit k = (x : ℕ).testBit k)

section BitMatchUnder

variable {n : ℕ} {x : Fin (2 ^ n)} {i : ℕ}

@[simp]
theorem mem_bitMatchUnder_iff {q : ℕ} :
    q ∈ bitMatchUnder i x ↔ q < 2 ^ n ∧ ∀ k < i, q.testBit k = (x : ℕ).testBit k := by
  unfold bitMatchUnder
  simp_rw [Finset.mem_filter, Finset.mem_range]

/-- `bitMatchUnder i x` is in bijection with `Fin (2 ^ (n - i))`: an element is determined by the
`n - i` bits above position `i`, since the low `i` bits are fixed to match `x`. -/
def equivBitMatchUnder {n i : ℕ} {x : Fin (2 ^ n)} :
    bitMatchUnder i x ≃ Fin (2 ^ (n - i)) where
  toFun a := ⟨(a.1 / 2 ^ i) % 2 ^ (n - i), mod_lt _ (Nat.two_pow_pos _)⟩
  invFun b := ⟨2 ^ i * b.1 + x.1 % 2 ^ i, Finset.mem_filter.mpr
    ⟨Finset.mem_range.mpr (by
    rcases le_or_gt i n with hin | hin
    · refine (?_ : 2 ^ i * ↑b + 2 ^ i ≤ _).trans_lt' (by grind)
      refine (Nat.add_le_add_right (mul_le_mul_left _ (le_sub_one_of_lt b.isLt)) _).trans
        (?_ : _ = _).le; rw [Nat.mul_sub, ← Nat.pow_add]; grind
    · have H := mod_lt_of_lt x.isLt (b := 2 ^ i); grind),
    by grind [Nat.two_pow_pos]⟩⟩
  left_inv  := fun ⟨a, ha⟩ => Subtype.ext <| testBit_ext <| by
    rw [mem_bitMatchUnder_iff] at ha
    have H : ∀ j < i, a.testBit j = x.1.testBit j := ha.2
    have H' : ∀ j ≥ n, a.testBit j = false :=
      fun j hj => testBit_eq_false_of_lt (ha.1.trans_le (by grind))
    grind [Nat.two_pow_pos]
  right_inv b := Fin.ext <| by
    simp_rw [Nat.mul_add_div (Nat.two_pow_pos _), Nat.mod_div_self,
      add_zero, Nat.mod_eq_of_lt b.isLt]

theorem card_bitMatchUnder (i : ℕ) (x : Fin (2 ^ n)) :
    (bitMatchUnder i x).card = 2 ^ (n - i) :=
  (Finset.card_eq_of_equiv_fintype equivBitMatchUnder).trans (Fintype.card_fin (2 ^ (n - i)))

end BitMatchUnder

end Finset
