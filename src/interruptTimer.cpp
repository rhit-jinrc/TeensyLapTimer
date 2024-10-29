#include "interruptTimer.h"

InterruptLapTimer::InterruptLapTimer(uint8_t ledPin, uint8_t rssiPin, uint8_t sdPin, unsigned int adcThreshold, size_t maxLaps)
    : ledPin(ledPin), rssiPin(rssiPin), sdPin(sdPin), adcThreshold(adcThreshold), maxLaps(maxLaps),
      absoluteTimer(0), previousLapTime(0), lapIndex(0), thresholdCounter(0), underThresholdCounter(0),
      isFinished(false), overThreshold(false) {}

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
    absoluteTimer++;

    const unsigned long debounceThreshold = 5000;
    if (rssiValue >= adcThreshold && !overThreshold && thresholdCounter >= debounceThreshold)
    {
        overThreshold = true;
        unsigned long currentLapTime = absoluteTimer;

        if (previousLapTime != 0 && lapIndex < maxLaps)
        {
            unsigned long lapDuration = currentLapTime - previousLapTime;
            lapTimesArray[lapIndex++] = lapDuration;
        }
        previousLapTime = currentLapTime;
        thresholdCounter = 0;
        underThresholdCounter = 0;
    }
    else if (rssiValue >= adcThreshold && overThreshold)
    {
        underThresholdCounter = 0;
    }
    else if (rssiValue < adcThreshold && overThreshold)
    {
        underThresholdCounter++;
        if (underThresholdCounter >= 5000)
        {
            overThreshold = false;
            isFinished = true;
        }
    }
    else
    {
        thresholdCounter++;
    }
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
