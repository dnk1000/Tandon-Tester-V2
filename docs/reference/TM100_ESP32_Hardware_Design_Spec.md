# Hardware Design Specification
## ESP32-S3 Tandon TM-100 Floppy Drive Diagnostic & Exerciser

**Document Version:** 1.0  
**Date:** 2026-10-16  
**Status:** Engineering Design Baseline  
**Parent Document:** TM100 ESP32 Diagnostic Tool PRD v1.0  
**Primary Target Drive:** Tandon TM-100-1  
**Secondary Target:** Tandon TM-100-2 and compatible 34-pin Shugart-interface drives  

---

# 1. Purpose

This document converts the product requirements for the TM-100 Diagnostic & Exerciser into a hardware implementation baseline suitable for schematic capture.

The tester is a USB-connected bench instrument built around an ESP32-S3. It directly exercises the low-level floppy-drive interface while electrically isolating the ESP32's 3.3 V GPIO from the original 5 V TTL/open-collector drive interface.

Revision A is intentionally focused on non-destructive diagnostics:

- Drive select
- Motor control
- Direction
- Step
- Track 0
- Write protect
- Index
- RPM
- Raw read-data activity and timing
- +5 V and +12 V voltage/current measurement
- Automated seek and endurance testing

The board shall include the write-related interface hardware for future use, but actual magnetic write operations are not required in Revision A firmware.

---

# 2. Design Summary

Recommended implementation:

- MCU: ESP32-S3-WROOM-1-N8
- USB: native ESP32-S3 USB Serial/JTAG / CDC over USB-C
- Controller-to-drive outputs: SN74LS06 open-collector inverters
- Drive-to-controller inputs: SN74LVC14A 5 V-tolerant Schmitt-trigger buffers
- Read Data capture: dedicated SN74LVC14A channel feeding ESP32 hardware capture input
- Rail monitors: two INA226 current/voltage monitors
- Main power input: regulated 12 VDC external adapter, 3 A minimum
- +5 V generation: LMR51430-class 3 A synchronous buck converter
- Drive power: standard 4-pin floppy power connector
- Drive signal connector: keyed/shrouded 2x17 0.1-inch 34-pin header
- Write protection: firmware interlock plus independent physical hardware write-disable
- Test points: all critical interface signals and supply rails

---

# 3. Source Drive Electrical Requirements

The design shall conform to the original Tandon TM-100 interface rather than assuming modern PC-floppy electrical levels.

Key drive requirements:

- 34-pin signal interface
- +12 VDC rail
- +5 VDC rail
- Nominal +12 V current: approximately 900 mA average
- Nominal +5 V current: approximately 600 mA average
- +12 V tolerance: ±0.6 V
- +5 V tolerance: ±0.25 V
- Nominal spindle speed: 300 RPM
- Index period: approximately 200 ms
- Interface cable: ribbon/twisted-pair, up to 10 ft per Tandon documentation
- Interface signals are active-low where identified by the original interface convention
- Original interface termination is approximately 150 Ω to +5 V
- Tandon specifies a low-level interface output of no more than 0.4 V at up to 48 mA
- Minimum Step pulse width: 200 ns
- Minimum Step-to-Step interval: 5 ms
- Direction setup/hold around Step: at least 100 ns
- Motor stabilization: no more than approximately 250 ms in the original timing documentation
- Read Data pulse width nominally approximately 1 µs ±250 ns
- FM/MFM raw timing is well within ESP32-S3 timer/capture capability

The hardware shall not rely on ESP32 pins directly meeting these legacy TTL current requirements.

---

# 4. 34-Pin Drive Interface

## 4.1 Connector

Reference designator:

**J1 – DRIVE SIGNAL**

Recommended connector:

- 2x17
- 0.1-inch pitch
- Shrouded
- Keyed
- Pin 1 visibly marked on PCB
- Straight or right-angle based on enclosure layout

The cable shall preserve the Tandon direct 34-conductor mapping. No PC-style drive-select twist is required or desired for the normal bench cable.

## 4.2 Pin Assignment

| Pin | Signal | Direction at Tester | Revision A Use |
|---:|---|---|---|
| 1 | Ground / clamp | GND | Ground |
| 2 | Connector clamp / reserved | — | NC |
| 3 | Ground | GND | Ground |
| 4 | Spare | — | NC |
| 5 | Ground | GND | Ground |
| 6 | Drive Select 3 | Output | Yes |
| 7 | Ground | GND | Ground |
| 8 | Index / Sector | Input | Yes |
| 9 | Ground | GND | Ground |
| 10 | Drive Select 0 | Output | Yes |
| 11 | Ground | GND | Ground |
| 12 | Drive Select 1 | Output | Yes |
| 13 | Ground | GND | Ground |
| 14 | Drive Select 2 | Output | Yes |
| 15 | Ground | GND | Ground |
| 16 | Motor On | Output | Yes |
| 17 | Ground | GND | Ground |
| 18 | Direction Select | Output | Yes |
| 19 | Ground | GND | Ground |
| 20 | Step | Output | Yes |
| 21 | Ground | GND | Ground |
| 22 | Composite Write Data | Output | Hardware fitted; FW later |
| 23 | Ground | GND | Ground |
| 24 | Write Enable / Write Gate | Output | Hardware fitted; interlocked |
| 25 | Ground | GND | Ground |
| 26 | Track 0 | Input | Yes |
| 27 | Ground | GND | Ground |
| 28 | Write Protect | Input | Yes |
| 29 | Ground | GND | Ground |
| 30 | Composite Read Data | Input | Yes |
| 31 | Ground | GND | Ground |
| 32 | Side Select | Output | Future / TM-100-2 |
| 33 | Ground / spare | GND | Ground |
| 34 | Spare / connector clamp | — | NC |

