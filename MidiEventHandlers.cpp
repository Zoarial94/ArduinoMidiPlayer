//
//  MidiEvents.cpp
//  Midi
//
//  Created by s804024 on 9/24/18.
//  Copyright © 2018 Zoarial. All rights reserved.
//

#include "MidiFile.hpp"
#include "MidiEvent.hpp"

void* Midi::MidiFile::noteOn(unsigned char channel) {
  char note = 0;
  char vel = 0;
  midiFile.read(&note, 1);
  midiFile.read(&vel, 1);
  //std::cout << "Note On - Channel: " << (int)channel << std::hex <<  " - Note: " << (int)note << " - Velocity: " << (int)vel << std::dec;
  Event* e = new Midi::Event((0x90 + channel), 2);
  e->setData(0, note);
  e->setData(1, vel);
  return e;

}

void* Midi::MidiFile::noteOff(unsigned char channel) {
  char note = 0;
  char vel = 0;
  midiFile.read(&note, 1);
  midiFile.read(&vel, 1);
  //std::cout << "Note Off - Channel: " << (int)channel << std::hex <<  " - Note: " << (int)note << " - Velocity: " << (int)vel << std::dec;
  Event* e = new Midi::Event((0x80 + channel), 2);
  e->setData(0, note);
  e->setData(1, vel);
  return e;
}

void* Midi::MidiFile::polyAftertouch(unsigned char channel) {
  char note = 0;
  char pressure = 0;
  midiFile.read(&note, 1);
  midiFile.read(&pressure, 1);
  //std::cout << "Polyphonic aftertouch - Channel: " << (int)channel << std::hex << " - Pressure: " << (int)pressure << std::dec;
  Event* e = new Midi::Event((0xA0 + channel), 2);
  e->setData(0, note);
  e->setData(1, pressure);
  return e;
}

void* Midi::MidiFile::controlModeChange(unsigned char channel) {
  char byte1 = 0;
  char byte2 = 0;
  midiFile.read(&byte1, 1);
  midiFile.read(&byte2, 1);
  //std::cout << "Control mode change - Channel: " << (int)channel << std::hex << " - Byte 1: " << (int)byte1 << " - Byte 2: " << (int)byte2 << std::dec;
  //printFileInBits(&midiFile, tempLen, "hex");
  Event* e = new Midi::Event((0xB0 + channel), 2);
  e->setData(0, byte1);
  e->setData(1, byte2);
  return e;
}

void* Midi::MidiFile::programChange(unsigned char channel) {
  char programNum = 0;
  midiFile.read(&programNum, 1);
  //std::cout << "Program Change: " << std::hex << (int)programNum << std::dec << std::dec;
  Event* e = new Midi::Event((0xC0 + channel), 1);
  e->setData(0, programNum);
  return e;
}

void* Midi::MidiFile::channelAftertouch(unsigned char channel) {
  return nullptr;
}

void* Midi::MidiFile::pitchWheelRange(unsigned char channel) {
  return nullptr;
}

void* Midi::MidiFile::systemExclusive(unsigned char * event) {
  if (*event == 0xF0) {
    //std::cout << "System exclusive: 0xF0";
    long toSkip = 0;
    readVarLen(&toSkip);
    char data[toSkip];
    readFileToChar(data, (int)toSkip);
    Event* e = new Midi::Event(0xF0, 1);
    for (int i = 0; i < toSkip; i++) {
      e->setData(i, data[i]);
    }
    return e;
  } else if (*event == 0xF7) {
    //std::cout << "System exclusive: 0xF7";
    long toSkip = 0;
    readVarLen(&toSkip);
    char data[toSkip];
    readFileToChar(data, (int)toSkip);
    Event* e = new Midi::Event(0xF7, 1);
    for (int i = 0; i < toSkip; i++) {
      e->setData(i, data[i]);
    }
    return e;
  }
  return nullptr;
}

void* Midi::MidiFile::metaEvent() {
  char metaType = 0;
  long metaLength = 0;
  readFileToChar(&metaType, 1);
  readVarLen(&metaLength);
  unsigned char data[metaLength + 1];
  data[0] = metaType;
  readFileToChar(data + 1, (int)metaLength);
  Event* e = new Midi::Event(0xFF, metaLength + 1);
  for (int i = 0; i < metaLength + 1; i++) {
    e->setData(i, data[i]);
  }
  return e;
}


