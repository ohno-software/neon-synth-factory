# Neon FM - Algorithm Reference

Visual reference for all 8 FM synthesis algorithms used in Neon FM.

## Algorithm 1: Serial Chain
Linear cascade through all operators.

![Serial Chain](source/resources/algorithms/algo_1_serial.svg)

**Topology:** 1 → 2 → 3 → 4 → OUT

---

## Algorithm 2: Branch
Two paths converge at operator 4.

![Branch](source/resources/algorithms/algo_2_branch.svg)

**Topology:** (1 → 2 → 4) + (3 → 4) → OUT

---

## Algorithm 3: Dual Stack
Two independent FM pairs.

![Dual Stack](source/resources/algorithms/algo_3_dualstack.svg)

**Topology:** (1 → 2) + (3 → 4) → OUT

---

## Algorithm 4: Triple Modulator
Three modulators feeding a single carrier.

![Triple Modulator](source/resources/algorithms/algo_4_triplemod.svg)

**Topology:** (1 + 2 + 3) → 4 → OUT

---

## Algorithm 5: One to Many
Single modulator feeding three independent carriers.

![One to Many](source/resources/algorithms/algo_5_onetomany.svg)

**Topology:** 1 → (2 + 3 + 4) → OUT

---

## Algorithm 6: Parallel Dual
One FM pair + two independent carriers.

![Parallel Dual](source/resources/algorithms/algo_6_paralleldual.svg)

**Topology:** (1 → 2) + 3 + 4 → OUT

---

## Algorithm 7: Complex Fork
One modulator to two operators, both feed the carrier.

![Complex Fork](source/resources/algorithms/algo_7_complexfork.svg)

**Topology:** 1 → (2, 3) → 4 → OUT

---

## Algorithm 8: Full Parallel
All operators independent (additive synthesis).

![Full Parallel](source/resources/algorithms/algo_8_fullparallel.svg)

**Topology:** 1 + 2 + 3 + 4 → OUT

---

## Color Legend

- **Blue boxes** = Modulators (modifying operator, not directly audible)
- **Green boxes** = Carriers (directly audible output operators)
- **Arrows** = FM modulation routing