All odd ground pins shall connect to the tester digital ground plane near the connector.

---

# 5. ESP32-S3 Module

## 5.1 Recommended Device

**U1 – ESP32-S3-WROOM-1-N8**

Recommended characteristics:

- 8 MB flash
- No PSRAM required for Revision A
- Native USB support
- Adequate GPIO count
- Hardware timers and pulse-count/capture peripherals
- 240 MHz dual-core MCU
- 3.3 V logic

An N8 module is preferred because it avoids unnecessary memory complexity while providing substantially more firmware space than required.

## 5.2 USB Pins

The ESP32-S3 native USB PHY uses:

- GPIO19 = USB D-
- GPIO20 = USB D+

These pins are reserved exclusively for USB.

## 5.3 Pins to Avoid for Normal Interface Functions

Avoid relying on ESP32-S3 strapping pins for drive-control functions:

- GPIO0
- GPIO3
- GPIO45
- GPIO46

These may be brought to service pads if desired but shall not be used for critical floppy control signals.

GPIO43/44 should remain available for optional UART0 debug access.

---

# 6. Proposed ESP32 GPIO Assignment

This assignment is the schematic baseline.

| ESP32 GPIO | Function | Direction |
|---:|---|---|
| GPIO4 | DS0_DRV | Output |
| GPIO5 | DS1_DRV | Output |
| GPIO6 | DS2_DRV | Output |
| GPIO7 | DS3_DRV | Output |
| GPIO10 | MOTOR_DRV | Output |
| GPIO11 | DIR_DRV | Output |
| GPIO12 | STEP_DRV | Output |
| GPIO13 | WRITE_DATA_DRV | Output |
| GPIO14 | WRITE_GATE_CMD | Output |
| GPIO15 | SIDE_SELECT_DRV | Output |
| GPIO16 | INDEX_IN | Input |
| GPIO17 | TRACK0_IN | Input |
| GPIO18 | WRITE_PROTECT_IN | Input |
| GPIO21 | READ_DATA_IN | Input |
| GPIO8 | I2C_SDA | Bidirectional |
| GPIO9 | I2C_SCL | Output |
| GPIO38 | POWER_FAULT_IN | Input |
| GPIO39 | HW_WRITE_ENABLE_SENSE | Input |
| GPIO40 | LED_STATUS | Output |
| GPIO41 | LED_FAULT | Output |
| GPIO42 | LED_WRITE_ARMED | Output |
| GPIO43 | UART0_TX service header | Output |
| GPIO44 | UART0_RX service header | Input |
| GPIO47 | Spare / future | Bidirectional |
| GPIO48 | Spare / future | Bidirectional |
| GPIO19 | USB D- | USB |
| GPIO20 | USB D+ | USB |

GPIO assignments may be changed during PCB routing, but USB, strapping-pin avoidance, input/output grouping, and safe power-up behavior shall be preserved.

---

# 7. Controller-to-Drive Output Stage

## 7.1 Requirements

The following lines are driven by the tester:

- DS0
- DS1
- DS2
- DS3
- Motor On
- Direction
- Step
- Write Data
- Write Gate
- Side Select

The drive interface expects open-collector style sinking capability and can impose significantly greater current than an ESP32 GPIO should sink directly.

## 7.2 Recommended Driver

Use:

**U2, U3 – SN74LS06D or SN74LS06N**

Characteristics relevant to this design:

- 6 channels per package
- 5 V supply
- Open-collector output
- High sink-current capability
- TTL-compatible input threshold
- ESP32 3.3 V HIGH level is compatible with LS TTL input thresholds

Two packages provide 12 channels, leaving two spare channels.

## 7.3 Logic Convention

Because the SN74LS06 is an inverter:

```text
ESP GPIO LOW  -> LS06 transistor OFF -> drive line pulled HIGH -> signal deasserted
ESP GPIO HIGH -> LS06 transistor ON  -> drive line LOW         -> signal asserted
```

Firmware signal names shall use logical assertion semantics, not physical voltage semantics.

Example:

```text
motor_assert(true)  => GPIO10 HIGH => Motor On line LOW
motor_assert(false) => GPIO10 LOW  => Motor On line released HIGH
```

