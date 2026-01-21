import Mathlib.Algebra.BigOperators.Fin
import Mathlib.Data.Fin.Tuple.Sort

namespace List

open Finset in
theorem countP_finRange {p : Fin n → Bool} : countP p (finRange n) = #{i : Fin n | p i} := by
  induction n with
  | zero => rfl
  | succ n IH =>
    simp_rw [finRange_succ, countP_cons, countP_map, IH, Function.comp_apply, Finset.card_filter,
      Fin.sum_univ_succ, add_comm]

open Finset in
theorem count_ofFn {f : Fin n → α} {a : α} [BEq α] :
    (ofFn f).count a = #{i : Fin n | f i == a} := by
  rw [List.ofFn_eq_map, count, countP_map, Function.comp_def, countP_finRange]

theorem ofFn_comp_perm_ofFn_iff {f : Fin n → α} {π : Equiv.Perm (Fin n)} :
    ofFn (f ∘ π) ~ ofFn f := by classical
  simp_rw [perm_iff_count, count_ofFn]
  exact fun _ => Finset.card_equiv π <| by grind

end List

namespace Prod

end Prod

abbrev Tuple (α : Sort u) (n : ℕ) := Fin n → α

namespace Tuple

open Equiv

section PermOn

@[specialize] def permOn (f : Tuple α n) (π : Perm (Fin n)) : Tuple α n := (f <| π ·)

@[grind =]
theorem permOn_apply {f : Tuple α n} {π : Perm (Fin n)} {k : Fin n} :
    f.permOn π k = f (π k) := rfl

@[simp, grind =]
theorem permOn_one {f : Tuple α n} : f.permOn 1 = f := rfl

@[simp, grind =]
theorem permOn_mul {f : Tuple α n} {π ρ : Perm (Fin n)} :
    f.permOn (π * ρ) = (f.permOn π).permOn ρ := rfl

open List in
theorem ofFn_permOn_perm_ofFn {f : Tuple α n} {π : Perm (Fin n)} :
    ofFn (f.permOn π) ~ ofFn f := ofFn_comp_perm_ofFn_iff

open List in
theorem ofFn_permOn_perm_ofFn_permOn {f : Tuple α n} {π ρ : Perm (Fin n)} :
    ofFn (f.permOn π) ~ ofFn (f.permOn ρ) := ofFn_permOn_perm_ofFn.trans ofFn_permOn_perm_ofFn.symm

@[simp, grind =]
theorem permOn_permOn_inv {f : Tuple α n} : (f.permOn π⁻¹).permOn π = f := by
  simp only [← permOn_mul, inv_mul_cancel, permOn_one]

@[simp, grind =]
theorem permOn_inv_permOn {f : Tuple α n} : (f.permOn π).permOn π⁻¹ = f := by
  simp only [← permOn_mul, mul_inv_cancel, permOn_one]

theorem permOn_injective (π : Perm (Fin n)) : (permOn (α := α) · π).Injective := fun f g => by
  simp_rw [funext_iff, permOn_apply]
  exact fun hfg x =>
    ((congrArg f (Equiv.apply_symm_apply π x).symm).trans (hfg (⇑π⁻¹ x))).trans
    (congrArg g (Equiv.apply_symm_apply π x))

@[simp]
theorem permOn_inj {f g : Tuple α n} {π : Perm (Fin n)} : f.permOn π = g.permOn π ↔ f = g :=
  (permOn_injective π).eq_iff

open List in
theorem permOn_eq_self_of_monotone [LinearOrder α] {f : Tuple α n}
    {π : Perm (Fin n)} (hfπ : Monotone <| f.permOn π) (hf : Monotone <| f) : f.permOn π = f := by
  refine ofFn_injective <|
    ofFn_permOn_perm_ofFn.eq_of_pairwise (fun _ _ _ _ => le_antisymm) ?_ ?_ <;>
  grind [pairwise_ofFn, monotone_iff_forall_lt]

open List in
theorem permOn_eq_of_monotone [LinearOrder α] {f : Tuple α n}
    {π ρ : Perm (Fin n)} (hπ : Monotone <| f.permOn π) (hρ : Monotone <| f.permOn ρ) :
    f.permOn π = f.permOn ρ := by
  refine ofFn_injective <|
    ofFn_permOn_perm_ofFn_permOn.eq_of_pairwise (fun _ _ _ _ => le_antisymm) ?_ ?_ <;>
  grind [pairwise_ofFn, monotone_iff_forall_lt]

