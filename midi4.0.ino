#include <MemoryFree.h>

#include "MidiFile.hpp"
#include <stdint.h>

Midi::MidiFile* file;
char buf[255];

void setup() {
  Serial.begin(2000000);
  Serial.setTimeout(10);

  Serial1.begin(31250);

  Serial2.begin(115200);
  Serial2.setTimeout(10);


  Serial2.println(F("CONNECTED"));

  bool b;

  Serial2.print(F("FREE MEMORY BEFORE CONSTRUCTION: "));
  Serial2.println(freeMemory());

  file = new Midi::MidiFile(53);

  /*b = file->open("SOUL5.mid");

  if (!b) {
    Serial2.println(F("ERROR OPENING FILE"));
    while (1);
  }

  b = file->start();

  if (!b) {
    Serial2.println(F("ERROR STARTING"));
    while (1);
  }

  while (!file->isFinished()) {

  }

  Serial2.println(F("MIDI IS DONE"));

  delete file;

  Serial2.print(F("FREE MEMORY AFTER DECONSTRUCTION: "));
  Serial2.println(freeMemory());*/
}

void loop() {
  if(file->isFileReady() && !file->isFinished()) {
    file->loop();
  } else if(file->isFinished()) {
    Serial2.println(F("Midi is finished"));
    file->close();
  }
  if (Serial2.available() > 0) {
    int len = Serial2.readBytes(buf, 255);
    for (int i = 0; i < len; ++i) {
      if(buf[i] == '\n') {
        buf[i] = '\0';
      }
    }
    buf[len] = '\0';
    file->command(buf, len);
  }
}
