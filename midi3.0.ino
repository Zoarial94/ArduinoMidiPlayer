#include "MidiFile.hpp"

Midi::MidiFile* doom;

void setup() {
  Serial.begin(1000000);
  Serial1.begin(31250);
  
  doom = new Midi::MidiFile();

  doom->open("flam.mid");
  doom->start();
  while(!doom->isFinished()) {
    //Serial.println(freeMemory());
    doom->loop();
  }

  Serial.println(F("MIDI IS DONE"));
}

void loop() {
}

