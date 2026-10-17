EESchema Schematic File Version 4
LIBS:TM100_Tester
EELAYER 29 0
EELAYER END
$Descr A2 23386 16535
Sheet 1 1
Title "TM-100 ESP32-S3 Floppy Drive Diagnostic Tester"
Date "2026-10-16"
Rev "A / Design Draft 0.1"
Comp "Tandon Drive Tester Project"
Comment1 "Non-destructive Revision A; Write Gate physically interlocked"
Comment2 "Open in KiCad and convert to modern .kicad_sch before layout"
Comment3 "Verify exact footprints and power parts before fabrication"
Comment4 ""
$EndDescr
Text Notes 700 600 0    90   ~ 12
ESP32-S3 / USB
Text Notes 7800 600 0    90   ~ 12
LEGACY FLOPPY INTERFACE
Text Notes 700 8300 0    90   ~ 12
POWER / RAIL MONITORING
Text Notes 14200 8300 0    90   ~ 12
DRIVE CONNECTORS / TEST POINTS
$Comp
L TM100_Tester:ESP32_S3_WROOM U1
U 1 1 00000001
P 3200 2700
F 0 "U1" H 3300 2800 50  0000 C CNN
F 1 "ESP32-S3-WROOM-1-N8" H 3350 2600 50  0000 C CNN
F 2 "" H 3200 2700 50  0001 C CNN
	1    3200 2700
	1    0    0    -1
$EndComp
Text GLabel 1900 1500 0    40   Output ~ 0
DRV_DS0
Wire Wire Line
	1900 1500 2200 1500
Text GLabel 1900 1680 0    40   Output ~ 0
DRV_DS1
Wire Wire Line
	1900 1680 2200 1680
Text GLabel 1900 1860 0    40   Output ~ 0
DRV_DS2
Wire Wire Line
	1900 1860 2200 1860
Text GLabel 1900 2040 0    40   Output ~ 0
DRV_DS3
Wire Wire Line
	1900 2040 2200 2040
Text GLabel 1900 2220 0    40   BiDi ~ 0
I2C_SDA
Wire Wire Line
	1900 2220 2200 2220
Text GLabel 1900 2400 0    40   BiDi ~ 0
I2C_SCL
Wire Wire Line
	1900 2400 2200 2400
Text GLabel 1900 2580 0    40   Output ~ 0
DRV_MOTOR
Wire Wire Line
	1900 2580 2200 2580
Text GLabel 1900 2760 0    40   Output ~ 0
DRV_DIR
Wire Wire Line
	1900 2760 2200 2760
Text GLabel 1900 2940 0    40   Output ~ 0
DRV_STEP
Wire Wire Line
	1900 2940 2200 2940
Text GLabel 1900 3120 0    40   Output ~ 0
DRV_WRITE_DATA
Wire Wire Line
	1900 3120 2200 3120
Text GLabel 1900 3300 0    40   Output ~ 0
WRITE_GATE_CMD
Wire Wire Line
	1900 3300 2200 3300
Text GLabel 1900 3480 0    40   Output ~ 0
DRV_SIDE
Wire Wire Line
	1900 3480 2200 3480
Text GLabel 4500 1550 0    40   Input ~ 0
INDEX_ACTIVE
Text GLabel 4500 1760 0    40   Input ~ 0
TRACK0_ACTIVE
Text GLabel 4500 1970 0    40   Input ~ 0
WP_ACTIVE
Text GLabel 4500 2180 0    40   Input ~ 0
READ_DATA_ACTIVE
Text GLabel 4500 2390 0    40   Input ~ 0
POWER_FAULT
Text GLabel 4500 2600 0    40   Input ~ 0
HW_WRITE_ENABLE
Text GLabel 4500 2810 0    40   Output ~ 0
LED_STATUS
Text GLabel 4500 3020 0    40   Output ~ 0
LED_FAULT
Text GLabel 4500 3230 0    40   Output ~ 0
LED_WRITE_ARMED
$Comp
L TM100_Tester:USB_C_DEVICE J3
U 1 1 00000002
P 1100 2600
F 0 "J3" H 1200 2700 50  0000 C CNN
F 1 "USB-C DEVICE" H 1250 2500 50  0000 C CNN
F 2 "" H 1100 2600 50  0001 C CNN
	1    1100 2600
	1    0    0    -1
