# Test Specification: `temp_monitor`

**Test level:** unit (module) test · **Method:** requirements-based black-box design, then white-box coverage check · **Framework:** Unity (C) · **Coverage target:** 100 % statement, branch and MC/DC

## 1. Test design techniques

### Equivalence classes (input `temp_dc`, units of 0.1 °C)

| Class | Range | Expected | Valid? |
|---|---|---|---|
| EC1 | INT16_MIN … −401 | `TM_SENSOR_FAULT` | invalid |
| EC2 | −400 … 849 | `TM_NORMAL` | valid |
| EC3 | 850 … 1049 | `TM_WARNING` | valid |
| EC4 | 1050 … 1500 | `TM_SHUTDOWN` | valid |
| EC5 | 1501 … INT16_MAX | `TM_SENSOR_FAULT` | invalid |

### Boundary values

Every class edge is tested on both sides: **−401 / −400**, **849 / 850**, **1049 / 1050**, **1500 / 1501**, plus the hysteresis edge **799 / 800** and the type limits **INT16_MIN / INT16_MAX** (robustness).

### State-based tests

Hysteresis, the shutdown latch and the invalid-sample counter depend on history, so those cases are written as sequences of samples (state transition testing).

## 2. Test cases and traceability

| Test case | Input / sequence | Expected result | Requirement(s) |
|---|---|---|---|
| TC01 | 250 | NORMAL | REQ-TM-01 |
| TC02 | 849 | NORMAL | REQ-TM-01 |
| TC03 | 850 | WARNING | REQ-TM-02 |
| TC04 | 1049 | WARNING | REQ-TM-02 |
| TC05 | 1050 | SHUTDOWN | REQ-TM-03 |
| TC06 | −400 | NORMAL | REQ-TM-04 |
| TC07 | −401 | SENSOR_FAULT | REQ-TM-04 |
| TC08 | 1500 | SHUTDOWN | REQ-TM-03, REQ-TM-04 |
| TC09 | 1501 | SENSOR_FAULT | REQ-TM-04 |
| TC10 | INT16_MIN, INT16_MAX | SENSOR_FAULT (no crash) | REQ-TM-04 (robustness) |
| TC11 | 900 → 800 | WARNING (held) | REQ-TM-05 |
| TC12 | 900 → 799 | NORMAL (released) | REQ-TM-05 |
| TC13 | 500 → 820 | NORMAL (hysteresis only applies when coming down) | REQ-TM-01, REQ-TM-05 |
| TC14 | 1100 → 200 | SHUTDOWN (latched) | REQ-TM-06 |
| TC15 | 1100, reset(799) → 500 | reset OK, then NORMAL | REQ-TM-07 |
| TC16 | 1100, reset(800) → 500 | reset REJECTED, still SHUTDOWN | REQ-TM-07 |
| TC17 | 1100, reset(−401) → 500 | reset REJECTED, still SHUTDOWN | REQ-TM-07 |
| TC18 | −500 ×2 → 300 | NORMAL (no latch after 2) | REQ-TM-08 |
| TC19 | −500 ×3 → 300 | SHUTDOWN on 3rd, stays latched | REQ-TM-08, REQ-TM-06 |
| TC20 | −500 ×2, 300, −500 ×2 | SENSOR_FAULT (counter was reset) | REQ-TM-08 |
| TC24 | −500 ×300 → 300 | counter saturates at 3, still SHUTDOWN | REQ-TM-08 (robustness) |
| TC21 | each state | 100 / 50 / 0 / 0 % | REQ-TM-09 |
| TC22 | invalid enum value 99 | 0 % (fail-safe default) | REQ-TM-09 (robustness) |
| TC23 | NULL ctx / NULL out / NULL reset / NULL init | `TM_ERR_NULL`, state unchanged | REQ-TM-10 |

**Requirement coverage:** all 10 requirements have at least one test case; each test names its requirement in the code.

## 3. Structural coverage result (gcov, GCC 14)

| Metric | Result |
|---|---|
| Statement | 100 % |
| Branch | 100 % |
| MC/DC (condition outcomes) | 100 % |

**Finding from coverage analysis:** the first run reached only 97.2 % MC/DC. gcov showed that the `invalid_count < TM_MAX_INVALID` check was never false, because no test fed more than 3 invalid samples in a row. The saturation logic, which stops the `uint8_t` counter from wrapping around to 0 after 255 invalid samples, was untested. TC24 was added to close this gap. This is why requirements-based tests are followed by a structural coverage check.
