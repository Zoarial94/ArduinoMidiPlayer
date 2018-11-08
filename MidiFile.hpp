//
//  MidiFile.hpp
//  MidiFile
//
//  Created by s804024 on 9/19/18.
//  Copyright © 2018 Zoarial. All rights reserved.
//

#ifndef Midi_hpp
#define Midi_hpp

#include <CircularBuffer.h>
#include <SD.h>

#ifndef QUEUESIZE
#define QUEUESIZE 100
#endif

namespace Midi {
class MidiFile {

  private:
    CircularBuffer<void*, QUEUESIZE> eventQueue;

    File midiFile;

    bool tracksAreDone;
    bool midiIsFinished;
    bool firstTime;

    long long fileSize; //Midi Info
    unsigned char numOfTracks; //Midi Info
    unsigned char fileType; //Midi Info
    long timeDiv; //ticks per quarter note
    long uSecPerQuarterNote;
    unsigned char numerator;
    unsigned char denominator;

    unsigned char prevSentEvent;

    long startTime;
    float * prevEventTime;

    bool canPush(); //Is The CircularBuffer Full?
    bool isQEmpty();
    int numInEvent();
    bool push(void* pointer); //Push Next Event to the End
    void* pop(); //Get Next Event (Make Sure to Destory it)
    long filePos();

    void setVars();
    void delVars();
    void setLists();

    void checkForEoT(int trackNum);

    long* EoTPlaces; //End of Track Places
    long* placeInTrack; //The Current Position in Each Track
    void** eventsWaiting; //A Buffer Which Holds the Latest Event in Each Track
    unsigned char* prevEvents; //A Buffer Which Holds the Previous Event Used in Each Track (Running Status)
    long* trackCurrentTime; //Time to check in track for (?)
    bool* trackStatus; //Is track finished

    unsigned char nextEventTrack; //Used in nextEvent Function
    unsigned long nextEventPrevTime; //Used in nextEvent Function
    unsigned char nextEventTracksChecked; //Used in nextEvent Function

    float getMilliFromDelta(long delta);

    void stopAllNotes();

    //Bytes to signal end of track
    const unsigned char endOfTrack[3] = {static_cast<char>(0xFF), 0x2F, 0x00};
    //Bytes that are the the beginning of a midi file
    const unsigned char MIDIHEADER[8] = {'M', 'T', 'h', 'd', 0, 0, 0, 6};

    //Get initial info before the loop
    void setup();
    //Checks eventsWaiting for latest event retuns its pointer
    void* nextEvent();
    //returns pointer to an event at 'place' in the file
    void* getEventAt(long place);
    //Get First Item in Queue
    void* first();

    //Funtions for reading the file
    //Read to array of char
    void readFileToChar(unsigned char * chars, int numOfBytes);
    void readFileToChar(char * chars, int numOfBytes);
    //Read to long (Or std::bitset)
    void readFileToLong(long * num, int numOfBytes);
    //Peak at next character
    unsigned char peek();
    //Read variable length
    //Returns number of bytes read
    void readVarLen(long * value);
    //Finds string in file and prints 8 bytes from that location
    void findInFile(unsigned char * str, int len, long * places);


    //Functions to handle Midi Events
    //Note on
    void* noteOn(unsigned char channel);
    //Note off
    void* noteOff(unsigned char channel);
    //Polyphonic aftertouch
    void* polyAftertouch(unsigned char channel);
    //Control mode change
    void* controlModeChange(unsigned char channel);
    //Program Change
    void* programChange(unsigned char channel);
    //Channel aftertouch
    void* channelAftertouch(unsigned char channel);
    //Pitch wheel range
    void* pitchWheelRange(unsigned char channel);
    //System Exclusive
    void* systemExclusive(unsigned char * event);
    //Meta Event
    void* metaEvent();
    //*** depreciated ***
    void readEvent(File * file);
    //Go through track and handle events
    void handleTrack(long * place);
    //Should only be called in handleTrack()
    //Takes event and arguments
    //Returns number of bytes read
    void* handleEvent(unsigned char * event);

    void Midi::MidiFile::printFileInBits(long num, String type);

    long currentTime();

    void sendEvent(unsigned char event, unsigned char* data, int len);

  public:

    MidiFile();

    ~MidiFile();

    //Open File
    void open(String);
    //Start: make sure is a midi file
    //Get basic information
    void start();
    //Must be continually called to keep class functioning
    void loop();

    bool isFinished();

};

class Event;

class StandardEvent;
class ExtendedEvent;

}


#endif /* Midi_hpp */