$EndComp
Text Label 1650 2400 0    40   ~ 0
USB_D-
Text Label 1650 2200 0    40   ~ 0
USB_D+
Wire Wire Line
	1650 2400 1900 2400
Wire Wire Line
	1650 2200 1900 2200
$Comp
L TM100_Tester:R_US R1
U 1 1 00000003
P 1800 2700
F 0 "R1" H 1900 2800 50  0000 C CNN
F 1 "5.1k CC1" H 1950 2600 50  0000 C CNN
F 2 "" H 1800 2700 50  0001 C CNN
	1    1800 2700
	1    0    0    -1
$EndComp
$Comp
L TM100_Tester:R_US R2
U 1 1 00000004
P 1800 2900
F 0 "R2" H 1900 3000 50  0000 C CNN
F 1 "5.1k CC2" H 1950 2800 50  0000 C CNN
F 2 "" H 1800 2900 50  0001 C CNN
	1    1800 2900
	1    0    0    -1
$EndComp
Text GLabel 2200 2700 0    40   Power ~ 0
GND
Text GLabel 2200 2900 0    40   Power ~ 0
GND
Text Notes 650 3400 0    50   ~ 12
USB VBUS powers logic only through protected OR path; never powers the floppy drive.
$Comp
L TM100_Tester:SN74LS06 U2
U 1 1 00000005
P 8500 2200
F 0 "U2" H 8600 2300 50  0000 C CNN
F 1 "SN74LS06 - DS/MOTOR/DIR" H 8650 2100 50  0000 C CNN
F 2 "" H 8500 2200 50  0001 C CNN
	1    8500 2200
	1    0    0    -1
$EndComp
$Comp
L TM100_Tester:SN74LS06 U3
U 1 1 00000006
P 8500 4700
F 0 "U3" H 8600 4800 50  0000 C CNN
F 1 "SN74LS06 - STEP/WRITE/SIDE" H 8650 4600 50  0000 C CNN
F 2 "" H 8500 4700 50  0001 C CNN
	1    8500 4700
	1    0    0    -1
$EndComp
Text GLabel 7300 1600 0    40   Input ~ 0
DRV_DS0
Text GLabel 7300 1800 0    40   Input ~ 0
DRV_DS1
Text GLabel 7300 2000 0    40   Input ~ 0
DRV_DS2
Text GLabel 7300 2200 0    40   Input ~ 0
DRV_DS3
Text GLabel 7300 2400 0    40   Input ~ 0
DRV_MOTOR
Text GLabel 7300 2600 0    40   Input ~ 0
DRV_DIR
Text GLabel 9700 1600 0    40   Output ~ 0
FDD_DS0_N
Text GLabel 9700 1800 0    40   Output ~ 0
FDD_DS1_N
Text GLabel 9700 2000 0    40   Output ~ 0
FDD_DS2_N
Text GLabel 9700 2200 0    40   Output ~ 0
FDD_DS3_N
Text GLabel 9700 2400 0    40   Output ~ 0
FDD_MOTOR_N
Text GLabel 9700 2600 0    40   Output ~ 0
FDD_DIR_N
Text GLabel 7300 4100 0    40   Input ~ 0
DRV_STEP
Text GLabel 7300 4320 0    40   Input ~ 0
DRV_WRITE_DATA
Text GLabel 7300 4540 0    40   Input ~ 0
WRITE_GATE_HW
Text GLabel 7300 4760 0    40   Input ~ 0
DRV_SIDE
Text GLabel 9700 4100 0    40   Output ~ 0
FDD_STEP_N
Text GLabel 9700 4320 0    40   Output ~ 0
FDD_WRITE_DATA_N
Text GLabel 9700 4540 0    40   Output ~ 0
FDD_WRITE_GATE_N
Text GLabel 9700 4760 0    40   Output ~ 0
FDD_SIDE_N
Text GLabel 8500 1250 0    40   Power ~ 0
SYS_5V
Text GLabel 8500 3750 0    40   Power ~ 0
SYS_5V
Text GLabel 8500 3150 0    40   Power ~ 0
GND
Text GLabel 8500 5650 0    40   Power ~ 0
GND
Text Notes 7600 5850 0    45   ~ 12
10k pull-downs on every LS06 input are required for a safe reset state.
Use resistor array(s) in final schematic/layout.
$Comp
L TM100_Tester:SW_DPDT SW3
U 1 1 00000007
P 6500 4850
F 0 "SW3" H 6600 4950 50  0000 C CNN
F 1 "PHYSICAL WRITE ENABLE" H 6650 4750 50  0000 C CNN
F 2 "" H 6500 4850 50  0001 C CNN
	1    6500 4850
	1    0    0    -1
