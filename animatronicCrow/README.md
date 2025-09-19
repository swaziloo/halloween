![The Animatronic Crow](images/AniCrow022.jpg)
# Animatronic Crow
Based on  designs from Thingiverse:
[Animatronic Halloween Crow by andymcculloch](https://www.thingiverse.com/thing:6278223)
and [Animatronic Halloween Crow by Doctor_JohnSmith](https://www.thingiverse.com/thing:6258153)
which are themselves remixes from [danman](https://www.thingiverse.com/danman/designs)
who remixed the original model by [YahooJAPAN](https://www.thingiverse.com/yahoojapan/designs).

My work has been done in [KiCad](https://www.kicad.org/),
[FreeCAD](https://www.freecad.org/),
and [Blender](https://www.blender.org/download/releases/4-4/).

## [Crow Build Guide](BUILD.md) ##
### Parts List: ###

#### Crow ####

| Qty | Part                     | Note                                                                                                                               |
|-----|--------------------------|------------------------------------------------------------------------------------------------------------------------------------|
| 2   | 5mm Red LEDs             | For the eyes. I am using frosted.                                                                                                  |
| 1   | SG90 5v Servo            |                                                                                                                                    |
| 1   | 28BYJ-48 5v Stepper      | If using CC5x12 you do not need the breakout board (though you could transfer those components to the CC5x12).                     |
| 1   | 40mm 4Ω Speaker          | 8Ω will work.                                                                                                                      |
| 1   | Micro SD Card            | For the mp3 audio files.                                                                                                           |
| 1   | HC-SR312 AM312 Mini PIR  | Human motion sensor.                                                                                                               |
| 3   | Wires/Connectors         | Two 2-wire and one 3-wire for the speaker, LEDs, and PIR. CC5X12 uses standard 2.54mm pin headers (or could be soldered directly). |
| 1   | Large Paperclip          | ~1.25mm thick.                                                                                                                     |
| 8   | M3x4x5 Heat Melt Inserts | 5mm tall inserts will also fit.                                                                                                    |
| 8   | M3 screws                | 2 x 8mm for the head, 4 x 6mm for the stepper and PCB, 2 x 5mm countersunk for the body (or regular 6mm should work).              |
| 8   | 9 x 2mm magnets          | Neodymium high strength magnets for attaching tail to body.                                                                        |
|     | Hot Glue                 | Black preferred.                                                                                                                   |
|     | Polyfill                 | Just a little for sound dampening in the speaker housing.                                                                          |

#### [Creature Control 5x12](../creatureControl/README.md) ####
Note that the CC5x12 board will not be fully filled for this project. 
CC5x12 provides multiple options for powering the 5v rail.
***You must only ever connect one of them at a time***. 
The RP2040-Zero can be plugged into power (including your computer) while the 5v rail is also receiving power.

| Qty          | Part                              | Note                                                         |
|--------------|-----------------------------------|--------------------------------------------------------------|
| 1            | RP2040-Zero                       | Waveshare or equivalent.                                     |
| 1            | DFPlayer Mini                     | DFRobot or equivalent.                                       |
| 1            | IC DIP Socket 16-Pin              | For ULN2003A.                                                |
| 1            | ULN2003A                          |                                                              |
| 1            | XH2.54mm 5-Pin Connector          | Straight pin for stepper.                                    |
| 1            | 1N5819 Schottky Diode             | For RP2040-Zero VCC.                                         |
| 1 (optional) | 3mm LED                           | For power rail indicator.                                    |
| 1 (optional) | Resistor for above LED            | I used 2.2kΩ for a blue 3mm LED.                             |
| 1            | Resistor for Eye LEDs             | I used a 2.2kΩ for a pair of red 5mm LEDs wired in parallel. |
| 1            | 10nF 50v Ceramic Capacitor        | For ULN2003A.                                                |
| 2            | 100uF 16v Electrolytic Capacitor  | For servo and stepper.                                       |
| 1            | 1000uF 16v Electrolytic Capacitor | For 5v rail.                                                 |
| 1 (optional) | 5.5x2.1MM DC-005 Power Jack       | If using a 5v/3A adapter.                                    |
| 1 (optional) | USB-C 6-pin Surface Mount         | If using a USB battery.                                      |
| 2 (optional  | 5.1kΩ Resistors                   | If using a USB battery.                                      |
| optional     | 2.54mm Pin Headers                | If not soldering wires directly to board.                    |

![New beak/head in the foreground.](images/AniCrow011.jpg)
### Modifications and model sources: ###

| Model                             | Source           | Modifications                                                                                                                                                                                                                        |
|-----------------------------------|------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Global                            |                  | Standardized on M3 x 4 or 5 x 5 heat set inserts for M3 fasteners.                                                                                                                                                                   
| Head                              | andymcculloch    | Refined beak shape to add more of a point. Smoothed/bevelled lower beak opening. Cut away mounts and rebuilt them slightly wider and short enough to fit below the SG90 servo.                                                       |
| Beak                              | Doctor_Johnsmith | Added a lower collar that fits inside the head without a visible cut-away. Re-drew mounting rail. Sliced to enable 3D printing standing up.                                                                                          |
| Beak Servo Mount Plate            | andymcculloch    | Refined connector tower to accommodate the larger beak. Straightened out SG-90 mount and shrunk mounting holes to match the screws that come with SG90 servos. Added hexagonal grille for sound. Moved M3 screw holes.               |
| Head Stepper Collar               | andymcculloch    | Moved stepper center. Shrunk cable opening and added alignment pins. Added hexagonal grille for sound. Moved M3 screw holes.                                                                                                         |
| Body Stepper Mount Plate          | Andymcculloch    | Moved stepper center. Added arched cutout for alignment pins. Added head stepper collar support pins. Added hexagonal grille for sound. Enlarged head holes to accommodate M3 screws.                                                |
| Body *(in progress, will change)* | Doctor_Johnsmith | Cut away servo mount plate and added mounts for andymcculloch stepper mount plate. Added enclosed box for 40mm speaker. Added mount rail for Creature Control 5x12 PCB. Enclosed 9x2mm magnet mounts. Added alignment tabs for feet. |
| Feet *(in progress, will change)* | Doctor_Johnsmith | Added alignment tabs.                                                                                                                                                                                                                |
| Tail *(in progress, will change)* | Doctor_Johnsmith | Enclosed 9x2mm magnet mounts. Enlarged hole for exterior connection.                                                                                                                                                                 |

### Additions: ###

| Model           | Description                                                                                    |
|-----------------|------------------------------------------------------------------------------------------------|
| Actuator Spacer | Fits between the SG90 and the 90-degree bend in the control wire.                              |
| PCB Mount       | Small rail to mount the Creature Control 5x12 board with M3 hardware to slide inside the body. |
