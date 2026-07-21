import Mathlib.Algebra.Group.Submonoid.Units
import Mathlib.Algebra.Group.Pi.Units

/-!
# Transporting submonoids and subgroups along a `MulEquiv`

Supplementary constructions that restrict a monoid isomorphism `e : M₁ ≃* M₂` to matching
sub-objects: given a membership correspondence, `e` induces an isomorphism between them. The units
variants handle the case where `e` lands in a unit group `Mˣ`, so a subgroup of `G` corresponds to a
submonoid of `M` via its units.
-/

namespace MulEquiv

/-- Restrict a monoid isomorphism `e : M₁ ≃* M₂` to submonoids `S` and `T` whose memberships
correspond under `e`, giving `S ≃* T`. -/
def subtypeMulEquiv {M₁ M₂ : Type*} [Monoid M₁] [Monoid M₂] (e : M₁ ≃* M₂)
  {S : Submonoid M₁} {T : Submonoid M₂} (h : ∀ s, s ∈ S ↔ (e s) ∈ T) : S ≃* T where
  toEquiv := e.subtypeEquiv h
  map_mul' := by simp_rw [toEquiv_eq_coe, Equiv.toFun_as_coe, Equiv.subtypeEquiv_apply,
    Submonoid.coe_mul, coe_toEquiv, map_mul, Submonoid.mk_mul_mk, Subtype.forall, implies_true]

lemma mem_iff_map_mem_units_of_mem_iff_map_mem {G M : Type*} [Group G] [Monoid M] (e : G ≃* Mˣ)
  {S : Subgroup G} {T : Submonoid M} (h : ∀ s, s ∈ S ↔ (e s : M) ∈ T) (s : G) :
  s ∈ S ↔ e s ∈ T.units := by
  simp_rw [Submonoid.mem_units_iff, ← e.map_inv, ← h, iff_self_and, inv_mem_iff, imp_self]

/-- Given `e : G ≃* Mˣ` and a subgroup `S` of `G` corresponding to a submonoid `T` of `M`, the
induced isomorphism between `S` and the units submonoid `T.units`. -/
def subgroupMulEquivUnits {G M : Type*} [Group G] [Monoid M] (e : G ≃* Mˣ)
  {S : Subgroup G} {T : Submonoid M} (h : ∀ s, s ∈ S ↔ (e s : M) ∈ T) : S ≃* T.units :=
  e.subtypeMulEquiv (e.mem_iff_map_mem_units_of_mem_iff_map_mem h)

/-- As `subgroupMulEquivUnits`, but landing in the unit group `Tˣ` of the submonoid rather than its
units submonoid. -/
def subgroupMulEquivUnitsType {G M : Type*} [Group G] [Monoid M] (e : G ≃* Mˣ)
  {S : Subgroup G} {T : Submonoid M} (h : ∀ s, s ∈ S ↔ (e s : M) ∈ T) : S ≃* Tˣ :=
  (e.subgroupMulEquivUnits h).trans T.unitsEquivUnitsType

end MulEquiv