$EndComp
Text GLabel 5900 4700 0    40   Input ~ 0
WRITE_GATE_CMD
Text GLabel 7100 4700 0    40   Output ~ 0
WRITE_GATE_HW
$Comp
L TM100_Tester:R_US R20
U 1 1 00000008
P 7000 5200
F 0 "R20" H 7100 5300 50  0000 C CNN
F 1 "10k PULLDOWN" H 7150 5100 50  0000 C CNN
F 2 "" H 7000 5200 50  0001 C CNN
	1    7000 5200
	1    0    0    -1
$EndComp
Text GLabel 7450 5200 0    40   Power ~ 0
GND
Text GLabel 5900 5000 0    40   Power ~ 0
SYS_3V3
Text GLabel 7100 5000 0    40   Output ~ 0
HW_WRITE_ENABLE
Text Notes 5750 5550 0    45   ~ 12
Pole 1 gates Write Gate command. Pole 2 reports physical enable state.
With SW3 OFF, LS06 Write Gate input is held LOW and cannot assert.
$Comp
L TM100_Tester:SN74LVC14A U4
U 1 1 00000009
P 12300 2900
F 0 "U4" H 12400 3000 50  0000 C CNN
F 1 "SN74LVC14A @ 3.3V" H 12450 2800 50  0000 C CNN
F 2 "" H 12300 2900 50  0001 C CNN
	1    12300 2900
	1    0    0    -1
$EndComp
Text GLabel 12300 1950 0    40   Power ~ 0
SYS_3V3
Text GLabel 12300 3850 0    40   Power ~ 0
GND
Text GLabel 11100 2300 0    40   Input ~ 0
FDD_INDEX_N_TERM
Text GLabel 11100 2500 0    40   Input ~ 0
FDD_TRACK0_N_TERM
Text GLabel 11100 2700 0    40   Input ~ 0
FDD_WP_N_TERM
Text GLabel 11100 2900 0    40   Input ~ 0
FDD_READ_DATA_N_TERM
Text GLabel 13500 2300 0    40   Output ~ 0
INDEX_ACTIVE
Text GLabel 13500 2500 0    40   Output ~ 0
TRACK0_ACTIVE
Text GLabel 13500 2700 0    40   Output ~ 0
WP_ACTIVE
Text GLabel 13500 2900 0    40   Output ~ 0
READ_DATA_ACTIVE
Text Notes 11100 4100 0    45   ~ 12
SN74LVC14A powered at 3.3V; inputs tolerate 5V.
Read Data channel uses 100Ω series resistor and NO filter capacitor by default.
Text GLabel 10900 4700 0    40   Input ~ 0
FDD_INDEX_N
$Comp
L TM100_Tester:R_US R30
U 1 1 0000000A
P 11600 4530
F 0 "R30" H 11700 4630 50  0000 C CNN
F 1 "150R 0.5W TERM" H 11750 4430 50  0000 C CNN
F 2 "" H 11600 4530 50  0001 C CNN
	1    11600 4530
	1    0    0    -1
$EndComp
Text GLabel 11150 4530 0    40   Power ~ 0
SYS_5V
$Comp
L TM100_Tester:R_US R40
U 1 1 0000000B
P 12100 4700
F 0 "R40" H 12200 4800 50  0000 C CNN
F 1 "100R" H 12250 4600 50  0000 C CNN
F 2 "" H 12100 4700 50  0001 C CNN
	1    12100 4700
	1    0    0    -1
$EndComp
Text GLabel 12700 4700 0    40   Output ~ 0
FDD_INDEX_N_TERM
Text Notes 11200 4830 0    32   ~ 12
TERM removable via solder bridge/jumper footprint
Text GLabel 10900 5120 0    40   Input ~ 0
FDD_TRACK0_N
$Comp
L TM100_Tester:R_US R31
U 1 1 0000000C
P 11600 4950
F 0 "R31" H 11700 5050 50  0000 C CNN
F 1 "150R 0.5W TERM" H 11750 4850 50  0000 C CNN
F 2 "" H 11600 4950 50  0001 C CNN
	1    11600 4950
	1    0    0    -1
