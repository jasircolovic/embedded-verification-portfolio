# Embedded Software Verification Portfolio

Five small projects showing the work of an embedded software test and verification engineer: specifying tests from requirements, unit testing C code, measuring structural coverage up to MC/DC, static analysis against MISRA C, code review, hardware mocking with fault injection, and automated integration testing.

Each module comes from a safety-relevant domain: power electronics, railway, medical devices and motor control.

```bash
make all      # clean build, all tests, coverage, static analysis (needs gcc-14, cppcheck, clang-tidy, pytest)
```

CI runs the same command on every push (`.github/workflows/ci.yml`).

## Results

| # | Project | Domain | Tests | Statement | Branch | MC/DC |
|---|---|---|---|---|---|---|
| 1 | [Requirements-based unit testing](01-requirements-based-testing) | Inverter over-temperature protection | 24 | 100 % | 100 % | 100 % |
| 2 | [MC/DC coverage](02-mcdc-coverage) | Railway door interlock | 7 | 100 % | 100 % | 100 % |
| 3 | [Static analysis & code review](03-static-analysis-code-review) | ADC current-sensor filter | 7 | 100 % | 100 % | 100 % |
| 4 | [HAL mocking & fault injection](04-hal-mocking-fault-injection) | Infusion-pump pressure sensor (I2C) | 14 | 100 % | 100 % | 100 % |
| 5 | [Automated integration testing](05-integration-tests-python) | Motor controller ECU, serial protocol | 25 | 100 % | 100 % | 100 % |

## Tool mapping

The commercial tools used in industry need licenses, so each project uses an open-source tool that does the same job:

| Industry tool | Purpose | Used here |
|---|---|---|
| Tessy (Razorcat), Cantata (QA Systems) | Unit/integration test execution, stubs, coverage incl. MC/DC | Unity test framework, hand-written fakes, GCC 14 `gcov --conditions` |
| Klocwork (Perforce), Polyspace, PC-lint | Static analysis, MISRA C compliance | cppcheck + MISRA C:2012 addon, clang-tidy, GCC warnings |
| HIL / test bench + Python | Automated integration tests via real interfaces | pytest driving a simulated ECU over a byte-stream protocol |
| Jenkins / GitLab CI | Toolchain automation | GitHub Actions |

## What each project shows

**1. Requirements-based unit testing.** 10 written requirements ([REQUIREMENTS.md](01-requirements-based-testing/REQUIREMENTS.md)) are turned into a test specification ([TEST_SPEC.md](01-requirements-based-testing/TEST_SPEC.md)) using equivalence classes, boundary values and state-transition sequences, with a full traceability matrix. Coverage analysis then found a gap that the requirements-based tests had missed (counter saturation), and a test was added for it.

**2. MC/DC coverage.** A 4-condition railway interlock decision is tested with the minimal N+1 = 5 vectors, with the independence pairs documented ([MCDC_ANALYSIS.md](02-mcdc-coverage/MCDC_ANALYSIS.md)). `make coverage-partial` shows a suite with 100 % statement coverage but only 70 % MC/DC, which is why safety standards ask for MC/DC.

**3. Static analysis & code review.** A deliberately defective module is reviewed by hand and by three analyzers. Findings, severity, which tool caught what, and the fixes are in [CODE_REVIEW.md](03-static-analysis-code-review/CODE_REVIEW.md), with a documented deviation for MISRA advisory Rule 15.5. The three worst memory bugs were caught only by manual review. Each finding has a regression test.

**4. HAL mocking & fault injection.** An I2C sensor driver is tested on a PC by replacing the hardware layer at link time with a scriptable fake. The tests inject timeouts, NACKs, corrupted CRCs and out-of-range values, and verify the retry logic (exactly 3 attempts, no retry on a valid-but-impossible value). The CRC is checked against the sensor vendor's reference vector.

**5. Automated integration testing.** A simulated ECU with a framed serial protocol ([PROTOCOL.md](05-integration-tests-python/PROTOCOL.md)) is tested black-box from Python. The suite covers nominal commands, state machine rules (emergency stop latch), protocol errors, resynchronization after line noise, split and packed frames, and a 500-frame seeded fuzz test. It produces a JUnit XML report for CI and measures which ECU code the integration tests reached.
