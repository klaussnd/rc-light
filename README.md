# Light controller for RC model planes

This repository contains hardware and firmware for a light controller for RC model planes.
It has two channels for high-power LEDs (intended for anti-collision light (ACL) and
landing light) and 3 outputs for standard LEDs (intended for navigation lights).
The lights can be controlled remotely with a standard servo signal.
The circuit is powered directly from the drive battery to make powering the high-power LEDs
as efficient as possible. An LP2950 voltage regulator generates the 5V for the MCU.
The project uses an ATmega microcontroller. As high-power LED driver, the Zetec ZXLD1360
(or its pin-compatible successor ZXLD 1362) are used.
Debug output can be enabled through the USART of the chip.

![](doc/pcbf.jpg)
![](doc/pcbb.jpg)

![Schematic](doc/rc_light_schematic.svg)

You need to adjust the current control resistors (R6 to R9 and R11 to R13) to match the correct
current of your respective high-current LED according to the
[datasheet of the ZXLD1360](https://www.diodes.com/assets/Datasheets/ZXLD1360.pdf),
section "Setting nominal average output current with external resistor Rs": I = 0.1V / Rs.
For example, to set I = 400mA, use Rs = 0.25&Omega;, by having 4 1&Omega; resistors in parallel.
Also check the pre-resistors of the standard LEDs (R2 to R4), taking into account the pin
voltage as a function of the source current, as described in the
[datasheet of the ATmega168PA](https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/Atmel-9223-Automotive-Microcontrollers-ATmega48PA-ATmega88PA-ATmega168PA_Datasheet.pdf),
section 30.3.6 Pin Driver Strength, figure 30-64 I/O Pin Output Voltage versus Source Current (VCC = 5V).

The project uses git submodules. Remember to initialise and update the submodules recursively,
since also submodules of submodules are used.

Components:

|Comp.  | Part         |[Reichelt](http://reichelt.de) order no.|
|-------|--------------|----------------------------------------|
|U1     |LP2950 5V     |LP 2950 ACZ5,0                          |
|U2     |ATmega 168PA  |ATMEGA 168PA-AU                         |
|U3, U4 |ZXLD1362      |ZXLD 1362ET5TA                          |
|L1, L2 |L-PISG 68µH   |L-PISG 68µ                              |
|D1, D3 |BAT 46W       |BAT 46W                                 |
|D2, D4 |MBRS 140      |MBRS 140 SMD                            |
|C4, C6 |X7R 1210 4.7µF|KEM X7R1210 4,7U                        |

Remaining components are SMD 0805 resistors and capacitors.
Note that the L-PISG 68µH inductor has a maximum current of 0.4A.

The project is licensed under the [GNU affero general public license](LICENSE).