$EndComp
Text GLabel 11150 4950 0    40   Power ~ 0
SYS_5V
$Comp
L TM100_Tester:R_US R41
U 1 1 0000000D
P 12100 5120
F 0 "R41" H 12200 5220 50  0000 C CNN
F 1 "100R" H 12250 5020 50  0000 C CNN
F 2 "" H 12100 5120 50  0001 C CNN
	1    12100 5120
	1    0    0    -1
$EndComp
Text GLabel 12700 5120 0    40   Output ~ 0
FDD_TRACK0_N_TERM
Text Notes 11200 5250 0    32   ~ 12
TERM removable via solder bridge/jumper footprint
Text GLabel 10900 5540 0    40   Input ~ 0
FDD_WP_N
$Comp
L TM100_Tester:R_US R32
U 1 1 0000000E
P 11600 5370
F 0 "R32" H 11700 5470 50  0000 C CNN
F 1 "150R 0.5W TERM" H 11750 5270 50  0000 C CNN
F 2 "" H 11600 5370 50  0001 C CNN
	1    11600 5370
	1    0    0    -1
$EndComp
Text GLabel 11150 5370 0    40   Power ~ 0
SYS_5V
$Comp
L TM100_Tester:R_US R42
U 1 1 0000000F
P 12100 5540
F 0 "R42" H 12200 5640 50  0000 C CNN
F 1 "100R" H 12250 5440 50  0000 C CNN
F 2 "" H 12100 5540 50  0001 C CNN
	1    12100 5540
	1    0    0    -1
$EndComp
Text GLabel 12700 5540 0    40   Output ~ 0
FDD_WP_N_TERM
Text Notes 11200 5670 0    32   ~ 12
TERM removable via solder bridge/jumper footprint
Text GLabel 10900 5960 0    40   Input ~ 0
FDD_READ_DATA_N
$Comp
L TM100_Tester:R_US R33
U 1 1 00000010
P 11600 5790
F 0 "R33" H 11700 5890 50  0000 C CNN
F 1 "150R 0.5W TERM" H 11750 5690 50  0000 C CNN
F 2 "" H 11600 5790 50  0001 C CNN
	1    11600 5790
	1    0    0    -1
$EndComp
Text GLabel 11150 5790 0    40   Power ~ 0
SYS_5V
$Comp
L TM100_Tester:R_US R43
U 1 1 00000011
P 12100 5960
F 0 "R43" H 12200 6060 50  0000 C CNN
F 1 "100R" H 12250 5860 50  0000 C CNN
F 2 "" H 12100 5960 50  0001 C CNN
	1    12100 5960
	1    0    0    -1
$EndComp
Text GLabel 12700 5960 0    40   Output ~ 0
FDD_READ_DATA_N_TERM
Text Notes 11200 6090 0    32   ~ 12
TERM removable via solder bridge/jumper footprint
$Comp
L TM100_Tester:FDD_34 J1
U 1 1 00000012
P 17000 3000
F 0 "J1" H 17100 3100 50  0000 C CNN
F 1 "34-PIN SHUGART/TANDON" H 17150 2900 50  0000 C CNN
F 2 "" H 17000 3000 50  0001 C CNN
	1    17000 3000
	1    0    0    -1
