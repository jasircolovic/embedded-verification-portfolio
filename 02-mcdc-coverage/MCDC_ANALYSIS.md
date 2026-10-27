# MC/DC Analysis: Railway Door Interlock

Decision: `permit = A && (B || C) && !D`

| Condition | Meaning |
|---|---|
| A | train at standstill (speed ≤ 0.2 km/h) |
| B | doors on platform side |
| C | driver override |
| D | emergency brake fault |

## Why MC/DC

Modified Condition/Decision Coverage requires that **each condition is shown to independently change the decision outcome** while the other conditions do not change in a way that affects the result. It is highly recommended for SIL 4 in IEC 61508-3 (Annex B) and for ASIL D in ISO 26262-6, and is the coverage objective for Level A software in DO-178C (avionics). Full truth-table testing needs 2^N tests. MC/DC needs at least N+1, here 5 instead of 16.

## Test vectors

| Vector | A | B | C | D | Result |
|---|---|---|---|---|---|
| V1 | 1 | 1 | 0 | 0 | **1** |
| V2 | 0 | 1 | 0 | 0 | **0** |
| V3 | 1 | 0 | 0 | 0 | **0** |
| V4 | 1 | 0 | 1 | 0 | **1** |
| V5 | 1 | 1 | 0 | 1 | **0** |

## Independence pairs

| Condition | Pair | Only this condition changes | Result changes |
|---|---|---|---|
| A | V1 ↔ V2 | A: 1 → 0 | 1 → 0 ✔ |
| B | V1 ↔ V3 | B: 1 → 0 | 1 → 0 ✔ |
| C | V3 ↔ V4 | C: 0 → 1 | 0 → 1 ✔ |
| D | V1 ↔ V5 | D: 0 → 1 | 1 → 0 ✔ |

## Measured with GCC 14 (`-fcondition-coverage`, `gcov --conditions`)

| Suite | Statement | Branch | MC/DC |
|---|---|---|---|
| Full (V1–V5) | 100 % | 100 % | **100 %** |
| Partial (V1, V2, V5) — `make coverage-partial` | 100 % | 80 % | **70 %** |

**Lesson:** the partial suite executes every line, so statement coverage says 100 %. It never shows that B or C can change the outcome on their own. A bug such as writing `B && C` instead of `B || C` would pass all partial tests. That is why safety standards ask for MC/DC rather than statement coverage.

Note: GCC implements *masking* MC/DC (because of short-circuit evaluation, a condition that is not evaluated is masked). Commercial tools such as Tessy and Cantata report MC/DC in the same spirit and produce certification evidence for it.
