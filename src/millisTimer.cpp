#include "millisTimer.h"

MillisLapTimer::MillisLapTimer(int rssiPin, unsigned int adcThreshold)
    : rssiPin(rssiPin), adcThreshold(adcThreshold), startLapMillis(0), lastLapMillis(0), isLapRunning(false) {}

// Print time in minutes:seconds:milliseconds format
void MillisLapTimer::printTime(unsigned long timeMillis) const
{
    unsigned long minutes = timeMillis / 60000;
    unsigned long seconds = (timeMillis % 60000) / 1000;
    unsigned long milliseconds = timeMillis % 1000;

    Serial.print((minutes < 10) ? "0" : "");
    Serial.print(minutes);
    Serial.print(":");
    Serial.print((seconds < 10) ? "0" : "");
    Serial.print(seconds);
    Serial.print(":");

    if (milliseconds < 100)
        Serial.print("0");
    if (milliseconds < 10)
        Serial.print("0");
    Serial.print(milliseconds);
}

// Print running lap time if a lap is in progress
void MillisLapTimer::printRunningLapTime()
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

// Update function to be called repeatedly in loop to print RSSI value and manage laptiming
void MillisLapTimer::update()
{
    unsigned int rssiValue = analogRead(rssiPin);

    Serial.print("RSSI Value: ");
    Serial.print(rssiValue);
    Serial.print("   \r");

    unsigned long currentTime = millis();

    // Check for lap completion based on RSSI and debounce threshold
    if (rssiValue > adcThreshold && (currentTime - lastLapMillis) > 5000) // debounce threshold of 5 seconds
    {
        if (isLapRunning)
        {
            unsigned long lapTime = currentTime - startLapMillis;

            Serial.print("\nLap Completed! Time: ");
            printTime(lapTime);
            Serial.println();

            isLapRunning = false;
        }

        // Start a new lap
        startLapMillis = currentTime;
        lastLapMillis = currentTime;
        isLapRunning = true;
    }
}