$EndComp
Text Notes 15900 1150 0    70   ~ 12
J1 DRIVE SIGNAL - NO PC CABLE TWIST
Text GLabel 19000 1600 0    40   BiDi ~ 0
FDD_DS3_N
Text Notes 18300 1565 0    32   ~ 12
J1 pin 6
Text GLabel 19000 1850 0    40   BiDi ~ 0
FDD_INDEX_N
Text Notes 18300 1815 0    32   ~ 12
J1 pin 8
Text GLabel 19000 2100 0    40   BiDi ~ 0
FDD_DS0_N
Text Notes 18300 2065 0    32   ~ 12
J1 pin 10
Text GLabel 19000 2350 0    40   BiDi ~ 0
FDD_DS1_N
Text Notes 18300 2315 0    32   ~ 12
J1 pin 12
Text GLabel 19000 2600 0    40   BiDi ~ 0
FDD_DS2_N
Text Notes 18300 2565 0    32   ~ 12
J1 pin 14
Text GLabel 19000 2850 0    40   BiDi ~ 0
FDD_MOTOR_N
Text Notes 18300 2815 0    32   ~ 12
J1 pin 16
Text GLabel 19000 3100 0    40   BiDi ~ 0
FDD_DIR_N
Text Notes 18300 3065 0    32   ~ 12
J1 pin 18
Text GLabel 19000 3350 0    40   BiDi ~ 0
FDD_STEP_N
Text Notes 18300 3315 0    32   ~ 12
J1 pin 20
Text GLabel 19000 3600 0    40   BiDi ~ 0
FDD_WRITE_DATA_N
Text Notes 18300 3565 0    32   ~ 12
J1 pin 22
Text GLabel 19000 3850 0    40   BiDi ~ 0
FDD_WRITE_GATE_N
Text Notes 18300 3815 0    32   ~ 12
J1 pin 24
Text GLabel 19000 4100 0    40   BiDi ~ 0
FDD_TRACK0_N
Text Notes 18300 4065 0    32   ~ 12
J1 pin 26
Text GLabel 19000 4350 0    40   BiDi ~ 0
FDD_WP_N
Text Notes 18300 4315 0    32   ~ 12
J1 pin 28
Text GLabel 19000 4600 0    40   BiDi ~ 0
FDD_READ_DATA_N
Text Notes 18300 4565 0    32   ~ 12
J1 pin 30
Text GLabel 19000 4850 0    40   BiDi ~ 0
FDD_SIDE_N
Text Notes 18300 4815 0    32   ~ 12
J1 pin 32
Text Notes 15900 5000 0    45   ~ 12
All odd-numbered signal-return pins (1,3,5...33) connect to GND plane.
Pins 2,4,34 reserved/NC for Revision A.
$Comp
L TM100_Tester:CONN_4 J2
U 1 1 00000013
P 17100 9100
F 0 "J2" H 17200 9200 50  0000 C CNN
F 1 "TM-100 DRIVE POWER" H 17250 9000 50  0000 C CNN
F 2 "" H 17100 9100 50  0001 C CNN
	1    17100 9100
	1    0    0    -1
$EndComp
Text GLabel 16400 8800 0    40   Power ~ 0
DRIVE_12V
Text GLabel 16400 9000 0    40   Power ~ 0
GND
Text GLabel 16400 9200 0    40   Power ~ 0
GND
Text GLabel 16400 9400 0    40   Power ~ 0
DRIVE_5V
Text Notes 17700 9000 0    40   ~ 12
Pin 1 +12V
Pin 2 +12V return
Pin 3 +5V return
Pin 4 +5V
$Comp
L TM100_Tester:CONN_2 J4
U 1 1 00000014
P 1200 9300
F 0 "J4" H 1300 9400 50  0000 C CNN
F 1 "12VDC INPUT 3A+" H 1350 9200 50  0000 C CNN
F 2 "" H 1200 9300 50  0001 C CNN
	1    1200 9300
	1    0    0    -1
$EndComp
$Comp
L TM100_Tester:FUSE F1
U 1 1 00000015
P 2200 9200
F 0 "F1" H 2300 9300 50  0000 C CNN
F 1 "3.15A INPUT" H 2350 9100 50  0000 C CNN
F 2 "" H 2200 9200 50  0001 C CNN
	1    2200 9200
	1    0    0    -1
$EndComp
Text GLabel 2800 9200 0    40   Output ~ 0
VIN_12_PROT
Text GLabel 700 9400 0    40   Power ~ 0
GND
$Comp
L TM100_Tester:D_TVS D1
U 1 1 00000016
P 2700 9800
F 0 "D1" H 2800 9900 50  0000 C CNN
F 1 "SMBJ15A TVS (verify)" H 2850 9700 50  0000 C CNN
F 2 "" H 2700 9800 50  0001 C CNN
	1    2700 9800
	1    0    0    -1
$EndComp
Text GLabel 2200 9800 0    40   BiDi ~ 0
VIN_12_PROT
Text GLabel 3200 9800 0    40   Power ~ 0
GND
Text Notes 700 10150 0    42   ~ 12
Add ideal-diode/reverse-polarity P-MOSFET stage between J4 and VIN_12_PROT in final schematic.
Part selection remains an open item in HDS v1.0.
$Comp
L TM100_Tester:R_US R50
U 1 1 00000017
P 4200 9000
F 0 "R50" H 4300 9100 50  0000 C CNN
F 1 "0.025R 1% 0.5W" H 4350 8900 50  0000 C CNN
F 2 "" H 4200 9000 50  0001 C CNN
	1    4200 9000
	1    0    0    -1
