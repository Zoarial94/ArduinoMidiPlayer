//
//  MidiEvents.cpp
//  Midi
//

#include "MidiFile.hpp"
#include "MidiEvent.hpp"
#include <stdint.h>

namespace Midi {

Event* MidiFile::noteOn(unsigned char channel) {
  uint8_t note = 0;
  uint8_t vel = 0;
  midiFile.read(&note, 1);
  midiFile.read(&vel, 1);
  //std::cout << "Note On - Channel: " << (int)channel << std::hex <<  " - Note: " << (int)note << " - Velocity: " << (int)vel << std::dec;
  uint8_t data[3] = {0x90 + channel, note, vel};
  Event* e = new Midi::Event(data, 3);
  return e;

}

Event* MidiFile::noteOff(unsigned char channel) {
  uint8_t note = 0;
  uint8_t vel = 0;
  midiFile.read(&note, 1);
  midiFile.read(&vel, 1);
  //std::cout << "Note Off - Channel: " << (int)channel << std::hex <<  " - Note: " << (int)note << " - Velocity: " << (int)vel << std::dec;
  uint8_t data[3] = {0x80 + channel, note, vel};
  Event* e = new Midi::Event(data, 3);
  return e;
}

Event* MidiFile::polyAftertouch(unsigned char channel) {
  uint8_t note = 0;
  uint8_t pressure = 0;
  midiFile.read(&note, 1);
  midiFile.read(&pressure, 1);
  //std::cout << "Polyphonic aftertouch - Channel: " << (int)channel << std::hex << " - Pressure: " << (int)pressure << std::dec;
  uint8_t data[3] = {0xA0 + channel, note, pressure};
  Event* e = new Midi::Event(data, 3);
  return e;
}

Event* MidiFile::controlModeChange(unsigned char channel) {
  uint8_t byte1 = 0;
  uint8_t byte2 = 0;
  midiFile.read(&byte1, 1);
  midiFile.read(&byte2, 1);
  //std::cout << "Control mode change - Channel: " << (int)channel << std::hex << " - Byte 1: " << (int)byte1 << " - Byte 2: " << (int)byte2 << std::dec;
  //printFileInBits(&midiFile, tempLen, "hex");
  uint8_t data[3] = {0xB0 + channel, byte1, byte2};
  Event* e = new Midi::Event(data, 3);
  return e;
}

Event* MidiFile::programChange(unsigned char channel) {
  uint8_t programNum = 0;
  midiFile.read(&programNum, 1);
  //std::cout << "Program Change: " << std::hex << (int)programNum << std::dec << std::dec;
  uint8_t data[2] = {0xC0 + channel, programNum};
  Event* e = new Midi::Event(data, 2);
  return e;
}

Event* MidiFile::channelAftertouch(unsigned char channel) {
  return nullptr;
}

Event* MidiFile::pitchWheelRange(unsigned char channel) {
  return nullptr;
}

Event* MidiFile::systemExclusive(unsigned char * event) {
  if (*event == 0xF0) {
    //std::cout << "System exclusive: 0xF0";
    unsigned long toSkip = 0;
    readVarLen(&toSkip);
    uint8_t data[toSkip + 1];
    data[0] = 0xF0;
    readFile(data + 1, (int)toSkip);
    Event* e = new Midi::Event(data, toSkip + 1);
    return e;
  } else if (*event == 0xF7) {
    //std::cout << "System exclusive: 0xF7";
    unsigned long toSkip = 0;
    readVarLen(&toSkip);
    uint8_t data[toSkip + 1];
    data[0] = 0xF7;
    readFile(data + 1, (int)toSkip);
    Event* e = new Midi::Event(data, toSkip + 1);
    return e;
  }
  return nullptr;
}

Event* MidiFile::metaEvent() {
  uint8_t metaType = 0;
  unsigned long metaLength = 0;
  readFile(&metaType, 1);
  readVarLen(&metaLength);
  uint8_t data[metaLength + 2];
  data[0] = 0xFF;
  data[1] = metaType;
  readFile(data + 2, (int)metaLength);
  Event* e = new Midi::Event(data, metaLength + 2);
  return e;
}
}
