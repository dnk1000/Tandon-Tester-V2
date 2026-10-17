# Product Requirements Document
## ESP32-S3 Tandon TM-100 Floppy Drive Diagnostic & Exerciser Tool

**Document Version:** 1.0  
**Date:** 2026-10-16  
**Status:** Draft for Engineering  
**Primary Target:** Tandon TM-100-1 5.25-inch floppy disk drive  
**Architecture:** ESP32-S3 with USB serial command-line console  

---

# 1. Executive Summary

This product is a standalone bench diagnostic and exerciser tool for testing, troubleshooting, restoring, and repairing Tandon TM-100-series floppy disk drives, with the initial design targeted at the TM-100-1.

The tester will replace the need for a vintage host computer or floppy disk controller during basic drive diagnostics. It will directly control the drive's low-level interface signals and monitor drive feedback signals.

The primary operator interface will be a human-readable serial command-line interface presented over USB from an ESP32-S3.

The tester will support:

- Drive selection
- Spindle motor control
- Head direction and stepping
- Track 0 detection
- Logical track position tracking
- Index pulse monitoring
- RPM measurement
- Write-protect sensing
- Read-data activity detection
- Read-data pulse timing analysis
- Power rail voltage and current monitoring
- Automated seek and endurance tests
- Controlled write testing in later revisions
- Raw signal access for oscilloscope or logic-analyzer use

The product is intended primarily as a repair and diagnostic instrument, not as a replacement PC floppy controller.

---

# 2. Goals

## 2.1 Primary Goals

The product shall:

1. Allow a technician to manually exercise all major TM-100 drive mechanisms and interface signals.
2. Allow the technician to observe drive status without requiring a vintage computer.
3. Provide automated tests for common drive failure modes.
4. Provide repeatable, quantitative measurements such as spindle RPM and rail current.
5. Protect the ESP32 and drive from expected interface-voltage and current differences.
6. Minimize the risk of accidental disk writes.
7. Provide an interface that is both human-readable and scriptable.
8. Support future expansion to additional Shugart-compatible 5.25-inch floppy drives.

## 2.2 Secondary Goals

The product should:

- Assist in refurbishing multiple old drives efficiently.
- Make intermittent mechanical problems easier to reproduce.
- Serve as a breakout/interface adapter for oscilloscopes and logic analyzers.
- Permit PC-side automation through Python or similar scripting.
- Support raw timing capture for later FM/MFM analysis.

---

# 3. Non-Goals

Revision A is not required to:

- Read DOS/FAT filesystems.
- Decode directory structures.
- Emulate a full PC floppy disk controller.
- Perform complete FM/MFM sector decoding.
- Align heads automatically.
- Replace all oscilloscope-based alignment procedures.
- Support 8-inch floppy drives.
- Support modern 3.5-inch-only features unless they are electrically compatible.

Full magnetic write/read verification is deferred to later revisions.

---

# 4. Target Drive

## 4.1 Initial Target

Tandon TM-100-1.

Characteristics relevant to this design include:

- 5.25-inch floppy drive
- Single-sided
- Shugart-style 34-pin interface
- Nominal spindle speed: 300 RPM
- Track range approximately 0-39
- +5 V and +12 V power rails
- TTL/open-collector style control interface
- Raw read-data and write-data interface

## 4.2 Future Compatibility

The hardware should be designed to make future compatibility practical with:

- Tandon TM-100-2
- Other 34-pin Shugart-compatible 5.25-inch drives
- Selected double-sided drives
- Drives using compatible Drive Select, Motor, Step, Direction, Index, Track 0, Write Protect, Read Data, Write Data, and Write Gate signals

---

# 5. System Architecture

The system shall consist of:

1. ESP32-S3 controller
2. USB-C interface
3. Output driver stage
4. Input conditioning / level-shifting stage
5. 34-pin floppy interface connector
6. Floppy power connector
7. +5 V and +12 V power monitoring
8. Over-current protection
9. Optional physical write-enable interlock
10. Test points for critical signals

High-level architecture:

