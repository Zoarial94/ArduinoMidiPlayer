#include "MidiFile.hpp"

Midi::MidiFile* doom; //Pointer to class

void setup() {
  Serial.begin(1000000);           //COMM with computer
  Serial1.begin(31250);            //COMM with midi device
  
  doom = new Midi::MidiFile();     //Initialize class

  doom->open("flam.mid");          //Open midi file from SD Card
  doom->start();                   //Initialize part of class
  
  while(!doom->isFinished()) {     //While there are events to play...
    doom->loop();                  //do stuff...
  }

  Serial.println(F("MIDI IS DONE"));
}

void loop() {
}

