//
//  MidiConvenienceFuncs.cpp
//  Midi
//
//  Created by s804024 on 9/24/18.
//  Copyright © 2018 Zoarial. All rights reserved.
//

#include "MidiFile.hpp"
#include <SD.h>

void Midi::MidiFile::readFileToChar(unsigned char * chars, int numOfBytes) {
  midiFile.read(chars, numOfBytes);
}

void Midi::MidiFile::readFileToChar(char * chars, int numOfBytes) {
  midiFile.read(chars, numOfBytes);
}

void Midi::MidiFile::readFileToLong(long * num, int numOfBytes) {
  char * temp = new char[numOfBytes];
  midiFile.read(temp, numOfBytes);
  for (int i = 0; i < numOfBytes; i++) {
    (*num) = (*num) << 8;
    (*num) = (*num) | ((long)((temp)[i]) & 0xFF);
  }
  delete temp;
}

unsigned char Midi::MidiFile::peek() {
  return midiFile.peek();
}

void Midi::MidiFile::readVarLen(long * value) {
  unsigned char c;
  long tempValue = 0;
  if ( (tempValue = midiFile.read()) & 0x80 ) {
    tempValue &= 0x7F;
    do {
      tempValue = (tempValue << 7) + ((c = midiFile.read()) & 0x7F);
    } while (c & 0x80);
  }
  *value = tempValue;
}

void Midi::MidiFile::findInFile(unsigned char * str, int len, long * places) {
  long starting = midiFile.position();
  long ending = fileSize;
  long * pos = new long[numOfTracks];
  char buf[len];
  int curPlaceInBuf = 0;
  long curPos = starting;
  for (int i = 0; i < ((ending - starting) - (len - 1)); i++) {
    midiFile.seek(curPos);
    readFileToChar(buf, len);
    for (int j = 0; j < len; j++) {
      if (buf[j] == (char)str[j]) {
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
  midiFile.seek(starting);
}

void Midi::MidiFile::printFileInBits(long num, String type) {
    unsigned char bits = 0;
    for(int i = 0; i<num; i++) {
        bits=midiFile.read();
       Serial.print(bits, BIN);
       Serial.print(F("("));
        if(type == "ascii") {
            Serial.print(bits);
        } if(type == "int") {
            Serial.print(bits, DEC);
        } if(type == "hex") {
            Serial.print(bits, HEX);
        } else {
            Serial.print(F(" "));
        }
        Serial.print(F(") "));
    }
    Serial.println();
    midiFile.seek((long)fileSize-(long)num);
}

