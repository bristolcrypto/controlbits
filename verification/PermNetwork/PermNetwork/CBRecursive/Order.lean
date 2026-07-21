import Mathlib.Order.SuccPred.Basic

/-!
# Order lemmas for successor/predecessor orders

Supplementary lemmas about `SuccOrder`/`PredOrder`. The main result,
`eq_false_true_of_cond_succ_lt_of_cond_succ_lt`, analyses a pair of strict inequalities where each
side is optionally bumped by a successor depending on a `Bool`, and pins down the only way both can
hold at once. It is used to reason about the boundary behaviour of the bit-flip maps.
-/

namespace Order

theorem lt_pred_of_lt_of_lt {α : Type*} [Preorder α] [PredOrder α]
{a b c : α} (hab : a < b) (hbc : b < c) : a < pred c := lt_of_lt_of_le hab (le_pred_of_lt hbc)

theorem succ_lt_of_lt_of_lt {α : Type*} [Preorder α] [SuccOrder α]
{a b c : α} (hab : a < b) (hbc : b < c) : succ a < c := lt_of_le_of_lt (succ_le_of_lt hab) hbc

lemma eq_false_true_of_cond_succ_lt_of_cond_succ_lt {α : Type*} [LinearOrder α] [SuccOrder α]
  [NoMaxOrder α] {m n : α} {bm bn : Bool}
(hmn : (bif bm then succ else id) m < (bif bn then succ else id) n)
(hnm : (bif bn then id else succ) n < (bif bm then id else succ) m) :
bm = false ∧ bn = true ∧ (m = n) := by
  cases bm <;> cases bn <;>
  simp only [true_and, cond_true, cond_false, id_eq,
  succ_le_iff, lt_succ_iff] at *
  · exact (hmn.not_ge hnm.le).elim
  · exact le_antisymm hmn hnm
  · exact (lt_irrefl _ (((hnm.trans (lt_succ _)).trans hmn).trans (lt_succ _))).elim
  · exact (hmn.not_gt hnm).elim

end Order