## 7.4 Safe Power-Up State

Every SN74LS06 input shall have a local pull-down resistor:

**10 kΩ to GND**

This guarantees that before ESP32 firmware configures GPIOs, all open-collector outputs remain OFF and the floppy signals remain deasserted.

This is especially important for:

- Write Gate
- Write Data
- Step
- Motor On

## 7.5 Driver Mapping

Suggested mapping:

### U2 SN74LS06

| Channel | ESP GPIO | Drive Signal |
|---|---:|---|
| 1 | GPIO4 | DS0 |
| 2 | GPIO5 | DS1 |
| 3 | GPIO6 | DS2 |
| 4 | GPIO7 | DS3 |
| 5 | GPIO10 | Motor On |
| 6 | GPIO11 | Direction |

### U3 SN74LS06

| Channel | ESP GPIO | Drive Signal |
|---|---:|---|
| 1 | GPIO12 | Step |
| 2 | GPIO13 | Write Data |
| 3 | GPIO14 via write interlock | Write Gate |
| 4 | GPIO15 | Side Select |
| 5 | Spare | Test pad |
| 6 | Spare | Test pad |

## 7.6 Output Series Components

No large series resistance shall be inserted in the low-side sink path because the legacy interface requires a solid low voltage.

Optional:

- 22–47 Ω small series resistance may be provisioned as a DNI footprint only if later signal-integrity testing warrants it.
- Default assembly: 0 Ω or direct connection.

---

# 8. Drive-to-ESP32 Input Stage

## 8.1 Inputs

The tester receives:

- Index
- Track 0
- Write Protect
- Read Data

These legacy drive outputs must not connect directly to ESP32 GPIO.

## 8.2 Termination

The tester shall provide pull-up/termination on drive output lines consistent with the original interface.

Provision:

**150 Ω from each drive-output signal to +5 V**

for:

- Index
- Track 0
- Write Protect
- Read Data

For experimentation with drives already containing unusual termination arrangements, each 150 Ω termination shall be individually removable via:

- jumper, or
- solder bridge, or
- resistor footprint that can be depopulated

Preferred:

```text
+5 V
 |
150 Ω
 |
Legacy signal ----> buffer input
 |
Drive open-collector output
```

Power dissipation when asserted:

```text
P ≈ 5² / 150 ≈ 0.167 W
```

Use at least:

**0.25 W resistors**

Prefer:

**0.5 W** for margin and cool operation.

## 8.3 Input Buffer

Use:

**U4 – SN74LVC14A**

Supply:

**3.3 V**

The SN74LVC14A provides:

- Schmitt-trigger inputs
- Inputs tolerant of voltages up to 5.5 V
- 3.3 V output compatible with ESP32
- Fast propagation appropriate for raw Read Data

Suggested mapping:

| U4 Channel | Legacy Input | ESP32 |
|---|---|---:|
| 1 | Index | GPIO16 |
| 2 | Track 0 | GPIO17 |
| 3 | Write Protect | GPIO18 |
| 4 | Read Data | GPIO21 |
| 5 | Spare external input | GPIO47 optional |
| 6 | Spare external input | GPIO48 optional |

Because U4 is an inverter:

```text
Legacy line HIGH -> ESP signal LOW
Legacy line LOW  -> ESP signal HIGH
```

This is convenient because active-low drive signals become active-high internal firmware signals.

Firmware shall define:

- INDEX_IN = 1 when Index asserted
- TRACK0_IN = 1 when Track 0 asserted
- WRITE_PROTECT_IN = 1 when Write Protect asserted
- READ_DATA_IN pulse HIGH when Read Data asserted

## 8.4 Input Protection

Each legacy input shall include:

- 100 Ω to 330 Ω series resistor between connector node and buffer input
- optional low-capacitance ESD protection footprint
- 100 pF maximum optional DNI capacitor for noise troubleshooting

Do not populate significant input capacitance on Read Data by default.

For Read Data:

- series resistor: 100 Ω preferred
- ESD device must be low capacitance
- no RC low-pass filter in the default build

---

# 9. Read Data Capture Path

## 9.1 Electrical Path

```text
J1 Pin 30
   |
 150 Ω pull-up to +5 V
   |
 100 Ω series
   |
SN74LVC14A
   |
3.3 V pulse
   |
GPIO21
```

## 9.2 ESP32 Peripheral Use

GPIO21 shall be routed so firmware can use:

- RMT receive
- pulse counter where appropriate
- general GPIO interrupt
- high-resolution timer timestamping

RMT or equivalent hardware capture is preferred over polling.

## 9.3 Bandwidth

The input circuitry shall support at least:

**1 MHz pulse activity**

without material pulse distortion.

This gives substantial margin over TM-100 FM/MFM rates and the approximately 1 µs read-data pulses documented by Tandon.

---

# 10. Index Input

Index shall use a normal SN74LVC14A channel.

