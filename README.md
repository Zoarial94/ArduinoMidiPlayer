# ArduinoMidiPlayer
With a connected SD Card Reader, the arduino can output midi events

***THIS LIBRARY IS A WIP AND IS NOT FULLY FUNCTIONAL***

***THIS LIBRARY WILL ONLY WORK WITH ONE FILE BEFORE BREAKING***

***THERE WILL BE MEMORY LEAKS***

***ONLY WORKS WITH TYPE 1 MIDI FILES***

To use this code you first need a couple of items:
* Arduino (Only tested on MEGA 2560 REV3)
* SD Card Reader
* MIDI Header with soldered wires
* MIDI cable


## Usage

* This library uses the Arduino SD library so attach your reader accordingly. (Pin 53 is chip select)
* Next connect the midi header into the arduino using Serial1 as the output
* Load midi files onto SD Card
* Look at *Midi3.0.ino* for example usage