```text
                    USB-C
                      |
                +-------------+
                |  ESP32-S3   |
                +------+------+
                       |
        +--------------+---------------+
        |                              |
 +------+-------+               +------+-------+
 | Output       |               | Input        |
 | Drivers      |               | Buffers      |
 +------+-------+               +------+-------+
        |                              |
 MOTOR / STEP / DIR / DS      INDEX / TRACK0 / WP
 WRITE GATE / WRITE DATA      READ DATA
        |                              |
        +--------------+---------------+
                       |
                 34-pin cable
                       |
                 +-----------+
                 | TM-100-1  |
                 +-----------+

Power Input
   |
   +---- +12 V monitor/protection ----> Drive
   |
   +---- +5 V regulator/monitor ------> Drive
   |
   +---- logic power -----------------> Tester
```

---

# 6. Operator Interface

## 6.1 Primary Interface

The primary operator interface shall be a serial command-line console over USB.

Preferred implementation:

- ESP32-S3 native USB CDC
- Standard USB-C connector
- Compatible with Windows, macOS, and Linux
- Usable with PuTTY, Tera Term, minicom, screen, or similar terminal software

Nominal terminal settings:

- 115200 baud equivalent
- 8 data bits
- No parity
- 1 stop bit
- No hardware flow control
- Accept CR, LF, or CRLF

Native USB CDC may not use a physical UART baud rate, but 115200 shall be the documented nominal terminal setting.

## 6.2 Startup Banner

Example:

```text
TM100 Drive Diagnostic Tool
Firmware 1.0.0

Interface: READY
5V:  5.02 V
12V: 12.08 V

Type 'help' for available commands.

tm100>
```

---

# 7. Command-Line Requirements

## 7.1 General CLI Requirements

The CLI shall:

- Be case-insensitive for command names.
- Provide a `help` command.
- Provide command-specific help.
- Return a clear success or error message.
- Never leave hardware in an undefined state following invalid input.
- Permit Ctrl-C or equivalent cancellation of continuous tests.
- Use consistent naming and output formatting.
- Be usable interactively by a human.
- Be reasonably parseable by PC-side scripts.

Future output modes may include:

- Human-readable
- CSV
- JSON

## 7.2 Core Command Set

Revision A shall support at minimum:

```text
help
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

read activity [time]
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
config drive_select <0|1|2|3>
config save
config defaults
```

Future revisions may add:

```text
read capture
write arm
write disarm
write pattern <pattern>
write test
output <human|csv|json>
```

---

# 8. Drive Selection

The tester shall provide independent control of Drive Select 0-3.

Example:

```text
tm100> select 0
OK: DS0 asserted
```

```text
tm100> select off
OK: drive deselected
```

Only one Drive Select line shall be asserted at a time.

Future firmware should support:

```text
select auto
```

Auto mode shall test each Drive Select line and attempt to determine which select line causes meaningful drive status activity.

---

# 9. Motor Control

The tester shall independently control the drive spindle motor.

Commands:

```text
motor on
motor off
```

Optional timed form:

```text
motor on 10
```

The firmware should support automatic motor shutdown after configurable inactivity.

The tester shall monitor Index pulses while the motor is active.

---

# 10. Head Direction and Step Control

## 10.1 Manual Stepping

The tester shall support:

```text
step in
step out
step in <count>
step out <count>
```

Where:

- `in` means toward higher-numbered tracks.
- `out` means toward Track 0.

The CLI shall use intuitive terminology and not require the operator to reason in terms of electrical DIR states.

## 10.2 Timing

The firmware shall enforce safe direction setup, step pulse width, and minimum step spacing.

Default minimum track-to-track step interval:

**6 ms**

Configurable values may be supported down to the drive's allowable minimum.

## 10.3 Logical Track Counter

The firmware shall maintain an internal logical track number.

The track number shall be marked invalid after:

- Power-up
- Reset
- Drive deselection if configured to invalidate position
- Unexpected Track 0 behavior
- Explicit operator command, if provided

The logical track shall become valid only after a successful `home`.

---

# 11. Home / Restore Function

The `home` command shall:

1. Select the configured drive if not already selected.
2. Set Direction toward Track 0.
3. Issue step pulses at a safe interval.
4. Monitor Track 0.
5. Stop immediately when Track 0 is detected.
6. Set logical track to 0.
7. Fail after a configurable maximum number of steps.

Default home limit:

**50 steps**

Example success:

```text
tm100> home
Seeking Track 0...
Track 0 detected after 37 steps
Track = 0
PASS
```