$EndComp
Text GLabel 3700 9000 0    40   Power ~ 0
VIN_12_PROT
Text GLabel 4700 9000 0    40   Power ~ 0
DRIVE_12V
$Comp
L TM100_Tester:INA226 U6
U 1 1 00000018
P 5200 9700
F 0 "U6" H 5300 9800 50  0000 C CNN
F 1 "INA226 +12V MON" H 5350 9600 50  0000 C CNN
F 2 "" H 5200 9700 50  0001 C CNN
	1    5200 9700
	1    0    0    -1
$EndComp
Text GLabel 4550 10200 0    40   Input ~ 0
VIN_12_PROT
Text GLabel 5850 10200 0    40   Input ~ 0
DRIVE_12V
Text GLabel 5850 9950 0    40   Input ~ 0
DRIVE_12V
Text GLabel 5200 8850 0    40   Power ~ 0
SYS_3V3
Text GLabel 5200 10550 0    40   Power ~ 0
GND
Text GLabel 5850 9450 0    40   BiDi ~ 0
I2C_SDA
Text GLabel 5850 9700 0    40   Input ~ 0
I2C_SCL
Text GLabel 5850 9200 0    40   Output ~ 0
PWR12_ALERT
Text Notes 4050 10850 0    40   ~ 12
Kelvin-route INA226 IN+/IN- directly to shunt pads.
$Comp
L TM100_Tester:LMR51430 U5
U 1 1 00000019
P 7700 9250
F 0 "U5" H 7800 9350 50  0000 C CNN
F 1 "LMR51430 500kHz" H 7850 9150 50  0000 C CNN
F 2 "" H 7700 9250 50  0001 C CNN
	1    7700 9250
	1    0    0    -1
$EndComp
Text GLabel 7050 8850 0    40   Power ~ 0
VIN_12_PROT
Text GLabel 7050 9150 0    40   Input ~ 0
VIN_12_PROT
Text GLabel 7700 10000 0    40   Power ~ 0
GND
$Comp
L TM100_Tester:C C10
U 1 1 0000001A
P 8700 8600
F 0 "C10" H 8800 8700 50  0000 C CNN
F 1 "0.1uF BOOT" H 8850 8500 50  0000 C CNN
F 2 "" H 8700 8600 50  0001 C CNN
	1    8700 8600
	1    0    0    -1
$EndComp
$Comp
L TM100_Tester:L L1
U 1 1 0000001B
P 9100 8850
F 0 "L1" H 9200 8950 50  0000 C CNN
F 1 "6.8uH, >=4A RMS, >=5A SAT" H 9250 8750 50  0000 C CNN
F 2 "" H 9100 8850 50  0001 C CNN
	1    9100 8850
	1    0    0    -1
$EndComp
Text GLabel 9800 8850 0    40   Output ~ 0
SYS_5V_RAW
$Comp
L TM100_Tester:R_US R60
U 1 1 0000001C
P 9000 9450
F 0 "R60" H 9100 9550 50  0000 C CNN
F 1 "100k 1%" H 9150 9350 50  0000 C CNN
F 2 "" H 9000 9450 50  0001 C CNN
	1    9000 9450
	1    0    0    -1
$EndComp
$Comp
L TM100_Tester:R_US R61
U 1 1 0000001D
P 9600 9700
F 0 "R61" H 9700 9800 50  0000 C CNN
F 1 "13.7k 1%" H 9750 9600 50  0000 C CNN
F 2 "" H 9600 9700 50  0001 C CNN
	1    9600 9700
	1    0    0    -1
$EndComp
Text GLabel 10050 9700 0    40   Power ~ 0
GND
$Comp
L TM100_Tester:C C11
U 1 1 0000001E
P 6900 9600
F 0 "C11" H 7000 9700 50  0000 C CNN
F 1 "4.7uF 50V X7R" H 7050 9500 50  0000 C CNN
F 2 "" H 6900 9600 50  0001 C CNN
	1    6900 9600
	1    0    0    -1
$EndComp
$Comp
L TM100_Tester:C C12
U 1 1 0000001F
P 6900 9850
F 0 "C12" H 7000 9950 50  0000 C CNN
F 1 "4.7uF 50V X7R" H 7050 9750 50  0000 C CNN
F 2 "" H 6900 9850 50  0001 C CNN
	1    6900 9850
	1    0    0    -1