theorem eq_of_monotone_of_permOn [LinearOrder α] {f g : Tuple α n}
    (hf : Monotone <| f) (hg : Monotone <| g) (hfg : ∃ π, f.permOn π = g) : f = g := by
  grind [permOn_eq_self_of_monotone]

@[grind =>]
theorem permOn_eq_of_antitone_of_antitone [LinearOrder α] {f : Tuple α n} {π ρ : Perm (Fin n)}
    (hπ : Antitone <| f.permOn π) (hρ : Antitone <| f.permOn ρ) : f.permOn π = f.permOn ρ :=
  Tuple.unique_antitone hπ hρ

theorem monotone_permOn_sort [LinearOrder α] {f : Tuple α n} :
    Monotone (f.permOn (Tuple.sort f)) := Tuple.monotone_sort _

end PermOn

abbrev Comparator (n : ℕ) := Fin n × Fin n

section CSwap

@[specialize] def cSwap [LinearOrder α] (f : Tuple α n) (ij : Comparator n) : Tuple α n :=
  fun k => if k = ij.1 then min (f ij.1) (f ij.2) else
  if k = ij.2 then max (f ij.1) (f ij.2) else f k

variable [LinearOrder α]

@[grind =]
theorem cSwap_apply {f : Tuple α n} {ij : Comparator n} {k : Fin n} : f.cSwap ij k =
  if k = ij.1 then min (f ij.1) (f ij.2) else if k = ij.2 then max (f ij.1) (f ij.2) else f k := rfl

@[simp]
theorem cSwap_apply_left {f : Tuple α n} {ij : Comparator n} :
    f.cSwap ij ij.1 = min (f ij.1) (f ij.2) := by grind

@[simp]
theorem cSwap_apply_right {f : Tuple α n} {ij : Comparator n} :
    f.cSwap ij ij.2 = max (f ij.1) (f ij.2) := by grind

@[simp]
theorem cSwap_apply_of_ne_of_ne {f : Tuple α n} {ij : Comparator n} {k : Fin n}
    (hki : k ≠ ij.1) (hkj : k ≠ ij.2) : f.cSwap ij k = f k := by grind

@[simp]
theorem cSwap_of_eq {f : Tuple α n} {i : Fin n} : f.cSwap (i, i) = f := by grind

theorem cSwap_of_monotone_of_le {f : Tuple α n} (hf : Monotone f) {ij : Comparator n}
    (hij : ij.1 ≤ ij.2) : f.cSwap ij = f := by grind [Monotone]

theorem cSwap_of_antitone_of_ge {f : Tuple α n} (hf : Antitone f) {ij : Comparator n}
    (hij : ij.2 ≤ ij.1) : f.cSwap ij = f := by grind [Antitone]

@[simp, grind =>]
theorem cSwap_of_apply_le {f : Tuple α n} {ij : Comparator n} (hij : f ij.1 ≤ f ij.2) :
    f.cSwap ij = f := by grind

@[simp, grind =>]
theorem cSwap_of_apply_ge {f : Tuple α n} {ij : Comparator n} (hij : f ij.2 ≤ f ij.1) :
    f.cSwap ij = f.permOn (Equiv.swap ij.1 ij.2) := by grind

theorem cSwap_left_le_cSwap_right {f : Tuple α n} {ij : Comparator n} :
    f.cSwap ij ij.1 ≤ f.cSwap ij ij.2 := by grind

theorem cSwap_permOn {f : Tuple α n} {π : Perm (Fin n)} {ij : Comparator n} :
    (f.permOn π).cSwap ij =
    (f.cSwap (ij.map π π)).permOn π := funext <| fun k => by
  simp only [cSwap_apply, permOn_apply, Prod.map_fst, EmbeddingLike.apply_eq_iff_eq, Prod.map_snd]

theorem permOn_cSwap {f : Tuple α n} {π : Perm (Fin n)} {ij : Comparator n} :
    (f.cSwap ij).permOn π =
    (f.permOn π).cSwap (ij.map ⇑π⁻¹ ⇑π⁻¹) := by simp [cSwap_permOn, Prod.map_map]

@[grind =]
theorem cSwap_perm_apply_perm_apply {f : Tuple α n} {π : Perm (Fin n)} {ij : Comparator n} :
    f.cSwap (ij.map π π) = ((f.permOn π).cSwap ij).permOn π⁻¹ := by simp [cSwap_permOn]

end CSwap

section CSwapPerm