Example failure:

```text
tm100> home
Seeking Track 0...
ERROR: Track 0 not detected after 50 steps
ABORTED
```

---

# 12. Seek Function

Command:

```text
seek <track>
```

The tester shall:

- Reject seeks if logical position is unknown.
- Reject tracks below 0.
- Reject tracks above the configured maximum track.
- Determine direction automatically.
- Generate the required number of step pulses.
- Update the logical track counter.
- Enforce timing requirements.

Default maximum track:

**39**

Example:

```text
tm100> seek 20
Seeking...
Track: 20
OK
```

---

# 13. Track 0 Monitoring

Command:

```text
track0
```

Example:

```text
TRACK0: ACTIVE
```

The firmware shall continuously maintain the most recent Track 0 state internally.

If Track 0 becomes active unexpectedly while the logical track counter is non-zero, the firmware should flag a position inconsistency.

---

# 14. Write Protect Monitoring

Command:

```text
wp
```

Example:

```text
WRITE PROTECT: ACTIVE
```

The write-protect state shall also be shown in the `status` command.

Write-protect status shall be used as a safety interlock for future write functions.

---

# 15. Index Monitoring

Command:

```text
index
```

The tester shall report:

- Current Index state
- Frequency
- Period
- Pulse width
- Valid/invalid status

Example:

```text
INDEX
-----
State:       ACTIVE
Frequency:   5.001 Hz
Period:      199.96 ms
Pulse width: 3.21 ms
```

Loss of Index while the motor is expected to be running shall be considered a fault condition for automated tests.

---

# 16. RPM Measurement

Command:

```text
rpm
```

Optional:

```text
rpm <samples>
```

The tester shall calculate spindle speed using Index intervals.

Nominal target:

**300 RPM**

Nominal Index frequency:

**5 Hz**

Default measurement sample count:

**10 revolutions**

The tester shall report:

- Average RPM
- Minimum RPM
- Maximum RPM
- Deviation
- Pass/fail result

Example:

```text
RPM TEST

Samples:      10
RPM average:  300.12
RPM minimum:  299.88
RPM maximum:  300.35
Deviation:    0.16 %

PASS
```

The acceptable RPM tolerance shall be configurable.

Default tolerance:

**±1.5%**

---

# 17. Read-Data Activity Test

Command:

```text
read activity
```

Optional:

```text
read activity <seconds>
```

The tester shall detect and count transitions on the Read Data signal.

The test shall report:

- Number of transitions
- Measurement duration
- Calculated transitions per second
- Signal present / absent

Example:

```text
READ ACTIVITY

Duration:        1.000 s
Transitions:     126481
Rate:            126481 /s
Signal present:  YES

PASS
```

Revision A does not require sector decoding.

---

# 18. Read Timing Analysis

Command:

```text
read timing <samples>
```

The ESP32-S3 shall timestamp Read Data transitions.

The firmware should report:

- Minimum interval
- Maximum interval
- Average interval
- Distribution / histogram
- Number of captured samples

Example:

```text
READ TIMING

Captured: 10000 transitions

Minimum interval: 1.97 us
Maximum interval: 6.11 us
Average interval: 3.84 us

Histogram:
2 us   4219
4 us   3876
6 us   1905
```

Implementation should use hardware timer, RMT, capture peripheral, or equivalent mechanism rather than software polling.

---

# 19. Raw Read Capture — Future Requirement

A later revision should support:

```text
read capture
```

The command shall stream timestamped Read Data transitions to the host PC.

Potential formats:

- Plain text
- CSV
- Binary
- JSON metadata plus binary payload

This mode may support PC-side:

- FM decoding
- MFM decoding
- Clock/data separation
- Sector analysis
- Alignment diagnostics

---

# 20. Automated Basic Diagnostic

Command:

```text
test basic
```

The basic diagnostic shall test, where practical:

1. Controller health
2. +5 V rail
3. +12 V rail
4. Drive selection
5. Track 0
6. Motor control
7. Index detection
8. RPM
9. Write Protect input
10. Read Data activity

Example:

```text
TM-100 BASIC DIAGNOSTIC

Drive Select........ PASS
+5V Rail............ PASS  5.01 V
+12V Rail........... PASS  12.08 V
Track 0............. PASS
Motor Start......... PASS
Index Detected...... PASS
RPM................. PASS  300.2 RPM
Write Protect....... PASS
Read Activity....... PASS

RESULT: PASS
```

The test shall not perform destructive write operations.

---

# 21. Automated Seek Diagnostic

Command:

```text
test seek
```

Default sequence:

```text
Home
Seek 10
Seek 20
Seek 39
Home
```

Example:

```text
SEEK TEST

Home................ PASS
Seek Track 10....... PASS
Seek Track 20....... PASS
Seek Track 39....... PASS
Return Track 0...... PASS

RESULT: PASS
```

Failures shall identify the operation at which the fault occurred.

---

# 22. Exercise / Endurance Test

Command:

```text
test exercise <cycles>
```

Default motion cycle:

```text
Track 0 -> Track 39 -> Track 0
```

Example:

```text
tm100> test exercise 100

Cycle 10/100
Cycle 20/100
Cycle 30/100
...
Cycle 100/100

Cycles completed: 100
Track0 errors:     0
Seek errors:       0

PASS
```

The test shall support cancellation.

Optional future modes may vary:

- Maximum track
- Step speed
- Delay at endpoints
- Number of cycles

---

# 23. Status Command

Command:

```text
status
```

Example:

```text
DRIVE STATUS
------------

Selected:       YES
Drive Select:   DS0
Motor:          ON
Direction:      IN
Track:          17
Track valid:    YES

Track 0:        INACTIVE
Write Protect:  ACTIVE

Index:          ACTIVE
Index freq:     5.002 Hz
RPM:            300.12

Read activity:  YES
Read pulses:    127843 / sec

POWER
-----

+5V:             5.01 V
+5V current:     382 mA
+12V:           12.07 V
+12V current:    718 mA
```

---

# 24. Power System Requirements

## 24.1 Drive Power

The tester shall supply or pass through:

- +12 V
- +5 V
- Ground

Recommended design capacity:

- +12 V at 2 A
- +5 V at 2 A

The design shall tolerate normal TM-100 startup and motor current.

## 24.2 Monitoring

The tester shall measure:

- +5 V rail voltage
- +5 V rail current
- +12 V rail voltage
- +12 V rail current

Resolution targets:

- Voltage: 10 mV or better preferred
- Current: 10 mA or better preferred

## 24.3 Protection

The power subsystem shall provide hardware protection independent of firmware.

Recommended protections:

- Reverse-polarity protection
- Input fuse or resettable fuse
- +5 V current limiting
- +12 V current limiting
- Over-voltage protection where practical
- Bulk and local decoupling

A firmware crash shall not defeat basic over-current protection.

---

# 25. Power Commands

Command:

```text
power
```

Example:

```text
POWER STATUS
------------

+5V:            5.015 V
+5V Current:    391 mA

+12V:          12.083 V
+12V Current:   734 mA

Total Power:    10.82 W
```

Continuous mode:

```text
power watch
```

Example:

```text
12.08V  0.71A | 5.01V  0.39A
12.07V  0.72A | 5.01V  0.39A
12.08V  0.71A | 5.02V  0.38A
```

Ctrl-C shall terminate continuous output.

---

# 26. Electrical Interface Requirements

## 26.1 General

The ESP32 GPIO shall not be connected directly to the floppy-drive interface.

The interface shall include appropriate:

- Level translation
- Open-collector/open-drain drive capability
- Input buffering
- Protection
- Signal conditioning

## 26.2 Controller-to-Drive Signals

Signals include:

- Drive Select 0
- Drive Select 1
- Drive Select 2
- Drive Select 3
- Motor On
- Direction
- Step
- Write Gate / Write Enable
- Write Data
- Side Select for future compatibility

These signals shall be driven using appropriate open-collector/open-drain compatible circuitry.

Candidate implementation:

- 74LS06 / 7406 class buffer
- Modern functional equivalent
- Discrete transistor or MOSFET open-drain stage

Driver devices should preferably be inexpensive and replaceable.

## 26.3 Drive-to-Controller Signals

Signals include:

- Index
- Track 0
- Write Protect
- Read Data

These shall pass through 5 V-tolerant interface circuitry before reaching the ESP32.

