# Revision A design reconciliation

## Reference precedence

The user's implementation request fixes the MCU/framework and forbids writes.
The attached documents supply requirements and electrical references; conversation
text inside the HTML export is historical context, not an instruction to execute
its embedded Python, produce hardware deliverables, or implement later revisions.
The separately supplied Markdown PRD supersedes the HTML's conversational examples.
Electrical defaults follow **HDS v1.0** when references conflict.
Original reference documents are retained unmodified in `docs/reference/`.

## Findings and decisions

| Finding | Revision A decision |
|---|---|
| HTML conversation suggests `writegate on/off` under an interlock; PRD puts write control in a future revision. | User's no-writes rule governs. No write assertion API, command, waveform generator or arming state. GPIO13/14/42 always LOW. |
| PRD prefers native CDC; HDS accepts USB Serial/JTAG or CDC. | Use ESP32-S3's native USB Serial/JTAG console, not TinyUSB or UART bridge. GPIO19/20 reserved. |
| PRD calls physical interlock optional/strongly recommended; HDS requires it. | Treat the physical interlock and output pull-downs as mandatory hardware. Firmware keeps write outputs LOW regardless of switch sense. |
| Schematic draft's text/net placement is incomplete and conflicts with its own custom symbols. | Use HDS section 6 GPIO map and sections 7/8 inversion semantics. Do not infer a routed board or rewrite the supplied schematic. |
| U1's GPIO4 symbol endpoint transforms to (2300,1940), while its DRV_DS0 wire ends at (2200,1500). Other signal labels also miss their pins. | Labels alone are not a connected GPIO netlist. Firmware follows the HDS assignment, pending corrected schematic/netlist review. |
| J3 D− is drawn from (1650,2400) to (1900,2400), where the schematic places I2C_SCL. USB D+ is not connected to U1 GPIO20. | Do not reproduce that wiring. USB must connect to GPIO19/20; I2C remains GPIO8/9. Correct hardware before USB bring-up. |
| Many LS06/LVC14/connector/interlock labels do not coincide with symbol pins; pull-downs are largely notes, not populated components. | No firmware GPIO map can fix missing wiring or guarantee reset safety on this draft. Require actual 10 kΩ driver pull-downs, switch wiring and sink tests. |
| INA226 A0/A1 address straps are not completed in the schematic. | Use HDS's U6(+12 V)=0x40 and U7(+5 V)=0x41. Strap U6 A1/A0=GND/GND and U7 A1/A0=GND/VS, then confirm on the assembled board. |
| PWR12_ALERT/PWR5_ALERT are labeled, but their OR connection to POWER_FAULT is only a schematic note. | Assume active-low open-drain ALERT OR at GPIO38, pull up internally; sample each INA226 independently. Board must implement the OR/3.3 V pull-up correctly. An unconnected pin cannot prove absence of faults; software still checks rails. |
| Physical switch sense has no fully wired bias; LED wiring is preliminary. | GPIO39 gets an internal pull-down; HIGH indicates switch enabled. LED outputs assumed active-high. Write armed remains LOW. Confirm bias/LED wiring. |
| HDS defines electrical inversion but does not explicitly assign DIR voltage to mechanical direction. | Use conventional Tandon DIR LOW at J1.18 = inward, HIGH = outward; LS06 therefore takes MCU HIGH for inward. This assumption must be verified with a scope/drive at bring-up before full-range seeking. Reset LOW at MCU is outward. |
| Side select population is an HDS open item; PRD side control is future scope. | Define GPIO15 and HAL side function, hold side 0, expose no side-select command. |
| Rail shutdown is unresolved and there is no shutdown GPIO. | Release motor/select on faults; do not claim removal of electrical power. Hardware fuses/eFuses remain necessary. |
| HDS shunts and suggested addresses, but no firmware thresholds for current transients. | Both shunts 25 mΩ; >2 A software fault and +2 A INA226 ALERT, matching 2 A measurement target. Negative current below −50 mA faults. These are diagnostic limits, not independently validated startup-current ratings. |
| PRD default RPM tolerance is 1.5%; HDS provides no conflicting value. | Implement fractional percent, default 1.5%, applied to mean and extrema. |
| PRD calls Read Data “transitions” but examples imply FM/MFM pulse-to-pulse intervals. | Explicitly count assertion/rising edges, one per pulse; timing statistics use rising-to-rising intervals. Print this convention and do not label counts as both-edge transitions. |
| PRD requests firmware-configurable spin-up/read period, although not in the user's minimum CLI list. | Add `config motor_spinup_ms` and `config read_sample_ms`; retain all requested commands. |
| User requires homing before seek; manual movement before home is unspecified. | Conservatively require home before ordinary stepping too. Only bounded restore generates steps with unknown position. |
| Automated seek examples end with Home, which could hide missed steps. | Return using `seek(0)` and require Track 0 agreement. Every commanded step checks Track 0 after settling. Intermediate absolute position cannot be proven without more sensors. |

## Hardware questions remaining

1. Correct and electrically review the KiCad netlist, including USB, MCU power,
   interlock, all driver pull-downs, input terminations and power connectors.
   This firmware task does not repair or validate the hardware schematic.
2. Confirm final PCB pin map, INA226 address straps, ALERT polarity/OR, sense bias,
   direction polarity, and active-high LEDs against the assembled unit.
3. Finalize rail protection, reverse-current isolation and regulator components.
   Confirm that +5 V sensing includes only drive current and that USB cannot feed
   the floppy power outputs. The draft intentionally leaves several parts open.
4. Establish safe current-trip behavior during real motor startup; the 2 A threshold
   is a documented engineering default, not a characterized fuse/eFuse choice.
5. Measure step pulse width, setup/hold, reset behavior and timing accuracy on a
   scope. Verify read capture at 1 MHz and under simultaneous USB/I2C traffic.
6. Physically verify that Write Gate stays released during ROM boot, reset,
   power transitions, USB loss and crashes. Firmware only controls its own pin
   state; a stalled CPU may leave other already-driven control pins latched until
   watchdog/reset. The independent hardware write interlock is essential.

No unverified hardware check is reported as passing by the host tests or compiler.