$EndComp
Text GLabel 6400 9600 0    40   Power ~ 0
VIN_12_PROT
Text GLabel 6400 9850 0    40   Power ~ 0
VIN_12_PROT
$Comp
L TM100_Tester:C C13
U 1 1 00000020
P 10100 9150
F 0 "C13" H 10200 9250 50  0000 C CNN
F 1 "22uF 25V X7R" H 10250 9050 50  0000 C CNN
F 2 "" H 10100 9150 50  0001 C CNN
	1    10100 9150
	1    0    0    -1
$EndComp
$Comp
L TM100_Tester:C C14
U 1 1 00000021
P 10100 9400
F 0 "C14" H 10200 9500 50  0000 C CNN
F 1 "22uF 25V X7R" H 10250 9300 50  0000 C CNN
F 2 "" H 10100 9400 50  0001 C CNN
	1    10100 9400
	1    0    0    -1
$EndComp
Text GLabel 9650 9150 0    40   Power ~ 0
SYS_5V_RAW
Text GLabel 9650 9400 0    40   Power ~ 0
SYS_5V_RAW
Text GLabel 10600 9150 0    40   Power ~ 0
GND
Text GLabel 10600 9400 0    40   Power ~ 0
GND
Text Notes 6850 10350 0    42   ~ 12
LMR51430 values follow TI 12V->5V/3A, 500kHz example:
L=6.8uH, RFBT=100k, RFBB=13.7k, COUT=2x22uF/25V; use >=2x4.7uF/50V input ceramics.
$Comp
L TM100_Tester:R_US R51
U 1 1 00000022
P 11800 9000
F 0 "R51" H 11900 9100 50  0000 C CNN
F 1 "0.025R 1% 0.5W" H 11950 8900 50  0000 C CNN
F 2 "" H 11800 9000 50  0001 C CNN
	1    11800 9000
	1    0    0    -1
$EndComp
Text GLabel 11300 9000 0    40   Power ~ 0
SYS_5V_RAW
Text GLabel 12300 9000 0    40   Power ~ 0
DRIVE_5V
$Comp
L TM100_Tester:INA226 U7
U 1 1 00000023
P 12800 9700
F 0 "U7" H 12900 9800 50  0000 C CNN
F 1 "INA226 +5V MON" H 12950 9600 50  0000 C CNN
F 2 "" H 12800 9700 50  0001 C CNN
	1    12800 9700
	1    0    0    -1
$EndComp
Text GLabel 12150 10200 0    40   Input ~ 0
SYS_5V_RAW
Text GLabel 13450 10200 0    40   Input ~ 0
DRIVE_5V
Text GLabel 13450 9950 0    40   Input ~ 0
DRIVE_5V
Text GLabel 12800 8850 0    40   Power ~ 0
SYS_3V3
Text GLabel 12800 10550 0    40   Power ~ 0
GND
Text GLabel 13450 9450 0    40   BiDi ~ 0
I2C_SDA
Text GLabel 13450 9700 0    40   Input ~ 0
I2C_SCL
Text GLabel 13450 9200 0    40   Output ~ 0
PWR5_ALERT
$Comp
L TM100_Tester:LDO_3V3 U8
U 1 1 00000024
P 11500 11200
F 0 "U8" H 11600 11300 50  0000 C CNN
F 1 "TLV75533 / 1A LDO" H 11650 11100 50  0000 C CNN
F 2 "" H 11500 11200 50  0001 C CNN
	1    11500 11200
	1    0    0    -1
$EndComp
Text GLabel 10950 10950 0    40   Power ~ 0
SYS_5V_RAW
Text GLabel 10950 11300 0    40   Input ~ 0
SYS_5V_RAW
Text GLabel 12050 10950 0    40   Power ~ 0
SYS_3V3
Text GLabel 11500 11800 0    40   Power ~ 0
GND
Text Notes 10400 12000 0    40   ~ 12
Logic branches from SYS_5V_RAW before R51 so INA226 U7 reports drive +5V current only.
$Comp
L TM100_Tester:R_US R70
U 1 1 00000025
P 14500 10300
F 0 "R70" H 14600 10400 50  0000 C CNN
F 1 "4.7k" H 14650 10200 50  0000 C CNN
F 2 "" H 14500 10300 50  0001 C CNN
	1    14500 10300
	1    0    0    -1
$EndComp
$Comp
L TM100_Tester:R_US R71
U 1 1 00000026
P 14500 10600
F 0 "R71" H 14600 10700 50  0000 C CNN
F 1 "4.7k" H 14650 10500 50  0000 C CNN
F 2 "" H 14500 10600 50  0001 C CNN
	1    14500 10600
	1    0    0    -1