Index, Track 0, and Write Protect may use:

- Schmitt-trigger buffers
- Comparators
- 5 V tolerant logic

Read Data shall use a high-speed path appropriate for pulse timing measurement.

---

# 27. Signal Connector Requirements

The tester shall provide a standard 34-pin floppy-drive interface connection.

The hardware should support the conventional Shugart/PC-style signal set including:

- Drive Select 0-3
- Motor On
- Direction
- Step
- Write Data
- Write Gate
- Track 0
- Write Protect
- Read Data
- Side Select
- Index
- Signal grounds

The board shall clearly indicate Pin 1 orientation.

Reverse insertion protection should be provided where practical through:

- Keyed connectors
- Shrouded headers
- Cable keying
- Clear silkscreen marking

---

# 28. Physical Write Safety

The tester shall be designed such that accidental disk writing is difficult.

At minimum, actual write operations shall require:

1. Firmware write arming
2. Write Protect inactive
3. Valid logical track
4. Motor running
5. Index detected
6. Drive selected

Strongly recommended:

A physical jumper, toggle switch, or removable link shall independently disable the Write Gate driver.

With the physical write-enable control disabled, no firmware operation shall be capable of asserting Write Gate at the drive connector.

---

# 29. Write Arming — Future Firmware

Proposed sequence:

```text
tm100> write arm

WARNING:
Writing may destroy disk contents.
Type: ARM WRITE
```

Operator:

```text
ARM WRITE
```

Tester:

```text
WRITE FUNCTIONS ARMED
```

Write mode shall automatically disarm after:

- Timeout
- Reset
- USB disconnect
- Drive deselection
- Write error
- Power fault

Recommended default timeout:

**60 seconds**

---

# 30. Write Pattern Testing — Future Requirement

Potential commands:

```text
write pattern 55
write pattern aa
write test
write disarm
```

Later revisions may support:

- Repeating bit pattern generation
- Index-synchronized writes
- Raw write-data streams
- Read-after-write verification
- FM patterns
- MFM patterns

Write functionality is not required for Revision A.

---

# 31. Firmware Architecture

Firmware shall be separated into functional layers.

Recommended architecture:

```text
CLI Layer
   |
Test / Command Layer
   |
Drive Control API
   |
Hardware Abstraction
   |
ESP32 GPIO / Timers / RMT / ADC / I2C
```

The CLI shall not manipulate drive GPIO directly.

Example drive API:

```c
drive_select();
drive_deselect();

motor_on();
motor_off();

step_in();
step_out();

home();
seek();

get_track0();
get_write_protect();

measure_index();
measure_rpm();

measure_read_activity();
capture_read_timing();
```

This architecture shall centralize timing and safety enforcement.

---

# 32. Internal Drive State

The firmware shall maintain a state model similar to:

```c
struct drive_state {
    bool selected;
    uint8_t drive_select;

    bool motor_on;

    bool position_valid;
    int track;

    bool track0;
    bool write_protected;

    bool index_present;
    float rpm;

    bool read_activity;
    uint32_t read_pulse_rate;

    bool write_armed;
};
```

The state model shall be updated from both commands and measured hardware state.

---

# 33. Timing and Safety Rules

The firmware shall enforce all drive-interface timing centrally.

Examples include:

- Direction setup before Step
- Minimum Step pulse width
- Minimum interval between Step pulses
- Settling delay following seek
- Motor spin-up time before read/write tests
- Write Gate timing relative to Step
- No stepping while writing
- No write operation when Write Protect is active
- No write operation when track position is invalid

Illegal state transitions shall return an error.

Example:

```text
ERROR: write prohibited while drive position is unknown
```

---

# 34. Configuration

Command:

```text
config show
```

Minimum configurable parameters:

- Drive Select
- Maximum track
- Step interval
- Home step limit
- RPM sample count
- RPM tolerance
- Motor spin-up delay
- Read activity sample period

Examples:

```text
config step_time 6
config max_track 39
config home_limit 50
config rpm_samples 10
config drive_select 0
```

Persistent settings:

```text
config save
```

Restore defaults:

```text
config defaults
```

Configuration shall use ESP32 nonvolatile storage.

---

# 35. Machine-Readable Output — Future Requirement

Future firmware should support:

```text
output human
output csv
output json
```

