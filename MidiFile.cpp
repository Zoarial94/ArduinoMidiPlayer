//
//  MidiFile.cpp
//  MidiFile
//
//  Created by s804024 on 9/19/18.
//  Copyright © 2018 Zoarial. All rights reserved.
//


#include "MidiFile.hpp"
#include "MidiEvent.hpp"
#include <SD.h>

Midi::MidiFile::MidiFile() {

  firstTime = true;

  if (!SD.begin(53)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  stopAllNotes();
}

Midi::MidiFile::~MidiFile() {
  delVars();
}

void Midi::MidiFile::open(String str) {
  if(midiFile) {
    midiFile.close();
  }
  midiFile = SD.open(str);
  if (!midiFile) {
    Serial.println("error opening " + str);
    while (1);
  }
}

void Midi::MidiFile::setup() {
  delVars();
  setVars();

  //Get size of file
  fileSize = midiFile.size();

  //Get bytes for header
  char * header = new char[8];
  readFileToChar(header, 8);

  //Check if file is MIDI file
  if (strcmp(header, MIDIHEADER) == 0) {
    Serial.println("Is a midi file");
  } else {
    Serial.println("Not a midi file");
    while (1);
  }

  //Getting info on the MIDI file

  //Get and read file type
  readFileToChar(&fileType, 1); //Just zeros
  readFileToChar(&fileType, 1); //Should be 0, 1, or 2
  Serial.print(F("Midi Type: "));
  Serial.println(fileType, BIN);
  if (fileType == 1) {
    Serial.println(F("(Multi-track sync)"));
  } else if (fileType == 2) {
    Serial.println(F("(Multi-track async)"));
  } else {
    Serial.println(F("(Single track)"));
  }

  //Get and read number of tracks
  readFileToChar(&numOfTracks, 1);
  readFileToChar(&numOfTracks, 1);

  Serial.print(F("Number of tracks is: "));
  Serial.println(numOfTracks);

  setLists();

  //Get and read the Time Division
  long tmp = 0;
  readFileToLong(&tmp, 2);
  timeDiv = tmp;

  Serial.print(F("Time division: "));
  Serial.println(timeDiv);

  long BoTPlaces[numOfTracks];

  char tempStr[] = "MTrk";

  findInFile(tempStr, 4, BoTPlaces);

  EoTPlaces = new long[numOfTracks]; //Allocate space for EoTPlaces
  eventsWaiting = new void*[numOfTracks]; //Allocate space for eventsWaiting
  placeInTrack = new long[numOfTracks]; //Allocate space for placeInTrack
  prevEvents = new unsigned char[numOfTracks];
  trackCurrentTime = new long[numOfTracks];
  trackStatus = new bool[numOfTracks];


  findInFile(const_cast<unsigned char*>(endOfTrack), 3, EoTPlaces); //Find end of files

  for (int i = 0; i < numOfTracks; i++) {
    EoTPlaces[i] = EoTPlaces[i] + 3; //Add 3(Because endOfTrack is 3 long) to get the last byte of the track
    placeInTrack[i] = BoTPlaces[i] + 8;
    trackCurrentTime[i] = 0;
    Serial.print(BoTPlaces[i]);
    Serial.print(F(" to "));
    Serial.println(EoTPlaces[i]);
  }

  for (int i = 0; i < numOfTracks; i++) {
    eventsWaiting[i] = (void*)getEventAt(placeInTrack[i]);
  }

  Serial.println(F("\nEND OF MIDI HEADER INFO\n\n"));
}



//1: Can send next event
//2: Check next event for time and see if there is time to add to the queue
//3:
void Midi::MidiFile::loop() {
  if(!isQEmpty()) { //If there is an event play it
    Event* e = (Event*)eventQueue[0];
    long currentTime = this->currentTime() - startTime;
    float eventTime = getMilliFromDelta(e->getRealTime());
    float targetTime = (eventTime + prevEventTime[e->getTrack()]);
    /*Serial.print(F("Current Time: "));
    Serial.println(currentTime);
    Serial.print(F("Target Time: "));
    Serial.println(targetTime);
    Serial.print(F("Event Time: "));
    Serial.println(eventTime);
    Serial.print(F("Prev Event Time: "));
    Serial.println(prevEventTime[e->getTrack()]);
    Serial.println();*/
    if (targetTime <= (float)currentTime) {
      prevEventTime[e->getTrack()] += eventTime;
      char* s = e->toString();
      sendEvent(e->getEvent(), (unsigned char*)s, e->getLen());
      delete s;
      e = (Event*)first();
      delete e;
      /*bool sameTime = true;
      while(sameTime && !isQEmpty()) {
        char* s = e->toString();
        sendEvent(e->getEvent(), (unsigned char*)s, e->getLen());
        delete s;
        e = (Event*)first();
        delete e;
        
        e = (Event*)eventQueue[0];
        eventTime = getMilliFromDelta(e->getTime());
        if(eventTime <= (float)currentTime) {
          sameTime = true;
        } else {
          sameTime = false;
        }
      }*/
    }
  }
  if(!tracksAreDone && canPush()) { //If there is an event left in the queue and file... see if it can go into queue
    //Serial.println("Checking for time...");
    Event* e = (Event*)eventQueue[0];
    long currentTime = this->currentTime() - startTime;
    float eventTime = getMilliFromDelta(e->getTime());
    float targetTime = (eventTime + prevEventTime[e->getTrack()]);
    if ((targetTime - currentTime) >= 6 && !eventQueue.isFull()) {
      //Serial.println("Adding event to queue");
      push(nextEvent());
    } else {
      //Serial.println("Not time to add to queue");
    }
  } else if(tracksAreDone && isQEmpty()) { //If there are no events... finish the file
    //Serial.println(F("MIDI IS FINISHED"));
    midiIsFinished = true;
  }
  //Serial.print(F("Size of queue: "));
  //Serial.println(numInEvent());
}




void Midi::MidiFile::start() {
  setup();
  while (!eventQueue.isFull()) {
    push(nextEvent());
  }
  startTime = currentTime();
}

void* Midi::MidiFile::nextEvent() {

  char chosenTrack = 0;
  unsigned long earliestTime = 2147483647;
  for (int i = 0; i < numOfTracks; i++) {
    if(trackStatus[i]) {
      long eventTime = ((Event*)eventsWaiting[i])->getTime();
      if (earliestTime > eventTime) {
        chosenTrack = i;
        earliestTime = eventTime;
      }
    }
  }
  void* chosenEvent = eventsWaiting[chosenTrack];
  eventsWaiting[chosenTrack] = getEventAt(placeInTrack[chosenTrack]);
  return chosenEvent;
}

void* Midi::MidiFile::getEventAt(long place) {
  unsigned char trackNum = -1;
  for (int i = numOfTracks - 1; i >= 0; i--) {
    if (place < EoTPlaces[i]) {
      trackNum = i;
    }
  }

  if(!trackStatus[trackNum]) {
    return nullptr;
  }

  midiFile.seek(place);

  unsigned char event = 0;
  long deltaTime = 0;

  Event* eventInstance;

  readVarLen(&deltaTime); //Get deltaTime
  trackCurrentTime[trackNum] += deltaTime;
  event = peek(); //Peek at next byte
  if (!(event >= 0x80)) { //If it is not an event, use previous event.

    eventInstance = (Event*)handleEvent(&(prevEvents[trackNum]));
    checkForEoT((int)trackNum);
  } else { //If it is an event, handle event and set previousEvent

    readFileToChar(&event, 1);
    eventInstance = (Event*)handleEvent(&event);
    prevEvents[trackNum] = event;
    checkForEoT((int)trackNum);
  }

  if (eventInstance != NULL) {
    eventInstance->setTime(trackCurrentTime[trackNum]);
    eventInstance->setRealTime(deltaTime);
    eventInstance->setTrack(trackNum);
    placeInTrack[trackNum] = midiFile.position();
    return eventInstance;
  } else {
    return getEventAt((long)(midiFile.position()));
  }
}

void* Midi::MidiFile::handleEvent(unsigned char * event) {
  unsigned char eventID = *event & 0xF0; //First 4 bits give event
  unsigned char channel = *event & 0x0F; //Last 4 bits gives channel number
  if (eventID == 0x80) {           //Note Off
    return noteOff(channel);
  } else if (eventID == 0x90) {    //Note On
    return noteOn(channel);
  } else if (eventID == 0xA0) {    //Polyphonic Aftertouch
    return polyAftertouch(channel);
  } else if (eventID == 0xB0) {    //Control Mode Change
    return controlModeChange(channel);
  } else if (eventID == 0xC0) {    //Program Change
    return programChange(channel);
  } else if (eventID == 0xD0) {    //Channel Aftertouch
    return channelAftertouch(channel);
  } else if (eventID == 0xE0) {    //Pitch Wheel Range
    return pitchWheelRange(channel);
  } else if ((int)*event == 0xF0) { //System Exclusive
    return systemExclusive(event);
  } else if ((int)*event == 0xF7) { //System Exclusive 'Escape'
    return systemExclusive(event);
  } else if ((int)*event == 0xFF) { //Meta Event
    return metaEvent();
  } else {                          //Invalid Event
  }
  return nullptr;
}

bool Midi::MidiFile::canPush() {
  return !eventQueue.isFull();
}

bool Midi::MidiFile::push(void* pointer) {
  //Serial.print(F("Pushing event with time: "));
  //Serial.println(((Event*)pointer)->getTime());
  return eventQueue.push(pointer);
}

void* Midi::MidiFile::pop() {
  return eventQueue.shift();
}

void* Midi::MidiFile::first() {
  return eventQueue.shift();
}

long Midi::MidiFile::currentTime() {
  return millis();
}

void Midi::MidiFile::sendEvent(unsigned char event, unsigned char* data, int len) {
  /*Serial.print(event, HEX);
  Serial.print(F(": "));
  for(int i = 0; i < len; i++) {
    Serial.print(data[i], HEX);
    Serial.print(F(" "));
  }
  Serial.println();*/
  if (event >= 0x80) {
    if (event == prevSentEvent) {
      for (int i = 0; i < len; i++) {
        Serial1.write(data[i]);
      }
    } else {
      Serial1.write(event);
      for (int i = 0; i < len; i++) {
        Serial1.write(data[i]);
      }
    }
    if (event == 0xFF && data[0] == 0x51) {
      Serial.println(F("TEMPO CHANGE"));
      uSecPerQuarterNote = 0;
      uSecPerQuarterNote = (uSecPerQuarterNote << 8) | (long)data[1];
      uSecPerQuarterNote = (uSecPerQuarterNote << 8) | (long)data[2];
      uSecPerQuarterNote = (uSecPerQuarterNote << 8) | (long)data[3];
    } else if (data[0] == 0x58) {
      numerator = data[1];
      denominator = pow(2, data[2]);
    }
  }
}

float Midi::MidiFile::getMilliFromDelta(long delta) {
  if (uSecPerQuarterNote != 0) {
    float f = 0;
    f = delta * (((float)uSecPerQuarterNote / 1000.0f) / (float)timeDiv);
    return f;
    /*float kMillisecondsPerQuarterNote = uSecPerQuarterNote / 1000.0f;
    float kMillisecondsPerTick = kMillisecondsPerQuarterNote / timeDiv;
    float deltaTimeInMilliseconds = delta * kMillisecondsPerTick;
    Serial.print(F("EVENT TIME IN FUNCTION: "));
    Serial.println(deltaTimeInMilliseconds);
    return (long)deltaTimeInMilliseconds;*/
  } else {
    return delta;
  }
}

void Midi::MidiFile::stopAllNotes() {
  for(int i = 0; i < 16; i++) {
    Serial1.write(0xB0 + i);
    Serial1.write(0x7B);
    Serial1.write(0x00);
  }
}

bool Midi::MidiFile::isQEmpty() {
  return eventQueue.isEmpty();
}

int Midi::MidiFile::numInEvent() {
  return eventQueue.size();
}

bool Midi::MidiFile::isFinished() {
  return midiIsFinished;
}

long Midi::MidiFile::filePos() {
  return midiFile.position();
}

void Midi::MidiFile::checkForEoT(int trackNum) {
  if(filePos() == EoTPlaces[trackNum]) {
      trackStatus[trackNum] = 0;
    }
    tracksAreDone = 1;
    for(int i = 0; i < numOfTracks; i++) {
      if(trackStatus[i]) {
         tracksAreDone = 0;
      }
    }
}

void Midi::MidiFile::setVars() {
  fileType = 0;
  numOfTracks = 0;
  timeDiv = 0;
  uSecPerQuarterNote = 500000;
  numerator = 0;
  denominator = 0;
  nextEventTrack = 0;
  nextEventPrevTime = 0;
  nextEventTracksChecked = 0;
  tracksAreDone = false;
  midiIsFinished = false;
}

void Midi::MidiFile::setLists() {
  
  EoTPlaces = new long[numOfTracks]; //Allocate space for EoTPlaces
  eventsWaiting = new void*[numOfTracks]; //Allocate space for eventsWaiting
  placeInTrack = new long[numOfTracks]; //Allocate space for placeInTrack
  prevEvents = new unsigned char[numOfTracks];
  trackCurrentTime = new long[numOfTracks];
  trackStatus = new bool[numOfTracks];
  prevEventTime = new float[numOfTracks];

  for(int i = 0; i < numOfTracks; i++) {
    EoTPlaces[i] = 0;
    eventsWaiting[i] = 0;
    placeInTrack[i] = 0;
    prevEvents[i] = 0;
    trackCurrentTime[i] = 0;
    trackStatus[i] = 0;
    prevEventTime[i] = 0;
  }
}

void Midi::MidiFile::delVars() {
  if(firstTime) {
    firstTime = false;
    return;
  }
  if (*EoTPlaces) {
    delete [] EoTPlaces;
  }
  
  if (*placeInTrack) {
    delete [] placeInTrack;
  }
  
  if (*eventsWaiting) {
    for (int i = 0; i < numOfTracks; i++) {
      void* e = eventsWaiting[i];
      if (e) {
        delete (Event*)e;
      }
    }
    delete [] eventsWaiting;
  }
  
  if (*prevEvents) {
    delete [] prevEvents;
  }
  
  if (*trackCurrentTime) {
    delete [] trackCurrentTime;
  }

  if (*trackStatus) {
    delete [] trackStatus;
  }

  if (*prevEventTime) {
    delete [] prevEventTime;
  }
  
  int numOfEvents = numInEvent();
  for (int i = 0; i < numOfEvents; i++) {
    Event* e = (Event*)pop();
    delete (e);
  }
}

