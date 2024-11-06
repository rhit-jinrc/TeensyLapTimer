#include "interruptTimer.h"

InterruptLapTimer::InterruptLapTimer(uint8_t ledPin, uint8_t rssiPin, uint8_t sdPin, unsigned int adcThreshold, size_t maxLaps)
    : ledPin(ledPin), rssiPin(rssiPin), sdPin(sdPin), adcThreshold(adcThreshold), maxLaps(maxLaps),
      absoluteTimer(0), previousLapTime(0), lapDuration(0), lapIndex(0), thresholdCounter(0), underThresholdCounter(0),
      isFinished(false), overThreshold(false), finishedALap(false) {}

void InterruptLapTimer::setup()
{
    pinMode(rssiPin, INPUT);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    SD.begin(sdPin);
    createSDFile();
}

void InterruptLapTimer::blinkLED(unsigned long periodMs)
{
    static unsigned long lastBlinkTime = 0;
    unsigned long currentTime = millis();
    if (currentTime - lastBlinkTime >= periodMs)
    {
        digitalWrite(ledPin, !digitalRead(ledPin)); // Toggle LED
        lastBlinkTime = currentTime;
    }
}

void InterruptLapTimer::createSDFile()
{
    if (!SD.begin(sdPin))
    {
        Serial.println("SD card initialization failed!");
        return;
    }

    String fileName = "lap_times_" + String(micros()) + ".csv";
    lapTimeFile = SD.open(fileName.c_str(), FILE_WRITE);
    if (lapTimeFile)
        lapTimeFile.println("Absolute Lap Times");

    fileName = "rssi_values_" + String(micros()) + ".csv";
    rssiValueFile = SD.open(fileName.c_str(), FILE_WRITE);
    if (rssiValueFile)
        rssiValueFile.println("RSSI Values");
}

void InterruptLapTimer::createSDFileRSSIOnly()
{
    if (!SD.begin(sdPin))
    {
        Serial.println("SD card initialization failed!");
        return;
    }

    String fileName = "rssi_values_only_" + String(micros()) + ".csv";
    rssiValueFile = SD.open(fileName.c_str(), FILE_WRITE);
    if (rssiValueFile)
        rssiValueFile.println("RSSI Values");
}

void InterruptLapTimer::logRSSIOnly()
{
    unsigned int rssiValue = analogRead(rssiPin);
    if (rssiValueFile)
    {
        rssiValueFile.println(rssiValue);
        static int flushCounter = 0;
        flushCounter++;
        if (flushCounter >= 1000)
        {
            rssiValueFile.flush();
            flushCounter = 0;
            Serial.println("Flushed data");
        }
    }
}

void InterruptLapTimer::printRSSI()
{
    unsigned int rssiValue = analogRead(rssiPin);
    Serial.print("RSSI Value: ");
    Serial.print(rssiValue);
    Serial.print("   \r");
}

void InterruptLapTimer::readRXValue()
{
    unsigned int rssiValue = analogRead(rssiPin);
    absoluteTimer++; // Increment the absolute time counter

    const unsigned long debounceThreshold = 5000;

    // Check if the RSSI value crosses the threshold
    if (rssiValue >= adcThreshold)
    {
        if (!overThreshold && underThresholdCounter >= debounceThreshold)
        {
            overThreshold = true;
            unsigned long currentLapTime = absoluteTimer;

            lapDuration = currentLapTime - previousLapTime;
            lapTimesArray[lapIndex++] = lapDuration;
            finishedALap = true;

            previousLapTime = currentLapTime;
            thresholdCounter = 0;
            underThresholdCounter = 0;
        }

        // Check if car is idling by the laptimer for 5 seconds
        else if (overThreshold && thresholdCounter >= debounceThreshold)
        {
            // RSSI stayed over the threshold long enough to consider car idling
            overThreshold = false;
            isFinished = true;
        }
        else
        {
            thresholdCounter++; // Increment threshold counter while RSSI is high, but when the counter is less than the debounce
        }
    }
    else
    {
        // RSSI dropped below the threshold
        finishedALap = false;
        if (overThreshold)
        {
            overThreshold = false;
        }
        thresholdCounter = 0; // Reset counter when RSSI is below threshold
        underThresholdCounter++;
    }
}

unsigned long InterruptLapTimer::getLapTime()
{
    return lapDuration;
}

bool InterruptLapTimer::isSingleLapComplete() const
{
    return finishedALap;
}

bool InterruptLapTimer::isLapComplete() const
{
    return isFinished;
}

void InterruptLapTimer::processData()
{
    static int lastProcessedLapIndex = 0;
    while (lastProcessedLapIndex < lapIndex)
    {
        lapTimes.push_back(lapTimesArray[lastProcessedLapIndex++]);
    }
}

void InterruptLapTimer::saveToSDCard()
{
    if (lapTimeFile)
    {
        for (size_t i = 0; i < lapTimes.size(); i++)
        {
            lapTimeFile.println(lapTimes[i]);
        }
        lapTimeFile.close();
        Serial.println("Completed writing lap times to SD card file.");
    }
    if (rssiValueFile)
    {
        rssiValueFile.close();
    }
}

// Print time in minutes:seconds:milliseconds format
void InterruptLapTimer::printTime(unsigned long timeMillis) const
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
