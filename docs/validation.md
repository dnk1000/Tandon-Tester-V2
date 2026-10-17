# Validation record

## Executed in the cloud environment

Target: ESP32-S3, 8 MiB flash, no PSRAM; ESP-IDF v5.4.3 at commit
`ea1c174c1cbb7348bd8ba0ff1eb306246938dd80`.
Host: Linux x86-64, GCC 14.2, Python 3.12, CMake 4.4.4, Ninja 1.13.2.
Cross compiler: `xtensa-esp-elf` esp-14.2.0_20250730.
Python dependencies used by this build are recorded in
`tools/cloud-python-requirements.txt`.

The portable CTest targets compile production source with `-Wall -Wextra -Werror`
and assertions explicitly enabled:

| Target | Coverage |
|---|---|
| `safety_tests` | 31 safety/functional groups: config boundaries, home-required motion, auto-selection, exact homing limit including last allowed pulse, Track 0 stuck high/low, unexpected Track 0 while idle, min step spacing, cancellation, rail faults, drive switching, independent motor, all CLI forms and malformed/overflow input, RMT reducer including DMA boundaries and initial fragments, signed INA226 conversion, voltage/current fault classification, basic/seek diagnostics and a simulated 100-cycle exercise with 7,810 pulses from initial track 10 |
| `nvs_tests` | Defaults without saved settings, save/load round trip, fractional tolerance, reject invalid updates, unknown schema/truncated data fallback, init/write/commit error propagation; no automatic erase |
| `power_driver_tests` | Actual INA226 register I/O: address and IDs, configuration/calibration/alert writes, big-endian conversion, monitor-reset detection, I2C failures, stale reading and physical fault inhibition |
| `hal_tests` | Actual HAL output latch sequencing, all floppy outputs default LOW, one-hot break-before-make DS selection, motor/direction/side semantics, pulse setup/width/hold, safe deassertion, input polarity, and assertions that Write Gate, Write Data and write LED are never driven HIGH |

Tests use hardware/NVS doubles, not an emulated ESP32 or actual I2C bus. A test
pass establishes software behavior under those inputs, not mechanical/electrical
acceptance. Test command output distinguishes completed versus failed/cancelled
cycles and sensor observations versus exercised inputs.

## Build result

`idf.py build` completed successfully for ESP32-S3 and produced the bootloader,
partition table, `build/tm100_tester.elf`, and `build/tm100_tester.bin`. Flash size is
8 MiB, console is USB Serial/JTAG, power management is disabled, and radios are
not initialized. All four CTest targets passed. The reusable cloud installation
script was also executed against the prepared environment and completed its build
and tests; restoration/publication into a new task has not been tested.

## Physical validation still required

No ESP32 board, TM-100, scope, formatted disk, or serial device was attached.
Build completion does not establish the following acceptance criteria:

- USB enumeration, ROM flashing, reconnect and terminal cancellation behavior.
- Real GPIO mapping, LS06 inversion and sink levels, pull-downs, reset glitches,
  interlock isolation, and connector orientation/rail polarity.
- Direction meaning, Track 0 repeatability, full-range seeks and 100 physical cycles.
- Actual INA226 addresses, calibrated rail measurements versus a DMM/load, fault
  detection/response during motor transients and I2C disconnect/recovery.
- RPM error ≤0.1% versus a scope and missing-index/fault behavior on the drive.
- RMT and PCNT behavior at 125/250/500 kHz and 1 MHz, including idle, truncated
  captures, DMA servicing under USB/I2C load, exact interval/histogram values and
  the 16-bit PCNT overflow boundary.
- Write Protect input toggling: diagnostics report its observed state; there is
  no actuator or reference input that permits an automatic electrical PASS.

Follow HDS bring-up phases before connecting a drive. Keep physical Write Enable
OFF. Resolve the documented schematic connectivity problems before fabrication.
