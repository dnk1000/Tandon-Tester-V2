# TM100 Tester — Revision A

Non-writing firmware for **ESP32-S3-WROOM-1-N8**, using **ESP-IDF v5.4.3** and
native **USB Serial/JTAG** (GPIO19/20). No Arduino, PSRAM, radio initialization,
sector decoder, write arming, Write Gate control, or magnetic write generation.

The supplied [PRD](docs/reference/TM100_ESP32_Diagnostic_Tool_PRD.md) and
[hardware design spec](docs/reference/TM100_ESP32_Hardware_Design_Spec.md) are
retained alongside the [original KiCad draft](docs/reference/kicad/README.md).
See [design decisions and conflicts](docs/design-decisions.md) before connecting hardware.
The supplied schematic is **not a verified, connected production netlist**.

## Build, flash, monitor

Install the ESP-IDF **v5.4.3** ESP32-S3 tools using Espressif's normal installation
instructions, then source its `export.sh`. CMake and Ninja are required.

```sh
cd Tandon-Tester-V2
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

Use your actual serial port (`COMn` on Windows, `/dev/cu.usbmodem…` on macOS).
If the USB application is unavailable, hold BOOT, press/release RESET, release
BOOT, and flash through the ROM USB downloader. IDF Monitor exits with Ctrl-].
Configure IDF Monitor's Ctrl-C passthrough if needed; entering `stop` followed by
Enter is the equivalent firmware cancellation command.

Console: 115200 nominal, 8N1, no flow control, local echo off. Native USB ignores
the physical baud rate. CR, LF, and CRLF work. Commands are case-insensitive.
Wait for the `tm100>` prompt before sending another command. A command received
while busy is rejected and cancels the operation; motion commands are never queued.
`help <command>` gives usage. Invalid input releases controls and invalidates position.

In the prepared cloud machine:

```sh
cd /workspace/Tandon-Tester-V2
. tools/activate_cloud.sh
idf.py build
```

`bash tools/setup_cloud.sh` reproduces tool installation/build/host checks outside
the checkout; it never resets an existing SDK checkout. The cloud Python package
snapshot is pinned in `tools/cloud-python-requirements.txt`. The optional upstream
constraints host was blocked, so this setup uses IDF's supported `--no-constraints`
and PyPI-only installation. Tool archive checksums and TLS verification remain
active. Standard local ESP-IDF installations can use their upstream constraints.
Cloud scripts were exercised on Python 3.12; other hosts should use the normal
Espressif installer. No API keys or application servers are required.

## First session

**Bring up and verify the board without a drive first**, following HDS section 36.
Keep physical Write Enable OFF. Check the hardware 10 kΩ pull-downs and write
interlock, reset behavior, LS06 sink levels, input buffers, and drive power wiring
with bench instruments. Firmware cannot substitute for these electrical checks.
The drive requires an external 12 V supply; USB must not power its rails.

```text
help
power
config show
select 0
home
seek 10
step out 2
motor on
index
rpm
read activity 1
read timing 10000
motor off
select off
```

Motion and motor commands require both INA226s healthy and fresh rail samples.
Unpowered rails or absent monitors leave the CLI usable but block motion.
`home` automatically selects the configured drive if none is selected. `motor on`
is independent of selection and waits the configured spin-up period. Read/RPM
measurements require both selection and motor ON. Homing and stepping do not
require the spindle motor. Normal `step` also requires homing as a conservative
Revision A rule. A manual step count is preflighted before any motion.

## Commands

```text
help [command]
version
status
reset
select <0|1|2|3|off>
motor <on|off>
home
seek <track>
step <in|out> [count]
track0
wp
index
rpm [samples]
read activity [seconds]
read timing [samples]
power
power watch
test basic
test seek
test exercise <cycles>
config show
config step_time <ms>
config max_track <n>
config home_limit <n>
config rpm_samples <n>
config rpm_tolerance <percent>
config drive_select <0|1|2|3>
config motor_spinup_ms <ms>
config read_sample_ms <ms>
config save
config defaults
stop
```

`reset` safely deasserts controls and reboots. `stop`, Ctrl-C, a detected USB
disconnect, power fault, inconsistent Track 0, or failed command releases outputs
and invalidates position. Successful manual motor/select commands retain their
explicit state. All automated diagnostics release motor/select even on success.
A detected power recovery never automatically resumes motion. Re-home explicitly.

## Configuration and timing

| Setting | Default | Valid range |
|---|---:|---|
| `step_time` | 6 ms | 5–1000 ms |
| `max_track` | 39 | 1–79 |
| `home_limit` | 50 | greater than max_track, at most 200 |
| `rpm_samples` | 10 | 1–100 intervals |
| `rpm_tolerance` | 1.5% | 0.1–20%, decimal allowed |
| `drive_select` | 0 | 0–3 |
| `motor_spinup_ms` | 500 | 300–10000 ms |
| `read_sample_ms` | 1000 | 1–60000 ms |

Settings change RAM first. `config save` validates the complete configuration
and commits a versioned NVS blob; `config defaults` changes RAM only. Every
configuration change stops the drive and invalidates position. Increase
`home_limit` **before** increasing `max_track` past it. A missing, incompatible,
truncated or invalid blob loads defaults. An NVS initialization failure preserves
flash, retains RAM defaults, and prevents saves; firmware never erases NVS on its
own. A committed blob is validated again on boot.

Step pulses target 5 µs, bracketed by at least 10 µs direction setup/hold. The
short pulse is in a critical section. The drive layer enforces at least the
configured interval (absolute floor 5 ms), waits for mechanical settling, checks
Track 0 consistency, and caps homing at exactly `home_limit` pulses. Position is
inferred from commands and Track 0, **not an encoder measurement**; seek diagnostics
verify release and return of Track 0, but cannot prove every intermediate track.

## GPIO map

All floppy output GPIOs LOW = **released/deasserted** through LS06; HIGH sinks the
external line. Buffered floppy input HIGH = **asserted** through LVC14A.

| GPIO | Function | Drive connector / notes |
|---:|---|---|
| 4 | DS0 | J1.10 |
| 5 | DS1 | J1.12 |
| 6 | DS2 | J1.14 |
| 7 | DS3 | J1.6 |
| 10 | Motor | J1.16 |
| 11 | Direction | J1.18; HIGH = inward (external LOW), LOW = outward |
| 12 | Step | J1.20 |
| 13 | Write Data | J1.22; permanently LOW in Revision A |
| 14 | Write Gate command | J1.24 through physical interlock; permanently LOW |
| 15 | Side select | J1.32; defined in HAL, held LOW; no Rev A CLI |
| 16 | Index | J1.8 through LVC14A |
| 17 | Track 0 | J1.26 through LVC14A |
| 18 | Write Protect | J1.28 through LVC14A |
| 21 | Read Data | J1.30 through LVC14A; RMT + PCNT |
| 8 / 9 | I2C SDA / SCL | 100 kHz; external 4.7 kΩ pull-ups to 3.3 V |
| 38 | Power fault | active LOW open-drain ALERT OR; internal pull-up |
| 39 | Physical Write Enable sense | HIGH = enabled; internal pull-down; status only |
| 40 | Status LED | active HIGH; drive selected |
| 41 | Fault LED | active HIGH; power-monitor faults |
| 42 | Write armed LED | always LOW |
| 19 / 20 | USB D− / D+ | reserved exclusively for native USB |
| 43 / 44 | UART0 service | untouched |

No strapping pins control the drive. Output latches are set LOW before output
mode is enabled. The external pull-downs are essential for ROM boot/reset and
unpowered MCU states. A firmware crash alone does not make driven GPIOs
high-impedance immediately; hardware interlock and protection remain necessary.

## Measurements

**Index:** GPIO any-edge interrupt timestamps using `esp_timer_get_time()` at
1 µs resolution. Status reports period, width, frequency, RPM, lifetime min/max
period and peak-to-peak jitter. An index period is stale after 1 s without an
assertion. `rpm N` collects N **new** complete rising-to-rising intervals; each
edge has a 1 s deadline. Queue overflow fails the measurement. Average RPM uses
total elapsed time; minimum/maximum RPM, deviation from 300 RPM, period extrema,
and jitter are reported. Mean and extrema must all be within configured tolerance.
Interrupt latency and real measurement accuracy must be checked against a scope.

**Read activity:** PCNT counts **assertion/rising edges**, one per read pulse,
not both electrical transitions. A 100 ns glitch filter rejects shorter pulses.
Overflow watch points extend the 16-bit counter in hardware-driver software;
there is no per-read-pulse CPU interrupt. An observation is bounded to 60 s
(maximum 60 million counts at the 1 MHz hardware target). Output uses actual
elapsed time and reports presence/absence; absent data is a failed read check,
not proof of a bad drive without a formatted disk.

**Read timing:** RMT RX on GPIO21 uses DMA, a 20 MHz clock (50 ns ticks), 100 ns
filter and 1 ms idle termination. Two 512-symbol DMA blocks feed a streaming
ISR reducer; the 4 KiB aligned receive buffer is in internal DMA-capable RAM.
The reducer preserves elapsed time and rising-edge state across block boundaries,
discards the initial partial pulse, and computes **rising-to-rising pulse intervals**,
not alternating high/low widths. Captures are bounded to 1–10000 intervals and
2 s overall. Histogram bins are `[n,n+1)` µs for n=0..14 and `>=15` µs.
A short frame, absent data or timeout reports the actual partial count and an
error. Separate frames are never joined across an idle gap. DMA is disabled
before its buffer/state is reused. No flux streaming or FM/MFM decoding is provided.
Bench pulse injection at 125/250/500 kHz and 1 MHz is still required to establish
capture throughput and exclude DMA servicing loss under load.

**Power:** U6 `0x40` = +12 V, U7 `0x41` = +5 V. Both use 25 mΩ shunts. The driver
checks TI manufacturer/device IDs, configures continuous bus/shunt conversion,
four averages, 1.1 ms conversions and CAL=2048 (100 µA current LSB). Reported current
uses signed shunt voltage directly: 2.5 µV / 25 mΩ = 100 µA/count. Bus resolution is
1.25 mV/count. Each monitor is retried after a failed identification/read; reset
calibration is detected and reinitialized. Readings are refreshed approximately
every 100 ms and rejected for motion if older than 500 ms.

Faults: +5 V outside 4.75–5.25 V, +12 V outside 11.4–12.6 V, absolute current >2 A,
reverse current below −50 mA, device math overflow, alert, missing/stale monitor,
or active GPIO38. INA226 shunt-overvoltage ALERT is configured at +2 A, active LOW,
non-latching. Voltage and negative-current faults are checked in software. The
worker checks safety at most every 5 ms during waits and approximately every
10 ms while idle, excluding bounded peripheral/console operations. Software
releases motor/select; **there is no controllable drive-power switch in this design**.
Fuses/eFuses must independently protect both rails. Fault response is not a
hard-real-time overcurrent protection guarantee.

## Diagnostics and host tests

- `test basic`: rail health, select command, home, Track 0 release/return, motor
  spin-up, new index/RPM sample, WP observation, read activity. Requires a formatted
  disk. WP is observed, not falsely claimed to have been electrically exercised.
- `test seek`: home, 10, 20, configured maximum, 0; targets above the configured
  maximum are omitted. Return to zero uses tracked seek and verifies the sensor,
  so re-homing cannot hide missed steps.
- `test exercise N`: home, then 0 → maximum → 0 for N cycles. Reports completed
  cycles, the failed motion leg and errors. Ctrl-C/stop aborts with controls released.

Run the portable tests with a native C compiler:

```sh
cmake -S tests -B tests/build -G Ninja
cmake --build tests/build
ctest --test-dir tests/build --output-on-failure
```

Tests compile the actual drive/config/parser/timing/diagnostic/HAL code against
small hardware/NVS doubles. They cover 31 functional/safety groups, a simulated
100-cycle exercise, NVS round trips and failures, malformed input, the physical
output logic convention, safe initialization, selection exclusivity, and the
absence of any asserted write output. See [validation](docs/validation.md).

Code layers: `cli` owns USB input and a single command worker; `command` parses
without effects; `diagnostics` orchestrates checks; `drive` owns motion and state;
`floppy_hal` alone drives control GPIOs; `measurements` owns RMT/PCNT/index;
`power_monitor` owns I2C and publishes locked snapshots; `config` owns validated NVS.
Track 0 and Write Protect are sampled into the drive state on idle safety polls
and during cancellable waits; status commands also read their current levels.
Only the command worker changes drive state; the input task requests cancellation
atomically. Power monitor task controls only its independent fault LED.