The ESP32 shall timestamp both or one relevant edge as required.

At 300 RPM:

```text
300 rev/min / 60 = 5 rev/s
Index period ≈ 200 ms
```

Index measurement shall support at least:

- period
- pulse width
- RPM
- min/max period
- jitter

---

# 11. Track 0 and Write Protect Inputs

Track 0 and Write Protect use the same buffered input topology as Index.

The firmware shall sample them continuously.

Hardware shall ensure no input exceeds ESP32 supply voltage.

---

# 12. Physical Write Interlock

## 12.1 Requirement

A firmware bug must not be capable of asserting the drive's Write Gate unless the technician has physically enabled writing.

## 12.2 Recommended Implementation

Use a two-part write interlock:

1. Firmware signal `WRITE_GATE_CMD`
2. Physical front-panel switch `SW_WRITE_ENABLE`

Recommended circuit:

```text
GPIO14
  |
10k pull-down
  |
logic gate / transistor qualification
  |             SW_WRITE_ENABLE
  +------------- series hardware enable path
                |
            U3 LS06 input
                |
            Write Gate
```

The physical switch shall interrupt the ability to drive the LS06 Write Gate input HIGH.

Simplest acceptable implementation:

- SPST switch between GPIO14 and U3 input
- 10 kΩ pull-down on U3 input after the switch

Therefore:

Switch OFF:
- U3 input forced LOW
- Write Gate output cannot be asserted

Switch ON:
- GPIO14 can control U3 normally

## 12.3 Switch Sense

A second pole or auxiliary circuit shall report physical write-enable state to:

**GPIO39 – HW_WRITE_ENABLE_SENSE**

Recommended physical component:

**DPST or DPDT locking toggle switch**

One pole gates Write Gate hardware.  
One pole provides a 3.3 V logic sense.

## 12.4 Write Armed LED

LED_WRITE_ARMED shall illuminate only when:

- Physical switch enabled, AND
- Firmware write state armed

This should be implemented in firmware for Revision A.

A future revision may additionally implement hardware combinational indication.

---

# 13. USB-C Interface

## 13.1 USB Function

The tester shall use ESP32-S3 native USB.

Revision A expected use:

- USB Serial/JTAG or CDC console
- Firmware flashing
- Debugging

## 13.2 Connector

**J3 – USB-C receptacle, USB 2.0 device-only**

Required connections:

- D+ -> GPIO20
- D- -> GPIO19
- CC1 -> 5.1 kΩ to GND
- CC2 -> 5.1 kΩ to GND
- VBUS -> USB VBUS sense / optional 5 V source depending on power architecture
- Shield -> chassis/ground using intentional EMI connection strategy

## 13.3 USB Power Policy

The tester shall **not power the floppy drive from USB**.

Drive power comes from the external 12 V input.

The ESP32 electronics may optionally be powered from USB for firmware loading with the drive-power section off, provided reverse-current isolation is implemented.

Recommended architecture:

- External 12 V is normal operating source
- USB VBUS may power the 3.3 V logic only through an isolated OR-ing path
- No USB power shall feed the drive +5 V rail

## 13.4 USB ESD

Provide low-capacitance USB ESD protection on D+ and D- adjacent to J3.

---

# 14. Power Architecture

## 14.1 External Supply

Recommended:

**12 VDC regulated, 3 A minimum**

Input connector options:

- 5.5 mm × 2.1 mm locking barrel jack
- 2-pin locking connector
- XT30 for rugged bench use

Barrel jack is preferred for convenience.

Center positive if barrel jack is used.

## 14.2 Power Tree

```text
12 VDC INPUT
    |
   Fuse
    |
Reverse-polarity protection
    |
TVS / transient protection
    |
    +-----------------------------+
    |                             |
 INA226 #1                     5 V BUCK
    |                             |
 DRIVE +12 V                  INA226 #2
                                  |
                              DRIVE +5 V
                                  |
                             3.3 V regulator
                                  |
                              ESP32 / logic
```

The final schematic may place the 3.3 V regulator directly on the main 5 V logic rail before or after drive-current sensing depending on whether the desired +5 V current measurement should include tester electronics.

Preferred:

- INA226 #2 measures **drive +5 V only**
- tester logic branches before the drive +5 V current shunt

This produces measurements that represent the drive, not the tester.

---

# 15. Input Power Protection

Recommended protection stages:

1. Resettable fuse or conventional fuse:
   - 3 A to 4 A rating
2. Reverse-polarity MOSFET
3. TVS diode:
   - appropriate for nominal 12 V rail
4. Bulk capacitance:
   - 470 µF minimum near drive power output
5. Ceramic decoupling:
   - 1 µF and 100 nF distributed

The supply shall tolerate motor startup transients without resetting the ESP32.

---

# 16. +5 V Buck Converter

## 16.1 Recommended Device

**U5 – LMR51430**

Target:

