//
//  MidiFile.cpp
//  MidiFile
//


#include "MidiFile.hpp"
#include "MidiEvent.hpp"
#include <MemoryFree.h>
#include <stdint.h>
#include <limits.h>

namespace Midi {

MidiFile::MidiFile(int pin) {

  firstTime = true;

  SSPin = pin;
  pinMode(SSPin, OUTPUT);

  if (!SD.begin(53)) {
    Serial2.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  
  stopAllNotes();
  setVars();
  listsInitialized = false;
}

MidiFile::~MidiFile() {
  delLists();
}

bool MidiFile::open(String str) {
  str = "midi/" + str;
  stopAllNotes();
  if (midiFile) {
    close();
  }

  if (!SD.exists(str)) {
    Serial2.print(F("Error Opening: '"));
    Serial2.print(str);
    Serial2.println(F("'"));
    openFile = false;
  } else {
    Serial2.print(F("Opening file: "));
    midiFile = SD.open(str);
    fileSize = midiFile.size();
    Serial2.print(midiFile.name());
    Serial2.print(F(" (Size: "));
    Serial2.print(fileSize);
    Serial2.println(F(")"));

    //Get bytes for header
    uint8_t header[8];
    readFile(header, 8);

    //Check if file is MIDI file
    if (strncmp((char*)header, (char*)MIDIHEADER, 8) != 0) {
      Serial2.println(F("NOT A MIDI FILE"));
      Serial2.println(F("READ: "));
      for (int i = 0; i < 8; ++i) {
        Serial2.print(header[i], HEX);
        Serial2.print("\t");
      }
      Serial2.println(F("\nACTUAL HEADER: "));
      for (int i = 0; i < 8; ++i) {
        Serial2.print(MIDIHEADER[i], HEX);
        Serial2.print("\t");
      }
      openFile = false;
    }
    openFile = true;
  }
  isReady = false;
  return openFile;
}

bool MidiFile::setup() {
  if (!openFile) {
    Serial2.println("--NO OPEN FILE--");
    return false;
  }

  delLists();
  setVars();

  //Get size of file
  midiFile.seek(8);

  //Get and read file type
  readFile(&fileType, 1); //Just zeros
  readFile(&fileType, 1); //Should be 0, 1, or 2
  Serial2.print(F("Midi Type: "));
  Serial2.print(fileType, BIN);
  if (fileType == 1) {
    Serial2.println(F("(Multi-track sync)"));
  } else if (fileType == 2) {
    Serial2.println(F("(Multi-track async)"));
    return false;
  } else {
    Serial2.println(F("(Single track)"));
    return false;
  }

  //Get and read number of tracks
  readFile(&numOfTracks, 1);
  readFile(&numOfTracks, 1);

  Serial2.print(F("Number of tracks is: "));
  Serial2.println(numOfTracks);

  setLists(); //Set lists now that we have track num

  //Get and read the Time Division
  readFileToLong(&timeDiv, 2);

  Serial2.print(F("Time division: "));
  Serial2.println(timeDiv);

  unsigned long BoTPlaces[numOfTracks];
  findInFile(TRACKHEADER, 4, BoTPlaces);

  findInFile(endOfTrack, 3, EoTPlaces); //Find end of files

  for (int i = 0; i < numOfTracks; i++) {
    EoTPlaces[i] = EoTPlaces[i] + 3; //Add 3(Because endOfTrack is 3 long) to get the last byte of the track
    placeInTrack[i] = BoTPlaces[i] + 8;
    trackCurrentTime[i] = 0;
    Serial2.print(BoTPlaces[i]);
    Serial2.print(F(" to "));
    Serial2.println(EoTPlaces[i]);
  }

  for (int i = 0; i < numOfTracks; i++) {
    eventsWaiting[i] = getEventAt(placeInTrack[i]);
  }

  Serial2.println(F("END OF MIDI HEADER INFO"));
  return true;
}



//1: Can send next event
//2: Check next event for time and see if there is time to add to the queue
//3:
void MidiFile::loop() {
  if (paused || !isReady) {
    return;
  }
  //Serial2.println(numInEvent());  //Print number of items in queue
  float eventTime;
  float realEventTime;
  float prevTime;
  float targetTime;
  unsigned long currentTime2;
  if (!isQEmpty()) { //If there is an event play it
    Event* e = eventQueue[0];
    
    eventTime = getMilliFromDelta(e->getRealTime());
    prevTime = prevEventTime[e->getTrack()];
    currentTime2 = currentTime();
    if (scaleChange > prevTime) {
      float diff = scaleChange - prevTime;
      realEventTime = diff + ((eventTime - diff) / scale);
      Serial2.println(realEventTime);
    } else {
      realEventTime = eventTime / scale;
    }
    targetTime = realEventTime + prevTime;
    
    bool sameTime;
    if (targetTime <= (float)currentTime2) {
      sameTime = true;
    } else {
      sameTime = false;
    }
    while (sameTime && !isQEmpty()) {
      prevEventTime[e->getTrack()] += realEventTime;
      sendEvent(e->toString(), e->getLen());
      e = pop();
      delete e;

      e = eventQueue[0];
      
      eventTime = getMilliFromDelta(e->getRealTime());
      prevTime = prevEventTime[e->getTrack()];
      if (scaleChange > prevTime) {
        float diff = scaleChange - prevTime;
        realEventTime = diff + ((eventTime - diff) / scale);
      } else {
        realEventTime = eventTime / scale;
      }
      targetTime = realEventTime + prevTime;
      
      if (targetTime <= currentTime2) {
        sameTime = true;
      } else {
        sameTime = false;
      }
    }
  } else {
    push(nextEvent());
  }
  if (!tracksAreDone && canPush()) { //If there is an event left in the file... see if it can go into queue
    //Serial2.println("Checking for time...");
    Event* e = eventQueue[0];
    eventTime = getMilliFromDelta(e->getTime());
    prevTime = prevEventTime[e->getTrack()];
    if (scaleChange > prevTime) {
      float diff = scaleChange - prevTime;
      realEventTime = diff + ((eventTime - diff) / scale);
    } else {
      realEventTime = eventTime / scale;
    }
    targetTime = realEventTime + prevTime;
    if ((targetTime - (float)currentTime()) >= 6 && !eventQueue.isFull()) {
      //Serial2.println("Adding event to queue");
      push(nextEvent());
    } else {
      //Serial2.println("Not time to add to queue");
    }
  } else if (tracksAreDone && isQEmpty()) { //If there are no events... finish the file
    //Serial2.println(F("MIDI IS FINISHED"));
    midiIsFinished = true;
  }
  //Serial2.print(F("Size of queue: "));
  //Serial2.println(numInEvent());
}

bool MidiFile::start() {

  stopAllNotes();
  isReady = setup();

  if (!isReady) {
    startTime = -1;
    return false;
  }

  while (!eventQueue.isFull()) {
    push(nextEvent());
  }

  startTime = millis();
  paused = false;
  midiIsFinished = false;

  return true;
}

Event* MidiFile::nextEvent() {

  uint8_t chosenTrack = 0;
  unsigned long earliestTime = ULONG_MAX;
  for (int i = 0; i < numOfTracks; i++) {
    if (trackStatus[i]) {
      unsigned long eventTime = eventsWaiting[i]->getTime();
      if (earliestTime > eventTime) {
        chosenTrack = i;
        earliestTime = eventTime;
      }
    }
  }
  Event* chosenEvent = eventsWaiting[chosenTrack];
  eventsWaiting[chosenTrack] = getEventAt(placeInTrack[chosenTrack]);
  return chosenEvent;
}

Event* MidiFile::getEventAt(unsigned long place) {
  uint8_t trackNum = -1;
  for (int i = numOfTracks - 1; i >= 0; i--) {
    if (place < EoTPlaces[i]) {
      trackNum = i;
    }
  }

  if (!trackStatus[trackNum]) {
    return nullptr;
  }

  midiFile.seek(place);

  uint8_t event = 0;
  unsigned long deltaTime = 0;

  Event* eventInstance;

  readVarLen(&deltaTime); //Get deltaTime
  trackCurrentTime[trackNum] += deltaTime;
  event = peek(); //Peek at next byte
  if (!(event >= 0x80)) { //If it is not an event, use previous event.

    eventInstance = handleEvent(&(prevEvents[trackNum]));
    checkForEoT(trackNum);
  } else { //If it is an event, handle event and set previousEvent

    readFile(&event, 1);
    eventInstance = handleEvent(&event);
    prevEvents[trackNum] = event;
    checkForEoT(trackNum);
  }

  if (eventInstance != NULL) {
    eventInstance->setTime(trackCurrentTime[trackNum]);
    eventInstance->setRealTime(deltaTime);
    eventInstance->setTrack(trackNum);
    placeInTrack[trackNum] = midiFile.position();
    return eventInstance;
  } else {
    return getEventAt(midiFile.position());
  }
}

Event* MidiFile::handleEvent(uint8_t* event) {
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
  } else if (*event == 0xF0) { //System Exclusive
    return systemExclusive(event);
  } else if (*event == 0xF7) { //System Exclusive 'Escape'
    return systemExclusive(event);
  } else if (*event == 0xFF) { //Meta Event
    return metaEvent();
  } else {                          //Invalid Event
    return nullptr;
  }
}

void MidiFile::sendEvent(const uint8_t* data, int len) {
  /*Serial2.print(data[0], HEX);
    Serial2.print(F(": "));
    for (int i = 1; i < len; i++) {
    Serial2.print(data[i], HEX);
    Serial2.print(F(" "));
    }
    Serial2.println();*/
  if (data[0] >= 0x80) {
    if (data[0] == prevSentEvent) {
      for (int i = 1; i < len; i++) {
        Serial1.write(data[i]);
      }
    } else {
      for (int i = 0; i < len; i++) {
        Serial1.write(data[i]);
      }
    }
    if (data[0] == 0xFF && data[1] == 0x51) {
      uSecPerQuarterNote = 0;
      uSecPerQuarterNote = (uSecPerQuarterNote << 8) | (long)data[2];
      uSecPerQuarterNote = (uSecPerQuarterNote << 8) | (long)data[3];
      uSecPerQuarterNote = (uSecPerQuarterNote << 8) | (long)data[4];
    } else if (data[0] == 0xFF && data[1] == 0x58) {
      numerator = data[2];
      denominator = pow(2, data[3]);
    }
  }
}

float MidiFile::getMilliFromDelta(unsigned long delta) {
  if (uSecPerQuarterNote != 0) {
    float f = 0;
    f = (float)delta * (((float)uSecPerQuarterNote / (float)1000) / (float)timeDiv);
    return f;
  } else {
    return delta;
  }
}

void MidiFile::stopAllNotes() {
  for (int i = 0; i < 16; i++) {
    Serial1.write(0xB0 + i);
    Serial1.write(0x7B);
    Serial1.write(0x00);
  }
}

bool MidiFile::isFinished() {
  return midiIsFinished;
}

bool MidiFile::isFileReady() {
  return isReady;
}

void MidiFile::checkForEoT(uint8_t trackNum) {
  if (filePos() == EoTPlaces[trackNum]) {
    trackStatus[trackNum] = 0;
  }
  tracksAreDone = 1;
  for (int i = 0; i < numOfTracks; i++) {
    if (trackStatus[i]) {
      tracksAreDone = 0;
      return;
    }
  }
}

void MidiFile::setVars() {
  fileType = 0;
  numOfTracks = -1;
  timeDiv = 0;
  uSecPerQuarterNote = 500000;
  numerator = 0;
  denominator = 0;
  tracksAreDone = false;
  midiIsFinished = false;
  isReady = false;
  paused = false;
  scale = 1;
  scaleChange = 0;
}

void MidiFile::setLists() {

  if (!listsInitialized && numOfTracks != -1) {

    listsInitialized = true;

    EoTPlaces = new unsigned long[numOfTracks]; //Allocate space for EoTPlaces
    eventsWaiting = new Event*[numOfTracks]; //Allocate space for eventsWaiting
    placeInTrack = new unsigned long[numOfTracks]; //Allocate space for placeInTrack
    prevEvents = new uint8_t[numOfTracks];
    trackCurrentTime = new unsigned long[numOfTracks];
    trackStatus = new bool[numOfTracks];
    prevEventTime = new float[numOfTracks];
  }
  if (listsInitialized) {
    for (int i = 0; i < numOfTracks; i++) {
      EoTPlaces[i] = 0;
      eventsWaiting[i] = 0;
      placeInTrack[i] = 0;
      prevEvents[i] = 0;
      trackCurrentTime[i] = 0;
      trackStatus[i] = 1;
      prevEventTime[i] = 0;
    }
  }
}

void MidiFile::delLists() {

  if (numOfTracks != UINT8_MAX) {
    if (*eventsWaiting) {
      for (int i = 0; i < numOfTracks; i++) {
        Event* e = eventsWaiting[i];
        if (e) {
          delete e;
        }
      }
      delete [] eventsWaiting;
    }
  }

  if (listsInitialized) {

    delete [] EoTPlaces;

    delete [] placeInTrack;

    delete [] prevEvents;

    delete [] trackCurrentTime;

    delete [] trackStatus;

    delete [] prevEventTime;
  }

  int numOfEvents = numInEvent();
  for (int i = 0; i < numOfEvents; i++) {
    Event* e = pop();
    delete (e);
  }
  listsInitialized = false;
}

void MidiFile::command(char* command, unsigned int len) {
  if (strncmp(command, "pause", 5) == 0) {              //PAUSE
    pause();
  } else if (strncmp(command, "resume", 6) == 0) {      //RESUME
    resume();
  } else if (strncmp(command, "open ", 5) == 0) {       //OPEN
    String str;
    for (int i = 0; i < len - 5; ++i) {
      str.concat(command[5 + i]);
    }
    if(!strstr(str.c_str(), ".mid")) {
      str.concat(".mid");
    }
    open(str);
  } else if (strncmp(command, "scale ", 6) == 0) {      //SCALE
    String str;
    for (int i = 0; i < len - 6; ++i) {
      str.concat(command[6 + i]);
    }
    scale = str.toFloat();
    scaleChange = millis();
    Serial2.print(F("Scaling: "));
    Serial2.println(scale, 4);
  } else if (strncmp(command, "start", 5) == 0) {       //START
    start();
  } else if (strncmp(command, "close", 5) == 0) {       //CLOSE
    close();
  } else if (strncmp(command, "list", 4) == 0) {        //LIST
    list();
  } else {                                              //ELSE
    Serial2.print(F("Failed Command: '"));
    for (int i = 0; i < len; ++i) {
      Serial2.print(command[i]);
    }
    Serial2.println("'");
  }
}

void MidiFile::close() {
  stopAllNotes();
  if (midiFile) {
    Serial2.print(F("Closing file: "));
    Serial2.println(midiFile.name());
    midiFile.close();
    openFile = false;
    isReady = false;
  }
  midiIsFinished = false;
}

} //Namespace Midi
