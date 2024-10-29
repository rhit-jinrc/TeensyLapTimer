#ifndef MILLISTIMER_H
#define MILLISTIMER_H

#include <Arduino.h>

class MillisLapTimer
{
public:
    MillisLapTimer(int rssiPin, unsigned int adcThreshold);
    void update();                                  // To be called in loop to check RSSI and update lap status
    void printRunningLapTime();                     // Prints current lap time if a lap is running
    void printTime(unsigned long timeMillis) const; // Prints formatted time in minutes:seconds:milliseconds

private:
    int rssiPin;
    unsigned int adcThreshold;
    unsigned long startLapMillis;
    unsigned long lastLapMillis;
    bool isLapRunning;
};

#endif
