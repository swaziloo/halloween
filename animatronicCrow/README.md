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
___
## [Crow Printing Instructions](stl/README.md) ##
___
## [Crow Build Guide](BUILD.md) ##
___
![New beak/head in the foreground.](images/AniCrow011.jpg)
### Modifications and model sources: ###

| Model                    | Source           | Modifications                                                                                                                                                                                                                                                                                                                                                                                     |
|--------------------------|------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Global                   |                  | Standardized on M3 x 4 (or 5) x 5 heat set inserts and M3 fasteners.                                                                                                                                                                                                                                                                                                                              |
| Head                     | andymcculloch    | Refined beak shape to add more of a point. Smoothed/bevelled lower beak opening. Cut away mounts and rebuilt them slightly wider and short enough to fit below the SG90 servo.                                                                                                                                                                                                                    |
| Beak                     | Doctor_Johnsmith | Added a lower collar that fits inside the head without a visible cut-away. Re-drew mounting rail and added separate pin mount holes for actuator. Sliced to enable 3D printing standing up.                                                                                                                                                                                                       |
| Beak Servo Mount Plate   | andymcculloch    | Refined connector tower to accommodate the larger beak when fully open. Straightened out SG-90 mount and shrunk mounting holes to match the screws that come with SG90 servos. Added hexagonal grille for sound. Moved M3 screw holes and shrunk wire pass-thru.                                                                                                                                  |
| Head Stepper Collar      | andymcculloch    | Moved stepper center. Shrunk cable opening and added alignment pins. Added hexagonal grille for sound with slide path for head pivot pins. Moved M3 screw holes.                                                                                                                                                                                                                                  |
| Body Stepper Mount Plate | Andymcculloch    | Moved stepper center. Added arched cutout for alignment pins. Added head pivot pins. Added hexagonal grille for sound. Enlarged head holes to accommodate M3 screws.                                                                                                                                                                                                                              |
| Body                     | Doctor_Johnsmith | Cut away servo mount plate and added mounts for andymcculloch stepper mount plate. Added enclosed box for 40mm speaker. Added internal wire management ring. Added mount rail for Creature Control 5x12 PCB. Added internal sensor mount. Added PIR window to right knee (optional). Enclosed 10x3mm magnet mounts. Added flush pad and M3 hole for securing tail. Added alignment tabs for feet. |
| Feet                     | Doctor_Johnsmith | Added alignment tabs. Added tie-down horizontal hole (optional).                                                                                                                                                                                                                                                                                                                                  |
| Tail                     | Doctor_Johnsmith | Enclosed 10x3mm magnet mounts. Added snap-in tab mount for securing tail to body (optional). Enlarged hole for exterior connection. Made wing tips pointier.                                                                                                                                                                                                                                      |

### Additions: ###

| Model               | Description                                                                                                                                                                                 |
|---------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Actuator            | Printable arm for connecting hinged beak to servo. Recommend 0.2mm nozzle.                                                                                                                  |
| Actuator Spacer     | Fills the gap between the actuator and the SG90 arm. Recommend a 0.2mm nozzle.                                                                                                              |
| PCB Mount           | Small rail to mount the Creature Control 5x12 board with M3 hardware to slide inside the body.                                                                                              |
| PIR Mount           | Holds a mini PIR sensor and snaps into place inside the right leg. Recommend 0.2mm nozzle.                                                                                                  |
| PIR Cover           | Internal shield if your PIR sensor is seeing changes internally when the crow turns its head.                                                                                               | 
| PIR Window Plug     | Fills a PIR hole in the right knee if you change your mind. Print vertically to match the crow outer pattern.                                                                               |
| PIR Window Template | A printable oval that matches the hole shape for the PIR window. The intention was to use black IR transmissive acrylic to cover the sensor, but it's too dark for effective sensing. YMMV. |
| LD1020 Mount        | Holds an LD1020 radar sensor and snaps into place inside the right leg. Recommend a 0.2mm nozzle.                                                                                           |
| Magnet Shims        | 0.8mm and 1.0mm shims to fill the gap between your actual 10x3 magnet heights and the 3.5mm space in the tail and body. See [Crow Printing Instructions](stl/README.md).                    |