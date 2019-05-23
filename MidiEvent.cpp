//
//  MidiEvent.cpp
//  Midi
//

#include "MidiEvent.hpp"
#include <stdint.h>

unsigned long Midi::Event::getTime() {
  return timeOfEvent;
}

uint8_t Midi::Event::getLen() {
  return lenOfData;
}

Midi::Event::Event(uint8_t* data, uint8_t len) : data(new uint8_t[len]) {
  for (int i = 0; i < len; ++i) {
    this->data[i] = data[i];
  }
  lenOfData = len;
}

Midi::Event::~Event() {
  if (*data) {
    delete [] data;
  }
}

const uint8_t* Midi::Event::toString() {
  return data;
}

void Midi::Event::setTime(unsigned long time) {
  this->timeOfEvent = time;
}

void Midi::Event::setTrack(int track) {
  this->track = track;
}

void Midi::Event::setRealTime(unsigned long time) {
  this->realTimeOfEvent = time;
}

unsigned long Midi::Event::getRealTime() {
  return realTimeOfEvent;
}

uint8_t Midi::Event::getTrack() {
  return track;
}
