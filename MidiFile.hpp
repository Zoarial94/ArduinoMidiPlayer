//
//  MidiFile.hpp
//  MidiFile
//

#ifndef Midi_hpp
#define Midi_hpp

#include <CircularBuffer.h>
#include <SD.h>
#include "MidiEvent.hpp"
#include <stdint.h>

#define QUEUESIZE 100

namespace Midi {

class Event;

class MidiFile {

  private:

    //Bytes to signal end of track
    const uint8_t endOfTrack[3] = {0xFF, 0x2F, 0x00};
    //Bytes that are the the beginning of a midi file
    const uint8_t MIDIHEADER[8] = {(uint8_t)'M', (uint8_t)'T', (uint8_t)'h', (uint8_t)'d', 0, 0, 0, 6};
    //Bytes that are the beginning of a Track chunk
    const uint8_t TRACKHEADER[4] = {(uint8_t)'M', (uint8_t)'T', (uint8_t)'r', (uint8_t)'k'};

    CircularBuffer<Event*, QUEUESIZE> eventQueue;

    File midiFile;

    unsigned long fileSize;                 //Size of midi file
    unsigned long timeDiv;                  //ticks per quarter note
    unsigned long uSecPerQuarterNote;       //Microseconds per quarternote
    unsigned long startTime;                //When the file started being played
    unsigned long pauseTime;
    unsigned long scaleChange;

    unsigned long*    EoTPlaces;               //End of Track Places
    unsigned long*    placeInTrack;            //The current position in file for each track
    Event**           eventsWaiting;                  //A Buffer Which Holds the Latest Event in Each Track
    uint8_t*          prevEvents;                    //A Buffer Which Holds the Previous Event Used in Each Track (Running Status IN FILE ***NOT*** OVER THE WIRE)
    unsigned long*    trackCurrentTime;        //A buffer which holds
    bool*             trackStatus;                      //Is track finished
    float*            prevEventTime;                   //

    bool tracksAreDone;    //
    bool midiIsFinished;   //Has the file been completly run through
    bool firstTime;        //
    bool listsInitialized; //Are arrays initalized
    bool openFile;
    bool isReady;
    bool paused;

    uint8_t numOfTracks;                    //Number of tracks in midi file
    uint8_t fileType;                       //Midi file type - 0, 1, or 2
    uint8_t numerator;                      //Time signature numerator
    uint8_t denominator;                    //Time signature denominator
    uint8_t prevSentEvent;                  //Last event sent over midi -- Used for 'Running Status' OVER THE WIRE
    uint8_t SSPin;

    float scale;

    void setVars();   //
    void delLists();   //
    void setLists();  //

    void checkForEoT(uint8_t trackNum);   //Checks for end of track

    float getMilliFromDelta(unsigned long delta);

    void stopAllNotes();    //Stop all notes on midi device

    bool setup();                   //Get initial info before the loop
    Event* nextEvent();             //Checks eventsWaiting for latest event retuns its pointer
    Event* getEventAt(unsigned long place);  //returns pointer to an event at 'place' in the file
    Event* first();                 //Get First Item in Queue

    //Read to long (Or std::bitset)
    void readFileToLong(unsigned long* num, int numOfBytes);
    //Read variable length
    //Returns number of bytes read
    void readVarLen(unsigned long* value);
    //Finds string in file and prints 8 bytes from that location
    void findInFile(const uint8_t* data, int len, unsigned long* places);


    //Functions to handle Midi Events
    //Note on
    Event* noteOn(uint8_t channel);
    //Note off
    Event* noteOff(uint8_t channel);
    //Polyphonic aftertouch
    Event* polyAftertouch(uint8_t channel);
    //Control mode change
    Event* controlModeChange(uint8_t channel);
    //Program Change
    Event* programChange(uint8_t channel);
    //Channel aftertouch
    Event* channelAftertouch(uint8_t channel);
    //Pitch wheel range
    Event* pitchWheelRange(uint8_t channel);
    //System Exclusive
    Event* systemExclusive(uint8_t * event);
    //Meta Event
    Event* metaEvent();

    Event* handleEvent(uint8_t* event);

    void printFileInBits(unsigned long num, String type);    //Prints 'num' bytes in 'type' format

    void sendEvent(const uint8_t* data, int len);  //Send event over the wire

    bool canPush();                                   //Is The CircularBuffer Full?
    bool isQEmpty();                                  //Is the buffer empty?
    int numInEvent();                                 //How many items in queue
    bool push(Event* pointer);                        //Push Next Event to the End
    Event* pop();                                     //Get Next Event (Make Sure to Destory it)
    unsigned long filePos();                          //Position in file
    unsigned long currentTime();                      //Time since midi file started playing
    uint8_t peek();                                   //Peak at next character
    void readFile(uint8_t* data, int numOfBytes);     //Read numOfBytes bytes to data

    void pause();
    void resume();
    void list();


  public:

    MidiFile(int pin);

    ~MidiFile();

    //Open File
    bool open(String);
    void close();
    //Start: make sure is a midi file
    //Get basic information
    bool start();
    //Must be continually called to keep class functioning
    void loop();

    void command(char* command, unsigned int len);

    bool isFinished();

    bool isFileReady();

};

inline unsigned long Midi::MidiFile::filePos() {
  return midiFile.position();
}

inline bool Midi::MidiFile::isQEmpty() {
  return eventQueue.isEmpty();
}

inline int Midi::MidiFile::numInEvent() {
  return eventQueue.size();
}

inline bool Midi::MidiFile::canPush() {
  return !eventQueue.isFull();
}

inline bool Midi::MidiFile::push(Event* pointer) {
  //Serial2.print(F("Pushing event with time: "));
  //Serial2.println(pointer->getTime());
  return eventQueue.push(pointer);
}

inline Midi::Event* Midi::MidiFile::pop() {
  return eventQueue.shift();
}

inline unsigned long Midi::MidiFile::currentTime() {
  return millis() - startTime;
}

inline uint8_t Midi::MidiFile::peek() {
  return midiFile.peek();
}

inline void Midi::MidiFile::readFile(uint8_t* data, int numOfBytes) {
  midiFile.read(data, numOfBytes);
}

} //Namespace Midi

#endif /* Midi_hpp */