Example:

```text
tm100> output json
tm100> status
```

```json
{
  "selected": true,
  "drive_select": 0,
  "motor": true,
  "track": 17,
  "track_valid": true,
  "track0": false,
  "write_protect": true,
  "index": true,
  "rpm": 300.12,
  "read_activity": true,
  "read_pulse_rate": 127843,
  "v5": 5.01,
  "i5": 0.382,
  "v12": 12.07,
  "i12": 0.718
}
```

---

# 36. Error Handling

All commands shall terminate in one of:

- `OK`
- `PASS`
- `FAIL`
- `ERROR`

Errors shall include descriptive text.

Examples:

```text
ERROR: Track position unknown. Run 'home' first.
```

```text
ERROR: Track 42 exceeds configured maximum of 39.
```

```text
FAIL: Index pulse not detected.
```

```text
FAIL: RPM outside tolerance.
```

Automated tests shall stop safely or continue according to defined test behavior.

---

# 37. Fault Detection

The firmware should detect and report conditions such as:

- +5 V under-voltage
- +5 V over-current
- +12 V under-voltage
- +12 V over-current
- Missing Index
- RPM out of tolerance
- Track 0 not found
- Unexpected Track 0 assertion
- Read Data absent
- Write Protect state inconsistent with attempted write
- Drive not selected
- Invalid track position
- USB command interrupted

---

# 38. Test Points

The hardware shall expose accessible test points for at least:

- Ground
- +5 V
- +12 V
- Index
- Track 0
- Read Data
- Step
- Direction
- Motor On
- Write Data
- Write Gate

Preferred implementation:

- Labeled 0.1-inch header
- Grabber-friendly pads
- Optional BNC/SMA for high-value signals such as Read Data and Index

This allows the tester to double as an interface breakout during oscilloscope diagnostics.

---

# 39. LEDs

Revision A may include minimal indicators:

- Power
- USB / controller activity
- Drive Select
- Motor
- Fault
- Write Armed

The Write Armed indicator, if implemented, shall be visually distinct.

---

# 40. Physical Enclosure

The finished unit should include:

- USB-C connector
- DC power input
- Power switch
- 34-pin floppy connector
- 4-pin floppy power connector
- Write-enable physical switch/jumper
- Status LEDs
- External test points

The enclosure shall clearly label connector orientation and voltage rails.

---

# 41. Logging

Future firmware should optionally log:

- Test start time
- Firmware version
- Configuration
- RPM measurements
- Rail voltage/current
- Seek failures
- Endurance-test counts
- Read-data statistics

Logs may be streamed over USB rather than stored locally.

---

# 42. Firmware Update

The ESP32-S3 shall support convenient firmware updates.

Preferred methods:

- USB bootloader
- USB CDC / DFU if practical
- ESP32 standard flashing process

OTA update is not required.

---

# 43. Development Phases

## Revision A — Drive Exerciser

Required:

- USB serial CLI
- Drive Select 0-3
- Motor control
- Direction
- Step
- Home
- Seek
- Logical track counter
- Track 0 sensing
- Write Protect sensing
- Index sensing
- RPM measurement
- Read Data activity detection
- Basic read timing
- +5 V voltage/current
- +12 V voltage/current
- Basic automated test
- Seek test
- Exercise/endurance test
- Configuration storage
- Physical write-disable hardware

Revision A shall not require actual write-data generation.

## Revision B — Read Analyzer

Add:

- Higher-resolution raw Read Data capture
- Timing histograms
- Streaming capture to PC
- CSV/binary output
- Optional FM/MFM analysis
- PC-side analysis scripts

## Revision C — Write / Read Verification

Add:

- Write Data generation
- Write Gate control
- Safe write arming
- Index-synchronized patterns
- Read-after-write verification
- Pattern comparison
- FM/MFM test patterns
- Magnetic write-path diagnostics

---

# 44. Acceptance Criteria — Revision A

Revision A shall be considered functionally complete when the following can be demonstrated on a known-good TM-100-1:

