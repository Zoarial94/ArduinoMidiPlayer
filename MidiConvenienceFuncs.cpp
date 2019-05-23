//
//  MidiConvenienceFuncs.cpp
//  Midi
//

#include "MidiFile.hpp"
#include <SD.h>
#include <stdint.h>

namespace Midi {

void MidiFile::readFileToLong(unsigned long* num, int numOfBytes) {
  char * temp = new char[numOfBytes];
  midiFile.read(temp, numOfBytes);
  for (int i = 0; i < numOfBytes; i++) {
    (*num) = (*num) << 8;
    (*num) = (*num) | ((unsigned long)((temp)[i]) & 0xFF);
  }
  delete [] temp;
}

void MidiFile::readVarLen(unsigned long* value) {
  uint8_t c;
  unsigned long tempValue = 0;
  if ( (tempValue = midiFile.read()) & 0x80 ) {
    tempValue &= 0x7F;
    do {
      tempValue = (tempValue << 7) + ((c = midiFile.read()) & 0x7F);
    } while (c & 0x80);
  }
  *value = tempValue;
}

void MidiFile::findInFile(const uint8_t* data, int len, unsigned long* places) {
  unsigned long starting = filePos();
  unsigned long ending = fileSize;
  unsigned long * pos = new unsigned long[numOfTracks];
  uint8_t buf[len];
  uint8_t curPlaceInBuf = 0;
  unsigned long curPos = starting;
  for (int i = 0; i < ((ending - starting) - (len - 1)); i++) {
    midiFile.seek(curPos);
    readFile(buf, len);
    for (int j = 0; j < len; j++) {
      if (buf[j] == data[j]) {
        if (j == len - 1) {
          pos[curPlaceInBuf] = curPos;
          curPlaceInBuf++;
        }
      } else {
        break;
      }
    }
    curPos++;
  }
  for (int i = 0; i < numOfTracks; i++) {
    places[i] = pos[i];
  }
  delete [] pos;
  midiFile.seek(starting);
}

void MidiFile::printFileInBits(unsigned long num, String type) {
  uint8_t bits = 0;
  for (int i = 0; i < num; i++) {
    bits = midiFile.read();
    Serial2.print(bits, BIN);
    Serial2.print(F("("));
    if (type == "ascii") {
      Serial2.print(bits);
    } if (type == "int") {
      Serial2.print(bits, DEC);
    } if (type == "hex") {
      Serial2.print(bits, HEX);
    } else {
      Serial2.print(F(" "));
    }
    Serial2.print(F(") "));
  }
  Serial2.println();
  midiFile.seek((unsigned long)fileSize - (unsigned long)num);
}
}
