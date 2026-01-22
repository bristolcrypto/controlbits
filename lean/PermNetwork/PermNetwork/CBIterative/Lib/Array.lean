namespace Array

variable {α β γ : Type u} {k i : Nat}

def bswap (xs : Array α) (b : Bool) (i j : Nat) (hi : i < xs.size := by get_elem_tactic)
    (hj : j < xs.size := by get_elem_tactic) : Array α := bif b then xs.swap i j else xs

@[grind =>] theorem size_bswap {xs : Array α} {b : Bool} {i j : Nat} {hi hj} :
    (xs.bswap b i j hi hj).size = xs.size := by grind [bswap]

@[grind =>] theorem getElem_bswap {xs : Array α} {b : Bool} {i j : Nat} {hi hj}
    (hk : k < (xs.bswap b i j hi hj).size) :
    (xs.bswap b i j hi hj)[k] = bif b then (xs.swap i j)[k]'(by grind) else xs[k]'(by grind) := by
  grind [bswap]

@[simp]
theorem bswap_true {xs : Array α} {i j : Nat} {hi hj} :
    xs.bswap true i j hi hj = xs.swap i j hi hj := by grind

@[simp]
theorem bswap_false {xs : Array α} {i j : Nat} {hi hj} :
    xs.bswap false i j hi hj = xs := by grind

def bswapImpl (xs : Array α) (b : Bool) (i j : Nat) (hi : i < xs.size := by get_elem_tactic)
    (hj : j < xs.size := by get_elem_tactic) : Array α :=
  let v₁ := bif b then xs[j] else xs[i]
  let v₂ := bif b then xs[i] else xs[j]
  let xs' := xs.set i v₁
  xs'.set j v₂ (Nat.lt_of_lt_of_eq hj (size_set _).symm)

@[csimp] theorem bswap_eq_bswapImpl : @bswap = @bswapImpl := by
  ext <;> grind [bswap, bswapImpl]

def bswapIfInBounds (xs : Array α) (b : Bool) (i j : @& Nat) : Array α :=
  bif b then xs.swapIfInBounds i j else xs

@[grind =] theorem size_bswapIfInBounds {xs : Array α} {b : Bool} {i j : Nat} :
    (xs.bswapIfInBounds b i j).size = xs.size := by grind [bswapIfInBounds]

@[grind =] theorem getElem_bswapIfInBounds {xs : Array α} {b : Bool} {i j : Nat}
    (hk : k < (xs.bswapIfInBounds b i j).size) :
    (xs.bswapIfInBounds b i j)[k] =
    bif b then (xs.swapIfInBounds i j)[k]'(by grind) else xs[k]'(by grind) := by
  grind [bswapIfInBounds]

@[simp]
theorem bswapIfInBounds_true {xs : Array α} {i j : Nat} :
    xs.bswapIfInBounds true i j  = xs.swapIfInBounds i j := by grind

@[simp]
theorem bswapIfInBounds_false {xs : Array α} {i j : Nat} :
    xs.bswapIfInBounds false i j = xs := by grind

def bswapIfInBoundsImpl (xs : Array α) (b : Bool) (i j : @& Nat) : Array α :=
  if hi : i < xs.size then if hj : j < xs.size then xs.bswap b i j else xs else xs

@[csimp] theorem bswapIfInBounds_eq_bswapIfInBoundsImpl :
    @bswapIfInBounds = @bswapIfInBoundsImpl := by
  ext <;> grind [bswapIfInBounds, bswapIfInBoundsImpl]

end Array
