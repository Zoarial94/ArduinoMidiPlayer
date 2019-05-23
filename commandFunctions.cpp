#include "MidiFile.hpp"

namespace Midi {

void MidiFile::resume() {
  if (paused) {
    Serial2.println(F("Resuming"));
    startTime += millis() - pauseTime;
    paused = false;
  }
}

void MidiFile::pause() {
  if (!paused) {
    Serial2.println(F("Pausing"));
    pauseTime = millis();
    paused = true;
  }
}

void MidiFile::list() {
  File dir = SD.open(F("/midi/"));
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    Serial2.print(entry.name());
    if (!entry.isDirectory()) {
      // files have sizes, directories do not
      Serial2.print("\t\t");
      Serial2.println(entry.size(), DEC);
    }
    entry.close();
  }
}

}
