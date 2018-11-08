//
//  MidiEvent.cpp
//  Midi
//
//  Created by s804024 on 9/27/18.
//  Copyright © 2018 Zoarial. All rights reserved.
//

#include "MidiEvent.hpp"

long Midi::Event::getTime() {
  return timeOfEvent;
}

unsigned char Midi::Event::getLen() {
  return lenOfData;
}

Midi::Event::Event(unsigned char event, unsigned char len) {
  this->event = event;
  this->channel = event & 0xF0;
  data = new unsigned char[len];
  lenOfData = len;
}

Midi::Event::~Event() {
  if (*data) {
    delete [] data;
  }
}

char* Midi::Event::toString() {

  char* p = new char[lenOfData];
  for (int i = 0; i < lenOfData; i++) {
    p[i] = data[i];
  }

  return p;

}

void Midi::Event::setData(char place, unsigned char data) {
  if (place > lenOfData || place < 0) {
    return;
  }
  this->data[place] = data;
}

void Midi::Event::setTime(long time) {
  this->timeOfEvent = time;
}

unsigned char Midi::Event::getEvent() {
  return event;
}

int Midi::Event::getTrack() {
  return track;
}

void Midi::Event::setTrack(int track) {
  this->track = track;
}

void Midi::Event::setRealTime(long time) {
  this->realTimeOfEvent = time;
}

long Midi::Event::getRealTime() {
  return realTimeOfEvent;
}