- Input: nominal 12 V
- Output: 5.0 V
- Capacity: 3 A
- Switching frequency variant selected based on layout/EMI preference

A 3 A design gives adequate margin over the TM-100's approximately 600 mA average 5 V requirement and supports later compatibility with other drives.

## 16.2 Output Accuracy

Target +5 V output:

**5.00 V ±2% at drive connector**

This is tighter than the TM-100 allowable ±0.25 V.

## 16.3 Layout

Buck converter layout shall follow manufacturer hot-loop recommendations.

Keep switching node away from:

- Read Data
- Index
- 34-pin connector signal region
- ESP32 antenna area

---

# 17. +3.3 V Logic Supply

Recommended:

- 5 V to 3.3 V regulator rated at least 700 mA
- 1 A preferred

Possible device:

**AP2112K-3.3**, **TLV75533**, or equivalent.

If Wi-Fi/Bluetooth are not used, peak current remains manageable, but design margin is desirable.

Revision A firmware should disable unused radios unless specifically needed.

---

# 18. Drive Power Connector

## 18.1 J2 – 4-Pin Floppy Power

Pinout shall match Tandon documentation:

| Pin | Function |
|---:|---|
| 1 | +12 V |
| 2 | +12 V return |
| 3 | +5 V return |
| 4 | +5 V |

Use a standard 4-pin peripheral/floppy-compatible mating arrangement suitable for the actual TM-100 power connector/cable being used.

The original manual specifies separate returns. On the tester PCB both returns ultimately connect to the common power ground plane, but route them with sufficient copper and avoid forcing motor return current through sensitive logic-ground paths.

## 18.2 Wire Gauge

External drive-power harness should use:

**18 AWG preferred**

16–18 AWG is consistent with the original Tandon recommendation.

---

# 19. Rail Current and Voltage Monitoring

## 19.1 Monitor ICs

Use:

- **U6 – INA226**, +12 V drive rail
- **U7 – INA226**, +5 V drive rail

Both connect to the ESP32 via I2C.

Suggested addresses:

- U6: 0x40
- U7: 0x41

## 19.2 I2C Bus

- SDA = GPIO8
- SCL = GPIO9
- Pull-ups: 4.7 kΩ to 3.3 V
- Test pads on SDA/SCL

## 19.3 +12 V Shunt

Design target:

- Maximum expected measurement: 2 A
- Nominal: around 0.9 A

Recommended shunt:

**0.025 Ω, 1%, ≥0.5 W**

At 2 A:

```text
Vshunt = 50 mV
P = 0.1 W
```

This gives good measurement resolution with little voltage loss.

## 19.4 +5 V Shunt

Design target:

- Maximum expected measurement: 2 A
- Nominal: around 0.6 A

Recommended shunt:

**0.025 Ω, 1%, ≥0.5 W**

Same reasoning as +12 V.

Kelvin routing shall be used from each shunt to its INA226 inputs.

## 19.5 Alert Outputs

INA226 ALERT pins may be wire-ORed only if configured appropriately, or routed separately.

Preferred initial design:

- each ALERT through diode OR / open-drain arrangement to GPIO38
- individual fault source determined over I2C

Alternatively provide two separate GPIOs if routing permits.

---

# 20. Overcurrent Protection

Software current monitoring is not sufficient protection.

Each drive rail shall include hardware overcurrent protection.

Minimum acceptable Revision A:

- +12 V resettable fuse: approximately 1.5–2.0 A hold
- +5 V resettable fuse: approximately 1.5–2.0 A hold

Preferred later implementation:

- eFuse/load-switch device with programmable current limit
- independent fault output

The board shall survive a short on either drive-power output without relying on ESP32 response time.

---

# 21. Grounding

## 21.1 Ground Plane

Use a continuous ground plane.

Do not split digital and power ground into isolated islands.

Instead control current paths through placement and routing.

## 21.2 High-Current Returns

Route:

- motor +12 V return
- drive +5 V return

directly toward the power entry/current-sense region.

Do not route motor return beneath:

- Read Data buffer
- ESP32 crystal/module
- USB D+/D-
- analog current-sense Kelvin traces

## 21.3 Chassis Ground

Provide a dedicated chassis-ground lug/test point.

The original Tandon documentation recommends chassis earth grounding.

For a plastic tester enclosure:

- provide a binding post or chassis-ground terminal if bench earth connection is desired

For a metal enclosure:

- bond enclosure to protective/chassis ground
- connect circuit ground to chassis intentionally at a defined point rather than accidentally through multiple connectors

---

# 22. Decoupling

Each logic IC shall receive:

- 100 nF ceramic directly at VCC
- short ground return

Each LS06 package:

- 100 nF
- optional 1 µF nearby

ESP32 module:

- follow Espressif reference design
- provide local 10 µF plus 100 nF class decoupling on 3.3 V

5 V buffer rail:

- 10 µF bulk near LS06 drivers

Drive connector:

- +12 V: 220–470 µF bulk
- +5 V: 220–470 µF bulk
- additional ceramic capacitance

