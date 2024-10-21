#include "millisTimer.h"
#include <Arduino.h>

extern unsigned long startLapMillis;
extern bool isLapRunning;
extern unsigned long lastLapMillis;
#define RSSIPIN A4
#define ADC_THRESHOLD 340

// Simple lap timer
void printTime(unsigned long timeMillis)
{
    unsigned long minutes = timeMillis / 60000;          // Get minutes
    unsigned long seconds = (timeMillis % 60000) / 1000; // Get seconds
    unsigned long milliseconds = timeMillis % 1000;      // Get milliseconds

    Serial.print((minutes < 10) ? "0" : ""); // Leading zero if < 10
    Serial.print(minutes);
    Serial.print(":");

    Serial.print((seconds < 10) ? "0" : ""); // Leading zero if < 10
    Serial.print(seconds);
    Serial.print(":");

    if (milliseconds < 100)
        Serial.print("0"); // Leading zero for MS if < 100
    if (milliseconds < 10)
        Serial.print("0");
    Serial.print(milliseconds);
}

void printRunningLapTime()
{
    if (isLapRunning)
    {
        unsigned long currentTime = millis();
        unsigned long lapTime = currentTime - startLapMillis;

        Serial.print("Running Lap Time: ");
        printTime(lapTime);
        Serial.print("    \r"); // Print in place
    }
}

void simpleLapTimer()
{
    unsigned int rssiValue = analogRead(RSSIPIN);

    Serial.print("RSSI Value: ");
    Serial.print(rssiValue);
    Serial.print("   \r");

    unsigned long currentTime = millis();

    if (rssiValue > ADC_THRESHOLD && (currentTime - lastLapMillis) > 5000)
    {
        if (isLapRunning)
        {
            unsigned long lapTime = currentTime - startLapMillis;

            Serial.print("\nLap Completed! Time: ");
            printTime(lapTime);
            Serial.println();

            isLapRunning = false;
        }

        startLapMillis = currentTime;
        lastLapMillis = currentTime;
        isLapRunning = true;
    }
}