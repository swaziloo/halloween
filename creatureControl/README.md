![The Creature Control 5x12 PCB](images/cc5x12-003.png)
# Creature Control PCB #
When hand soldering a board to enable the animatronic crow I realized that I wanted a half-dozen crows and maybe some talking skulls and hand soldering wasn't my favorite way to spend my time.
Hence Creature Control 5x12. Designed to enable a bunch of capability using an inexpensive and easy to program Raspberry Pi 2040 in the tiny Waveshare Zero format.

## 5x12 v1.0 ##
Provides a 5-volt 3-amp power rail to supply 3 servos and 2 steppers controlled by a RP2040-Zero which is also wired to incorporate 2 LEDs, 2 motion sensors, the DFPlayer Mini mp3 player and a pin header for the remaining pins.
Designed to be hand-soldered and easily programmable using Arduino libraries.
### v1.1 2025-09 ###
Larger PTH (0.9mm) for MCU and mp3 player mounts, 
add 1KΩ serial resistor for DFPlayer RX (MCU GP0) connection, 
add capacitor (104) for DFPlayer 3V3, 
remove incorrect silk for C7 and C8 value,
rotate sensor mounts for better routing,
update schema.

*Gerber files have been produced for JLCPCB but may work elsewhere.*

![Animatronic Crow build on CC5x12](images/cc5x12-002.png)
### VITAL IMPORTANT SAFETY NOTICE! ###
*The 5 volt power rail allows for multiple source connections but only one should be used at any given time.* 
*Do not connect more than one 5 volt source to the power rail!*
(Unless you really know what you're doing.)
It is safe to connect the power to the 5v rail and the RP2040-Zero to your PC at the same time--in fact it's necessary for testing.
### Parts List ###
The intent is that one would install only the components needed for a project.

| Qty          | Ref   | Part                                                         | Note                                                                                                     |
|--------------|-------|--------------------------------------------------------------|----------------------------------------------------------------------------------------------------------|
| 1            |       | RP2040-Zero                                                  | Waveshare or equivalent.                                                                                 |
| 1            | D2    | 1N5819 Schottky Diode                                        | For RP2040-Zero VCC.                                                                                     |
| 1            |       | DFPlayer Mini                                                | DFRobot or equivalent.                                                                                   |
| 1            | C9    | 100nF 50v Ceramic Capacitor (104)                            | (v1.1) for DFPlayer Mini 3V3                                                                             |
| 1            | R6    | 1KΩ Resistor                                                 | (v1.1) for MCU GP0->DFPlayer Mini RX                                                                     |
| 1-2          |       | ULN2003A                                                     | Stepper driver.                                                                                          |
| 1-2          |       | IC DIP Socket 16-Pin                                         | For ULN2003A.                                                                                            |
| 1-2          | C7,C8 | 1µF 50v Ceramic Capacitor (105)                              | For ULN2003A (mislabelled on silk).                                                                      |
| 1-2          |       | XH2.54mm 5-Pin Connector                                     | Straight pin for stepper.                                                                                |
| 1 (optional) | D1    | 3mm LED                                                      | For power rail indicator.                                                                                |
| 1 (optional) | R3    | Resistor for power indicator LED                             | I used 2.2kΩ for a blue 3mm LED.                                                                         |
| 1-2          | R4,R5 | Resistor for external LEDs                                   | Based on LED(s) to be used.                                                                              |
| 1-5          | C2-C6 | 100µF 16v Electrolytic Capacitor                             | One for each 5v servo and stepper.                                                                       |
| 1            | C1    | 1000µF 16v Electrolytic Capacitor                            | For 5v rail.                                                                                             |
| 1 (optional) | J2    | 5.5x2.1MM DC-005 Power Jack                                  | If using a 5v/3A adapter.                                                                                |
| 1 (optional) | J1    | USB-C 6-pin Surface Mount                                    | If using a USB battery.                                                                                  |
| 2 (optional) | R1,R2 | 5.1kΩ Resistors                                              | If using a USB battery.                                                                                  |
| 1 (optional) | J3    | KF350-2P 3.5mm Pitch 2Pin PCB Screw Terminal Block Connector | If using a wired 5v source or drain. Also useful if needing to share ground with an external power rail. |
| optional     |       | 2.54mm Pin Headers                                           | For board connections.                                                                                   |

## Building the CC5x12 ##

I will add a full build guide, but for the time-being, here are a few tips:
* You do not need to fill the board. It is perfectly acceptable to install only the connections and components you intend to use.
* If you are installing the USB-C socket, do that first and check the electrical connections. Every other component is easier to hand solder.
* The RP2040-Zero can be powered by its USB-C connection or the board 5v power, but it will not provide power to the steppers or servos. LEDs, the DFPlayer Mini and sensors draw power from the RP2040-Zero 3V3.
* The C1 capacitor is intended to smooth out the 5v supply for the board. C2-C6 provide demand power for their nearby stepper or servo. Only install caps for those you intend to connect.
___

## Testing ##
Install the Arduino IDE and the following libraries:
[AccelStepper](https://www.airspayce.com/mikem/arduino/AccelStepper),
[DFRobotDFPlayerMini](https://github.com/DFRobot/DFRobotDFPlayerMini),
[arduino-pico](https://github.com/earlephilhower/arduino-pico) by Earle Philhower,
[Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel).
Install using the library manager in the Arduino IDE.

* Enable the *Waveshare RP2040-Zero* board in the board manager.
* Connect power to one of the 5v inputs and the RP2040-Zero to your pc. 
* Select the connected device/port and ensure the Waveshare RP2040-Zero board is assigned.
* Copy the [BoardTest.ino](BoardTest.ino) sketch into the IDE.
* Set 'true' for available components at the top of the sketch. 
* Select the 'Upload' button.
* The sketch should compile, the board should initialize, and a test loop should run.

The steppers, servos, and MP3 components will cycle.
The LED(s) will reflect the sensor(s) states.

___
## Connecting to a 12v Rail ##
You should be able to control 12v steppers and servos using the CC5X12 by eliminating the 5v connection to them and providing separate power. 
If you do this, you will want to pull a shared ground over from the block connector or another board source.
*I have not tested this functionality.*