---

# 23. LEDs

Recommended indicators:

## LED1 – POWER
Indicates tester logic powered.

## LED2 – STATUS
Controlled by GPIO40.

## LED3 – FAULT
Controlled by GPIO41.

## LED4 – WRITE ARMED
Controlled by GPIO42.

Use approximately:

**1 kΩ to 2.2 kΩ**

series resistors depending on LED efficiency.

No LED shall load a floppy-interface signal directly.

---

# 24. Service Header

Provide a 1x6 service header:

| Pin | Function |
|---:|---|
| 1 | 3.3 V |
| 2 | GND |
| 3 | UART TX / GPIO43 |
| 4 | UART RX / GPIO44 |
| 5 | EN / Reset |
| 6 | GPIO0 / Boot |

GPIO0 shall not be permanently pulled into a boot-unsafe state.

Include standard ESP32 reset and boot buttons:

- RESET / EN
- BOOT / GPIO0

---

# 25. External Test Points

At minimum expose:

- TP1 GND
- TP2 +12 V input
- TP3 Drive +12 V
- TP4 Drive +5 V
- TP5 3.3 V
- TP6 DS0
- TP7 Motor On
- TP8 Direction
- TP9 Step
- TP10 Index
- TP11 Track 0
- TP12 Write Protect
- TP13 Read Data legacy side
- TP14 Read Data 3.3 V buffered side
- TP15 Write Data
- TP16 Write Gate
- TP17 I2C SDA
- TP18 I2C SCL

Read Data and Index test points should be ground-adjacent for oscilloscope probing.

Optional:

- SMA footprint for buffered Read Data
- SMA footprint for Index

Do not populate SMA connectors in the base BOM unless desired.

---

# 26. Timing Implementation Constraints

Hardware must permit firmware to meet these baseline timings:

## Step

- Pulse width: firmware target 2–10 µs
- Drive minimum: 200 ns
- Direction setup: firmware target ≥10 µs
- Direction hold: firmware target ≥10 µs
- Track-to-track interval: default 6 ms
- Absolute configurable minimum: 5 ms

The firmware targets are intentionally conservative relative to original minimum timing.

## Motor

- Enable motor
- Wait at least 300 ms before requiring stable speed
- Preferred default diagnostic spin-up delay: 500 ms

## Read Data

- Input path must preserve ~1 µs pulses
- Hardware capture resolution target: ≤100 ns
- Preferred timestamp resolution: 25–50 ns if convenient

---

# 27. Power-Up Hardware State

Before firmware executes:

- DS0-DS3 deasserted
- Motor deasserted
- Step deasserted
- Write Data deasserted
- Write Gate physically unable to assert unless hardware write switch enabled
- Side Select deasserted
- Direction defaults toward Track 0 or another documented safe state
- No drive motion occurs solely due to ESP32 reset

This is achieved using 10 kΩ pull-downs on all SN74LS06 inputs.

---

# 28. Reset Behavior

When ESP32 reset occurs:

1. Output-driver input pull-downs immediately return outputs to inactive state.
2. Motor line releases.
3. Drive Select lines release.
4. Step releases.
5. Write Gate releases.
6. Firmware logical track becomes invalid after reboot.
7. User must execute `home` before logical seek operations.

---

# 29. Write Gate Fail-Safe Analysis

The Write Gate line must remain deasserted under:

- ESP32 unpowered
- ESP32 bootloader
- firmware crash
- GPIO high-impedance
- USB disconnect
- physical write switch OFF

The schematic shall be reviewed specifically for single-fault accidental Write Gate assertion.

A hardware test shall verify this before any disk containing valuable data is connected.

---

# 30. PCB Requirements

## 30.1 Layer Count

Recommended:

**4-layer PCB**

Suggested stack:

1. Top – components/signals
2. Ground plane
3. Power/signals
4. Bottom – signals/components

A 2-layer board is possible but not preferred because the board combines:

- switching power
- high-current motor supply
- USB
- raw pulse capture
- legacy TTL interface

## 30.2 Placement Regions

Separate functional regions:

1. USB / ESP32
2. Input buffer / Read Data
3. Output drivers
4. Power conversion
5. Current sensing
6. Drive connectors

Read Data buffer should be located close to the 34-pin connector.

LS06 drivers should be located close to the 34-pin connector.

## 30.3 ESP32 Antenna

If using ESP32-S3-WROOM-1 with PCB antenna:

- follow Espressif antenna keepout
- place module at PCB edge
- no copper or components beneath antenna keepout

Although Wi-Fi is not required, the module should still be laid out correctly.

---

# 31. USB Layout

Route USB D+ and D- as a differential pair.

Target:

- approximately 90 Ω differential
- length matched reasonably
- short
- minimal vias
- ESD protection adjacent to connector

Avoid routing under buck converter switching node.

---

# 32. Read Data Layout

The raw Read Data path shall:

