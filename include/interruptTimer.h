#ifndef INTERRUPTLAPTIMER_H
#define INTERRUPTLAPTIMER_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <vector>

class InterruptLapTimer
{
public:
    InterruptLapTimer(uint8_t ledPin, uint8_t rssiPin, uint8_t sdPin, unsigned int adcThreshold, size_t maxLaps);
    void setup();
    void blinkLED(unsigned long periodMs);
    void logRSSIOnly();
    void printRSSI();
    void processData();
    void saveToSDCard();
    void readRXValue();

private:
    void createSDFile();
    void createSDFileRSSIOnly();

    const uint8_t ledPin;
    const uint8_t rssiPin;
    const uint8_t sdPin;
    const unsigned int adcThreshold;
    const size_t maxLaps;

    unsigned long absoluteTimer;
    unsigned long previousLapTime;
    unsigned long lapTimesArray[100]; // Maximum number of laps (adjustable)
    volatile int lapIndex;
    std::vector<unsigned long> lapTimes;

    File lapTimeFile;
    File rssiValueFile;

    volatile unsigned int thresholdCounter;
    volatile unsigned int underThresholdCounter;
    volatile bool isFinished;
    volatile bool overThreshold;
};

#endif