1. Tester enumerates as a USB serial device.
2. CLI accepts commands reliably.
3. All four Drive Select outputs can be controlled.
4. Motor can be independently enabled and disabled.
5. Head can be stepped in both directions.
6. Home reliably detects Track 0.
7. Seek accurately moves between known logical tracks.
8. Track 0 state is correctly reported.
9. Write Protect state is correctly reported.
10. Index pulses are detected.
11. RPM is calculated correctly from Index.
12. Read Data activity can be detected on a formatted disk.
13. Basic Read Data timing can be captured.
14. +5 V voltage and current are reported.
15. +12 V voltage and current are reported.
16. Basic automated diagnostic runs without user intervention.
17. Seek test runs and reports results.
18. Endurance test can complete at least 100 Track 0-to-39-to-0 cycles.
19. Invalid commands do not create unsafe drive states.
20. Physical write-disable hardware prevents Write Gate assertion.

---

# 45. Suggested Revision A Command Reference

```text
help
help <command>

version
status
reset

select 0
select 1
select 2
select 3
select off

motor on
motor off

home

step in
step out
step in <count>
step out <count>

seek <track>

track0
wp

index
rpm
rpm <samples>

read activity
read activity <seconds>

read timing <samples>

power
power watch

test basic
test seek
test exercise <cycles>

config show
config step_time <ms>
config max_track <track>
config home_limit <steps>
config rpm_samples <count>
config rpm_tolerance <percent>
config drive_select <0|1|2|3>
config save
config defaults
```

---

# 46. Recommended Default Configuration

```text
drive_select      = 0
max_track         = 39
step_time_ms      = 6
home_limit        = 50
rpm_samples       = 10
rpm_tolerance_pct = 1.5
motor_spinup_ms   = 500
read_sample_ms    = 1000
```

These values shall remain firmware-configurable.

---

# 47. Future Enhancements

Potential future features include:

- Automatic Drive Select discovery
- TM-100-2 double-sided support
- Side Select control
- Head-load support for drives requiring it
- Flux-like raw timing export
- FM decoder
- MFM decoder
- Sector read/write tools
- Track quality scoring
- Index jitter analysis
- Motor startup-time measurement
- Stepper-current characterization
- Automated burn-in testing
- Drive serial-number database
- PC GUI front-end
- Python automation library
- CSV test report generation
- Known-good reference-disk test suite

---

# 48. Engineering Principles

The project shall follow these principles:

1. **Protect the vintage hardware.**  
   Hardware current limiting and interface buffers are required.

2. **Do not expose ESP32 pins directly to the drive.**  
   Proper 5 V-compatible interfacing is mandatory.

3. **Keep write operations physically and logically guarded.**  
   No disk should be modified accidentally.

4. **Centralize timing in the drive-control layer.**  
   CLI code shall not directly manipulate critical interface signals.

5. **Prefer measurement over assumption.**  
   RPM, current, Index, Track 0, and Read Data should be measured and reported.

6. **Keep the CLI scriptable.**  
   Human-readable output should not prevent later PC automation.

7. **Make the tool useful during repair, not only for pass/fail testing.**  
   Manual control and test-point access are core requirements.

---

# 49. Open Engineering Decisions

The following decisions remain for schematic and detailed-design work:

1. Exact ESP32-S3 module.
2. Exact GPIO assignment.
3. Output-driver IC selection.
4. Input-buffer IC selection.
5. High-speed Read Data capture circuitry.
6. +5 V generation method.
7. +12 V input architecture.
8. Current-sense IC selection.
9. Connector style and keying.
10. Physical Write Enable implementation.
11. ESD/transient protection.
12. Enclosure and PCB form factor.
13. Use of RMT vs timer/capture peripheral for Read Data.
14. Whether Read Data receives comparator conditioning.
15. Whether motor power is switchable by firmware or only the Motor On signal is controlled.

These items shall be resolved during hardware architecture and schematic design.

---

# 50. Next Engineering Deliverable

The next deliverable should be a detailed hardware design specification containing:

- Complete 34-pin signal map
- ESP32-S3 GPIO assignment
- Exact output-driver circuits
- Exact input-buffer circuits
- Read Data input circuit
- Index input circuit
- Power architecture
- Current-sense circuits
- Protection components
- Physical Write Enable circuit
- Recommended component part numbers
- Preliminary BOM
- Schematic block diagram
- Firmware pin abstraction
- Timing-state diagram

That document will provide sufficient detail to begin schematic capture and PCB layout.