@[specialize] def cSwapPerm [LinearOrder α] (f : Tuple α n) (ij : Comparator n) : Perm (Fin n) :=
  if f ij.1 ≤ f ij.2 then 1 else Equiv.swap ij.1 ij.2

variable [LinearOrder α]

@[grind =]
theorem cSwapPerm_apply {f : Tuple α n} {ij : Comparator n} : f.cSwapPerm ij k =
    if f ij.1 ≤ f ij.2 then k else if k = ij.1 then ij.2 else if k = ij.2 then ij.1 else k := by
  grind [cSwapPerm, Perm.one_apply]

@[simp, grind =]
theorem cSwapPerm_inv {f : Tuple α n} {ij : Comparator n} :
    (f.cSwapPerm ij)⁻¹ = f.cSwapPerm ij := by
  grind [cSwapPerm, inv_one, Equiv.swap_inv]

@[simp, grind =]
theorem cSwapPerm_mul_cSwapPerm {f : Tuple α n} {ij : Comparator n} :
    f.cSwapPerm ij * f.cSwapPerm ij = 1 := by
  grind [Perm.one_apply, Perm.mul_apply]

@[simp]
theorem cSwapPerm_apply_of_apply_le {f : Tuple α n} {ij : Comparator n} (h : f ij.1 ≤ f ij.2) :
    f.cSwapPerm ij k = k := by grind

@[simp]
theorem cSwapPerm_apply_left_of_apply_gt {f : Tuple α n} {ij : Comparator n}
    (h : f ij.2 < f ij.1) : f.cSwapPerm ij ij.1 = ij.2 := by grind

@[simp]
theorem cSwapPerm_apply_right_of_apply_gt {f : Tuple α n} {ij : Comparator n}
    (h : f ij.2 < f ij.1) : f.cSwapPerm ij ij.2 = ij.1 := by grind

@[simp]
theorem cSwapPerm_apply_of_ne_of_ne {f : Tuple α n} {ij : Comparator n} {k : Fin n}
    (hki : k ≠ ij.1) (hkj : k ≠ ij.2) : f.cSwapPerm ij k = k := by grind

@[simp, grind =]
theorem permOn_cSwap_cSwapPerm {f : Tuple α n} {ij : Comparator n} :
    (f.cSwap ij).permOn (f.cSwapPerm ij) = f := by grind

@[simp, grind =]
theorem permOn_cSwapPerm {f : Tuple α n} {ij : Comparator n} :
    f.permOn (f.cSwapPerm ij) = f.cSwap ij := permOn_injective (f.cSwapPerm ij) <| by grind

open List in
theorem ofFn_cSwap_perm_self {f : Tuple α n} {ij : Comparator n} :
    ofFn (f.cSwap ij) ~ ofFn f := by
  rw [← permOn_cSwapPerm]
  exact ofFn_permOn_perm_ofFn

end CSwapPerm

abbrev ComparatorNetwork (n : ℕ) := List (Comparator n)

section CSwaps

variable [LinearOrder α]

@[specialize] def cSwaps (f : Tuple α n) (l : ComparatorNetwork n) : Tuple α n := l.foldl cSwap f

@[simp, grind =]
theorem cSwaps_nil {f : Tuple α n} : f.cSwaps [] = f := rfl

@[simp, grind =]
theorem cSwaps_cons {f : Tuple α n} {ij : Comparator n} {l : ComparatorNetwork n} :
    f.cSwaps (ij :: l) = (f.cSwap ij).cSwaps l := rfl

theorem cSwaps_singleton {f : Tuple α n} {ij : Comparator n} :
    f.cSwaps [ij] = f.cSwap ij := by grind

