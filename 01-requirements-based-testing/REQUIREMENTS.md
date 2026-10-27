# Requirements: Inverter Over-Temperature Monitor (`temp_monitor`)

Context: a power-electronics inverter must reduce or cut output power when its heatsink gets too hot.
Temperatures are given in **deci-degrees Celsius** (`int16_t`, 1 unit = 0.1 °C), as typical ADC-based firmware does to avoid floating point.

| ID | Requirement |
|---|---|
| REQ-TM-01 | A valid temperature below 85.0 °C (850) shall be classified as `TM_NORMAL`. |
| REQ-TM-02 | A valid temperature from 85.0 °C (850) up to and including 104.9 °C (1049) shall be classified as `TM_WARNING`. |
| REQ-TM-03 | A valid temperature of 105.0 °C (1050) or higher shall be classified as `TM_SHUTDOWN`. |
| REQ-TM-04 | A sample outside the sensor range −40.0 °C … 150.0 °C (−400 … 1500) shall be treated as invalid and classified as `TM_SENSOR_FAULT`. |
| REQ-TM-05 | Hysteresis: once in `TM_WARNING`, the module shall return to `TM_NORMAL` only when the temperature drops below 80.0 °C (800). |
| REQ-TM-06 | `TM_SHUTDOWN` shall be latched: it stays active for every following sample until `tm_reset()` succeeds. |
| REQ-TM-07 | `tm_reset()` shall clear the latch only if the current temperature is valid and below 80.0 °C (800); otherwise it shall return `TM_ERR_REJECTED` and keep the latch. |
| REQ-TM-08 | Three consecutive invalid samples shall latch `TM_SHUTDOWN` (fail-safe). A valid sample resets the invalid-sample counter. |
| REQ-TM-09 | The allowed output power shall be 100 % in `TM_NORMAL`, 50 % in `TM_WARNING`, and 0 % in `TM_SHUTDOWN` and `TM_SENSOR_FAULT`. |
| REQ-TM-10 | Every function receiving a pointer shall return `TM_ERR_NULL` for a NULL pointer and leave the state unchanged. |
