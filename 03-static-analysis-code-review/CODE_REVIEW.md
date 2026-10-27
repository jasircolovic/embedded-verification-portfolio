# Code Review: `sensor_filter.c` (12-bit ADC current sensor filter)

**Reviewed:** `src_before/sensor_filter.c` · **Fixed:** `src_after/sensor_filter.c` · **Regression tests:** `test/test_sensor_filter.c`
**Method:** manual review against a checklist, plus three static analyzers (`make analyze`): GCC warnings, cppcheck with the MISRA C:2012 addon, and clang-tidy (Clang Static Analyzer, bugprone, CERT C).

## Findings

| ID | Line | Severity | Defect | Found by | Fix |
|---|---|---|---|---|---|
| D1 | 16 | **Critical** | `head > BUF_SIZE` lets `head` reach 8, so the next push writes `buffer[8]`: out-of-bounds write, memory corruption | **Review only** | `head = (head + 1) % SF_BUF_SIZE` |
| D2 | 24 | **Critical** | `sum` used uninitialized, so the average is garbage | cppcheck, clang-tidy | initialize to 0; widen to `uint32_t` (8 × 4095 fits) |
| D3 | 26 | **Critical** | `i <= count` reads one element past the stored samples, and reads `buffer[8]` when full | **Review only** | `i < count` |
| D4 | 28 | **Critical** | `sum / count` divides by zero when no sample has been pushed | **Review only** (clang-tidy flagged the line, but only because of D2) | return `SF_ERR_EMPTY` when `count == 0` |
| D5 | 33 | Major | `int` → `unsigned char` implicit narrowing; no input range check, so raw > 4095 gives wrong % | GCC `-Wconversion` | range check, explicit `uint32_t` math and cast |
| D6 | 39 | **Critical** | `if (avg = limit)` assigns instead of comparing: over-current is *always* reported | GCC, cppcheck, clang-tidy, MISRA 13.4 | `avg >= SF_OVERCURRENT_RAW` |
| D7 | 53 | **Critical** | returns pointer to local array `name`: dangling pointer | GCC, cppcheck, clang-tidy | return string literals (static storage) |
| D8 | 47–51 | Major | missing `break` in case 2 (50 % reported as "critical"); no `default`; 100 % (`case 4`) unhandled, so returns uninitialized data | GCC, clang-tidy, MISRA 16.3/16.4 | add `break`, `case 4`, `default` |
| D9 | whole file | Minor | `int` for ADC values and indexes; magic numbers; no header; `sprintf` from `<stdio.h>` | MISRA 8.4, 21.6, Dir 4.6 (review) | `stdint.h` types, named constants, header file, no stdio |

## Analyzer results (`make analyze`)

| Tool | Before | After |
|---|---|---|
| GCC `-Wall -Wextra -Wconversion` | 4 | 0 |
| cppcheck + MISRA addon | 22 | 5 (all Rule 15.5, see deviation) |
| clang-tidy | 14 | 0 |

## Key lesson

**The three worst memory bugs (D1, D3, D4) were found by human review, not by any of the free tools.** Static analysis finds many defects cheaply, so it runs first and the reviewer can focus on logic. It is not a replacement for review or for testing. Commercial analyzers such as Klocwork or Polyspace are built for deeper inter-procedural path and value analysis, which is exactly the class of bug D1/D3/D4 belong to.

## Documented deviation

**MISRA C:2012 Rule 15.5** (Advisory: "a function should have a single point of exit at the end"). The 5 remaining findings are early `return`s used for parameter validation (NULL pointer, empty buffer, range). Rewriting them to a single exit would add nesting and hurt readability. Advisory rules may be deviated with a documented rationale, so this is accepted as a project-level deviation. Required and Mandatory rules have no remaining violations.

## Review checklist used

- Array indexes and loop bounds (`<` vs `<=`, wrap-around)
- Every variable initialized before use
- Division: can the divisor be 0?
- Integer types: signedness, width, implicit conversions, overflow of intermediate results
- Input validation at module boundaries (range, NULL)
- `=` vs `==` in conditions
- Pointers to local variables escaping the function
- `switch`: `break` on every case, `default` present, all values handled
- Magic numbers replaced by named constants
- Return values of library calls checked