$EndComp
Text GLabel 14000 10300 0    40   Power ~ 0
SYS_3V3
Text GLabel 14000 10600 0    40   Power ~ 0
SYS_3V3
Text GLabel 15000 10300 0    40   BiDi ~ 0
I2C_SDA
Text GLabel 15000 10600 0    40   BiDi ~ 0
I2C_SCL
Text Notes 14200 11000 0    40   ~ 12
U6/U7 ALERT outputs are open-drain. Route separately if GPIO budget permits;
otherwise diode/open-drain OR to POWER_FAULT and query I2C to identify source.
Text GLabel 14500 12500 0    40   Input ~ 0
LED_STATUS
$Comp
L TM100_Tester:R_US R80
U 1 1 00000027
P 15100 12500
F 0 "R80" H 15200 12600 50  0000 C CNN
F 1 "1k" H 15250 12400 50  0000 C CNN
F 2 "" H 15100 12500 50  0001 C CNN
	1    15100 12500
	1    0    0    -1
$EndComp
$Comp
L TM100_Tester:LED D10
U 1 1 00000028
P 15800 12500
F 0 "D10" H 15900 12600 50  0000 C CNN
F 1 "STATUS" H 15950 12400 50  0000 C CNN
F 2 "" H 15800 12500 50  0001 C CNN
	1    15800 12500
	1    0    0    -1
$EndComp
Text GLabel 16400 12500 0    40   Power ~ 0
GND
Text GLabel 14500 12900 0    40   Input ~ 0
LED_FAULT
$Comp
L TM100_Tester:R_US R81
U 1 1 00000029
P 15100 12900
F 0 "R81" H 15200 13000 50  0000 C CNN
F 1 "1k" H 15250 12800 50  0000 C CNN
F 2 "" H 15100 12900 50  0001 C CNN
	1    15100 12900
	1    0    0    -1
$EndComp
$Comp
L TM100_Tester:LED D11
U 1 1 0000002A
P 15800 12900
F 0 "D11" H 15900 13000 50  0000 C CNN
F 1 "FAULT" H 15950 12800 50  0000 C CNN
F 2 "" H 15800 12900 50  0001 C CNN
	1    15800 12900
	1    0    0    -1
$EndComp
Text GLabel 16400 12900 0    40   Power ~ 0
GND
Text GLabel 14500 13300 0    40   Input ~ 0
LED_WRITE_ARMED
$Comp
L TM100_Tester:R_US R82
U 1 1 0000002B
P 15100 13300
F 0 "R82" H 15200 13400 50  0000 C CNN
F 1 "1k" H 15250 13200 50  0000 C CNN
F 2 "" H 15100 13300 50  0001 C CNN
	1    15100 13300
	1    0    0    -1
$EndComp
$Comp
L TM100_Tester:LED D12
U 1 1 0000002C
P 15800 13300
F 0 "D12" H 15900 13400 50  0000 C CNN
F 1 "WRITE ARMED" H 15950 13200 50  0000 C CNN
F 2 "" H 15800 13300 50  0001 C CNN
	1    15800 13300
	1    0    0    -1
$EndComp
Text GLabel 16400 13300 0    40   Power ~ 0
GND
Text Notes 17500 10800 0    60   ~ 12
REQUIRED TEST POINTS
Text Notes 17500 11100 0    40   ~ 12
GND, VIN12, DRIVE12, SYS5_RAW, DRIVE5, 3V3
Text Notes 17500 11400 0    40   ~ 12
DS0, MOTOR, DIR, STEP, INDEX, TRACK0, WP
Text Notes 17500 11700 0    40   ~ 12
READ_DATA raw + buffered, WRITE_DATA, WRITE_GATE, SDA, SCL
Text Notes 700 14100 0    80   ~ 12
DESIGN REVIEW / FABRICATION HOLD
Text Notes 700 14500 0    45   ~ 12
1. This is a KiCad schematic draft generated from HDS v1.0. Open/import in KiCad and save as modern format.
2. Verify package-specific pin numbering for LMR51430 and exact ESP32 module footprint before PCB placement.
3. Select and add exact reverse-polarity MOSFET/eFuse/PTC parts.
4. Confirm TM-100 termination configuration on the actual drives; 150R terminators are intentionally removable.
5. Do not enable write firmware until Write Gate fail-safe testing is completed.
$EndSCHEMATC
