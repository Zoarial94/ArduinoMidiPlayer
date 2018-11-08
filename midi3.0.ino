#include <MemoryFree.h>

#include "MidiFile.hpp"

Midi::MidiFile* doom;

void setup() {
  Serial.begin(1000000);
  Serial1.begin(31250);
  Serial.print(F("FREE MEMORY BEFORE: "));
  Serial.println(freeMemory());
  doom = new Midi::MidiFile();
  Serial.print(F("FREE MEMORY BEFORE: "));
  Serial.println(freeMemory());
  Serial.println(F("CONNECTED"));
  doom->open("flam.mid");
  doom->start();
  while(!doom->isFinished()) {
    //Serial.println(freeMemory());
    doom->loop();
  }
  Serial.println(F("MIDI IS DONE"));
  delete doom;
  Serial.print(F("FREE MEMORY AFTER: "));
  Serial.println(freeMemory());
  doom = new Midi::MidiFile();
  delay(3000);
  doom->open("5.mid");
  doom->start();
  while(!doom->isFinished()) {
    doom->loop();
  }
  Serial.println(F("MIDI IS DONE"));
}

void loop() {
}

