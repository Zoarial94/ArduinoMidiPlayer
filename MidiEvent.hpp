//
//  MidiEvent.hpp
//  Midi
//

#ifndef MidiEvent_hpp
#define MidiEvent_hpp

#include <stdint.h>

namespace Midi {
class Event {

  protected:
    unsigned long timeOfEvent;      //
    uint8_t* data;                  // event + data
    uint8_t lenOfData;              //Length of data
    unsigned long timeDiv;          //
    uint8_t track;                  //What track is event on
    unsigned long realTimeOfEvent;  //


  public:

    Event(uint8_t* data, uint8_t len);
    ~Event();
    uint8_t getLen();
    void setTime(unsigned long time);
    unsigned long getTime();
    void setRealTime(unsigned long time);
    unsigned long getRealTime();
    const uint8_t* toString();
    uint8_t getTrack();
    void setTrack(int track);

};
}

#endif /* MidiEvent_hpp */