@[simp, grind =]
theorem cSwaps_append {f : Tuple α n} {l l' : ComparatorNetwork n} :
    f.cSwaps (l ++ l') = (f.cSwaps l).cSwaps l' := by
  induction l generalizing f <;> grind

theorem cSwaps_append_singleton {f : Tuple α n} {ij : Comparator n} {l : ComparatorNetwork n} :
  f.cSwaps (l ++ [ij]) = (f.cSwaps l).cSwap ij := by grind

theorem cSwaps_permOn {f : Tuple α n} {π : Perm (Fin n)} {l : ComparatorNetwork n} :
    (f.permOn π).cSwaps l = (f.cSwaps (l.map <| Prod.map π π)).permOn π := by
  induction l generalizing f <;> grind [cSwap_permOn]

theorem permOn_cSwaps {f : Tuple α n} {π : Perm (Fin n)} {l : ComparatorNetwork n} :
    (f.cSwaps l).permOn π = (f.permOn π).cSwaps
    (l.map <| Prod.map ⇑π⁻¹ ⇑π⁻¹) := by simp [cSwaps_permOn, Prod.map_comp_map]

@[grind =]
theorem cSwaps_map_map_perm_perm {f : Tuple α n} {π : Perm (Fin n)} {l : ComparatorNetwork n} :
    f.cSwaps (l.map <| Prod.map π π) = ((f.permOn π).cSwaps l).permOn π⁻¹ := by simp [cSwaps_permOn]

section CSwapsPerm

@[specialize] def cSwapsPerm [LinearOrder α] (f : Tuple α n) (l : ComparatorNetwork n) :
    Perm (Fin n) := (l.foldl (fun fπ ij => (fπ.1.cSwap ij,
    fπ.2 * fπ.1.cSwapPerm ij)) (f, 1)) |>.2

@[simp, grind =]
theorem cSwapsPerm_nil {f : Tuple α n} : f.cSwapsPerm [] = 1 := rfl

@[simp, grind =]
theorem cSwapsPerm_cons {f : Tuple α n} {ij : Comparator n} {l : ComparatorNetwork n} :
    f.cSwapsPerm (ij :: l) = f.cSwapPerm ij * (f.cSwap ij).cSwapsPerm l
      := by
  suffices ∀ (f : Tuple α n) (π : Perm (Fin n)), (l.foldl (fun fπ ij => (fπ.1.cSwap ij,
    fπ.2 * fπ.1.cSwapPerm ij)) (f, π)).2 =
      π * ((l.foldl (fun fπ ij => (fπ.1.cSwap ij,
    fπ.2 * fπ.1.cSwapPerm ij)) (f, 1)) |>.2) by grind [cSwapsPerm]
  induction l <;> grind

theorem cSwapsPerm_singleton {f : Tuple α n} {ij : Comparator n} :
    f.cSwapsPerm [ij] = f.cSwapPerm ij := by grind

@[simp, grind =]
theorem cSwapsPerm_append {f : Tuple α n} {l l' : ComparatorNetwork n} :
    f.cSwapsPerm (l ++ l') = f.cSwapsPerm l * (f.cSwaps l).cSwapsPerm l' := by
  induction l generalizing f <;> grind

theorem cSwapsPerm_append_singleton {f : Tuple α n} {ij : Comparator n} {l : ComparatorNetwork n} :
  f.cSwapsPerm (l ++ [ij]) = f.cSwapsPerm l * (f.cSwaps l).cSwapPerm ij := by grind

@[simp, grind =]
theorem permOn_cSwaps_cSwapsPerm {f : Tuple α n} {l : ComparatorNetwork n} :
    (f.cSwaps l).permOn (f.cSwapsPerm l)⁻¹ = f := by
  induction l generalizing f <;> grind [inv_one, mul_inv_rev]

@[simp, grind =]
theorem permOn_cSwapsPerm {f : Tuple α n} {l : ComparatorNetwork n} :
    f.permOn (f.cSwapsPerm l) = f.cSwaps l := permOn_injective (f.cSwapsPerm l)⁻¹ <| by grind

open List in
theorem ofFn_cSwaps_perm_self {f : Tuple α n} {l : ComparatorNetwork n} :
    ofFn (f.cSwaps l) ~ ofFn f := by
  rw [← permOn_cSwapsPerm]
  exact ofFn_permOn_perm_ofFn

end CSwapsPerm

@[grind =>]
theorem cSwaps_permOn_eq_cSwaps_of_monotone_cSwaps {f : Tuple α n} {π : Perm (Fin n)}
    {l : ComparatorNetwork n} (hl : ∀ f : Tuple α n, Monotone (f.cSwaps l)) :
    (f.permOn π).cSwaps l = f.cSwaps l := by
  refine List.ofFn_injective <| (ofFn_cSwaps_perm_self.trans <|
    ofFn_permOn_perm_ofFn.trans ofFn_cSwaps_perm_self.symm).eq_of_pairwise
    (fun _ _ _ _ => le_antisymm) ?_ ?_ <;> grind [List.pairwise_ofFn, monotone_iff_forall_lt]

theorem permOn_cSwaps_map_of_monotone_cSwaps {f : Tuple α n} {π : Perm (Fin n)}
    {l : ComparatorNetwork n} (hl : ∀ f : Tuple α n, Monotone (f.cSwaps l)) :
    (f.cSwaps (l.map <| Prod.map π π)).permOn π = f.cSwaps l := by grind

end CSwaps

end Tuple