- be short
- avoid switching regulator region
- avoid motor-power traces
- avoid parallel routing with Step/Write Data over long distances
- have uninterrupted ground reference

Place:

1. connector
2. termination
3. series resistor
4. SN74LVC14A

close together.

---

# 33. Preliminary BOM

| Ref | Qty | Recommended Part | Function |
|---|---:|---|---|
| U1 | 1 | ESP32-S3-WROOM-1-N8 | MCU/module |
| U2-U3 | 2 | SN74LS06D | Open-collector drive outputs |
| U4 | 1 | SN74LVC14APW/D | 5 V-tolerant Schmitt input buffer |
| U5 | 1 | LMR51430 | 12 V to 5 V buck |
| U6-U7 | 2 | INA226 | Rail current/voltage monitor |
| U8 | 1 | TLV75533 / equivalent | 3.3 V regulator |
| J1 | 1 | 2x17 shrouded IDC header | 34-pin drive signal |
| J2 | 1 | 4-pin drive power connector | +12/+5 drive power |
| J3 | 1 | USB-C receptacle | Console/programming |
| J4 | 1 | 12 V input jack | External supply |
| J5 | 1 | 1x6 header | Service/debug |
| SW1 | 1 | Momentary switch | Reset |
| SW2 | 1 | Momentary switch | Boot |
| SW3 | 1 | DPDT locking toggle | Physical Write Enable |
| RTERM1-4 | 4 | 150 Ω, 0.5 W | Legacy input termination |
| ROUT_PD | 10 | 10 kΩ | LS06 safe-state pull-downs |
| RI2C | 2 | 4.7 kΩ | I2C pull-ups |
| RSH12 | 1 | 0.025 Ω, 1%, 0.5 W+ | +12 V current shunt |
| RSH5 | 1 | 0.025 Ω, 1%, 0.5 W+ | +5 V current shunt |
| RIN | 4 | 100 Ω | Legacy input series protection |
| LEDs | 4 | high-efficiency LED | Status |
| Fuse input | 1 | 3–4 A | Main protection |
| Fuse +12 | 1 | ~1.5–2 A PTC/fuse | Drive +12 protection |
| Fuse +5 | 1 | ~1.5–2 A PTC/fuse | Drive +5 protection |
| TVS1 | 1 | 12 V rail suitable TVS | Input transient protection |
| ESD_USB | 1 | low-C USB ESD array | USB protection |
| ESD_SIG | optional | low-C ESD parts | Drive interface protection |

Exact orderable manufacturer part numbers for passives, connectors, TVS, eFuse/PTC, USB ESD, and power magnetics shall be selected during schematic/BOM finalization.

---

# 34. Schematic Sheet Organization

Recommended schematic hierarchy:

## Sheet 1 – MCU / USB
- ESP32-S3
- USB-C
- reset/boot
- service header
- LEDs

## Sheet 2 – Floppy Output Interface
- LS06 drivers
- output pull-downs
- drive connector output nets
- write physical interlock

## Sheet 3 – Floppy Input Interface
- 150 Ω terminations
- SN74LVC14A
- input protection
- Read Data path
- test points

## Sheet 4 – Power
- 12 V input
- protection
- 5 V buck
- 3.3 V regulator
- drive connector

## Sheet 5 – Measurement
- INA226 devices
- shunts
- alert lines
- I2C

This organization should make review and troubleshooting straightforward.

---

# 35. Firmware / Hardware Net Naming

Recommended schematic and firmware names:

```text
FDD_DS0_N
FDD_DS1_N
FDD_DS2_N
FDD_DS3_N
FDD_MOTOR_N
FDD_DIR
FDD_STEP_N
FDD_WRITE_DATA_N
FDD_WRITE_GATE_N
FDD_SIDE_SELECT_N

FDD_INDEX_N
FDD_TRACK0_N
FDD_WP_N
FDD_READ_DATA_N

INDEX_ACTIVE
TRACK0_ACTIVE
WP_ACTIVE
READ_DATA_ACTIVE

DRIVE_12V
DRIVE_5V
SYS_5V
SYS_3V3
```

Suffix `_N` denotes an active-low external legacy signal.

Internal buffered firmware-visible signals should use positive logic.

---

# 36. Bring-Up Procedure

Initial board bring-up shall occur without a floppy drive attached.

## Phase 1 – Power

Verify:

- 12 V input polarity
- 5.00 V rail
- 3.3 V rail
- ESP32 boots
- USB enumerates
- no abnormal current

## Phase 2 – Output Drivers

Using dummy 150 Ω or suitable bench loads:

- verify each LS06 output releases HIGH
- verify each asserted output sinks LOW
- verify low voltage meets legacy requirement
- verify safe state during ESP32 reset

## Phase 3 – Input Buffers

Apply:

- 0 V
- 5 V

to each legacy input through appropriate source resistance.

Verify correct 3.3 V logic inversion.

## Phase 4 – Read Data

Inject test pulse train:

- 125 kHz
- 250 kHz
- 500 kHz

Verify pulse capture and timing accuracy.

## Phase 5 – Rail Monitoring

Apply known electronic loads.

Verify INA226 readings against bench DMM.

## Phase 6 – Known-Good Drive

Connect a known-good TM-100-1.

Test:

- DS
- Motor
- Index
- RPM
- Track 0
- Write Protect
- Step
- Home
- Seek
- Read activity

Do not enable physical write switch.

---

# 37. Validation Tests

## 37.1 Driver Sink Test

With representative 150 Ω / 5 V loading:

- commanded LOW shall meet required low voltage
- no driver shall exceed thermal limits

## 37.2 Reset Safety

Cycle:

- power
- reset
- USB connect/disconnect

Verify no Step or Write Gate glitches capable of causing unintended activity.

## 37.3 Motor Noise Immunity

Run motor while observing:

- 3.3 V rail
- USB console
- Read Data capture
- Index measurement

No ESP32 resets or USB dropouts allowed.

## 37.4 Read Data Integrity

Compare raw Read Data on:

- J1 legacy side
- buffered side
- ESP32 captured timing

against an oscilloscope.

## 37.5 RPM Accuracy

Compare ESP32 RPM calculation with oscilloscope measurement of Index period.

Target:

**≤0.1% tester measurement error**, excluding drive speed variation.

---

# 38. Open Items Before Schematic Release

The following shall be resolved before design release:

1. Exact USB-C receptacle footprint.
2. Exact 12 V power connector.
3. Exact TM-100 4-pin power mating connector strategy.
4. Exact input reverse-polarity MOSFET circuit.
5. Exact TVS voltage and part number.
6. Exact +12 V/+5 V fuse/PTC values after measuring startup current on several drives.
7. Exact LMR51430 inductor/capacitor values using TI design calculations.
8. Exact 3.3 V regulator.
9. Whether rail shutdown should be electronically controllable.
10. Whether the four 150 Ω input terminators should use jumpers or solder bridges.
11. Whether side-select hardware is populated in Revision A.
12. Whether spare LS06 channels are routed to an expansion header.
13. Whether buffered Read Data gets an SMA footprint.
14. Final PCB/enclosure dimensions.

---

# 39. Recommended Revision A Decisions

Unless bench testing provides a reason to change them, proceed with:

- ESP32-S3-WROOM-1-N8
- Two SN74LS06 output-driver ICs
- One SN74LVC14A input-buffer IC
- Four removable 150 Ω / 0.5 W terminators
- Two INA226 monitors
- 0.025 Ω current shunts
- LMR51430 5 V / 3 A buck
- 12 V / 3 A external adapter
- 4-layer PCB
- physical DPDT Write Enable switch
- no magnetic writing enabled in first firmware release
- native USB command console
- 6 ms default Step interval
- 500 ms default motor stabilization wait
- Track 39 default maximum for TM-100-1

---

# 40. Reference Documentation

Design assumptions in this document are based primarily on:

1. **Tandon TM100-1 / TM100-2 Product Specification and User's Manual**  
   Original Tandon documentation includes interface pin assignments, 150 Ω interface termination example, power connector, supply requirements, and control/data timing.  
   Public archive copy:  
   https://www.oldsilicon.com/portables/kaypro-ii/docs/Tandon_TM-100-1_Flexible_Disk_Drive.pdf

2. **Tandon TM-100-1 / TM-100-2 OEM Operating and Service Manual**  
   Public archive copy:  
   https://www.bitsavers.org/pdf/tandon/TM100-1_-2_OEM_Operating_and_Service_Manual_48TPI_1981.pdf

3. **Espressif ESP32-S3-WROOM-1 / WROOM-1U Datasheet**  
   https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf

4. **Texas Instruments SN74LS06**  
   Open-collector legacy-interface driver.  
   https://www.ti.com/product/SN74LS06

5. **Texas Instruments SN74LVC14A**  
   Schmitt-trigger buffer with over-voltage-tolerant inputs.  
   https://www.ti.com/product/SN74LVC14A

6. **Texas Instruments INA226**  
   36 V, 16-bit current/voltage/power monitor.  
   https://www.ti.com/product/INA226

7. **Texas Instruments LMR51430**  
   4.5–36 V, 3 A synchronous buck converter.  
   https://www.ti.com/product/LMR51430

---

# 41. Design Maturity Note

This document is an engineering hardware design specification, not yet a released production schematic.

Before fabrication:

- schematic capture shall be completed,
- component datasheets shall be checked against the exact selected package/ordering code,
- power-converter compensation/passives shall be calculated from the selected regulator reference design,
- ERC shall be run,
- PCB power and current paths shall be reviewed,
- Write Gate fail-safe behavior shall be independently reviewed,
- and the completed schematic shall be peer-reviewed against the original Tandon interface documentation.

The next deliverable should be the **complete schematic design**, followed by the finalized BOM and PCB layout constraints.
