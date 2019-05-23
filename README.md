# ArduinoMidiPlayer
With a connected SD Card Reader, the arduino can output midi events

***THIS LIBRARY IS A WIP AND IS NOT FULLY FUNCTIONAL***

***ONLY WORKS WITH TYPE 1 MIDI FILES***

To use this code you first need a couple of items:
* Arduino (Only tested on MEGA 2560 REV3)
* SD Card Reader
* MIDI Header with soldered wires
* MIDI cable


## Usage

* This library uses the Arduino SD library so attach your reader accordingly. (Chip select pin must be given during construction)
* Next connect the midi header into the arduino using Serial1 as the output
* Connect bluetooth module using Serial2
* Load midi files onto SD Card
* Look at *Midi4.0.ino* for example usage

## Features

* Will play most standard events
  * Will **NOT** send "channel aftertouch" and "pitch wheel range"
* Skips system exclusive events (Not tested)
* Sends meta events
* Will correct for tempo changes
* A loop function which needs to be regularly called
* Pause and continue functions
* Commands allow for opening and closing files, starting, and pausing payback

## Future Features

* Better optimizations
* Manual tempo change
