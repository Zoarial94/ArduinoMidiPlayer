//
//  MidiEvent.hpp
//  Midi
//
//  Created by s804024 on 9/27/18.
//  Copyright © 2018 Zoarial. All rights reserved.
//

#ifndef MidiEvent_hpp
#define MidiEvent_hpp

#include "MidiFile.hpp"

class Midi::Event {

  protected:
    char event;
    char channel;
    long timeOfEvent;
    unsigned char* data;
    unsigned char lenOfData;
    long timeDiv;
    int track;
    long realTimeOfEvent;


  public:

    Event(unsigned char event, unsigned char len);
    ~Event();
    unsigned char getLen();
    void setTime(long time);
    long getTime();
    void setRealTime(long time);
    long getRealTime();
    char* toString();
    void setData(char place, unsigned char data);
    unsigned char getEvent();
    int getTrack();
    void setTrack(int track);

};

#endif /* MidiEvent_hpp */


