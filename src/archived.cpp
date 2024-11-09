#include <Arduino.h>
// #include <FlexCAN_T4.h>
#include <SPI.h>
#include <SD.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <iterator>
#include "millisTimer.h"

#define LED 13
#define RSSIPIN A4
// #define ARRAY_SIZE 3000
#define ARRAY_BUFFER 100
#define ADC_THRESHOLD 340
#define NUM_LAPS 2
#define SS_PIN 10

void blinkLED(long unsigned int periodms);
void transmitData(long unsigned int periodms);
void readRXValue();
void createSDFile();
void createSDFileRSSIOnly();
void processData();
void saveToSDCard();
void logRSSIOnly();
void printRSSI();

// Simple Lap Timer Solution
void simpleLapTimer();
void printRunningLapTime();
void printLapTime();

// Declare global variables
unsigned long startLapMillis = 0;
unsigned long lastLapMillis = 0;
bool isLapRunning = false;

// FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> can0;
// CAN_message_t tx_msg;
IntervalTimer myTimer;
unsigned long absoluteTimer;
unsigned long absoluteLapTime;
unsigned long preProcessedLapTime;
volatile unsigned int rxAnalogValue;
File lapTimeFile;
File rssiValueFile;
// volatile unsigned int RSSIArray[ARRAY_SIZE] = {0};
// volatile unsigned int RSSIBuffer[ARRAY_SIZE] = {0};
volatile unsigned long absoluteLapTimes[NUM_LAPS];
unsigned int lapCounter = 0;

volatile int currentIndex = 0;
volatile int threshold_counter = 0;
int flushCounter = 0;
boolean isFinished = false;
boolean overThreshold = false;
unsigned long underThresholdCounter = 0;

#define MAX_LAPS 100 // Define max number of laps you want to store
unsigned long lapTimesArray[MAX_LAPS];
volatile int lapIndex = 0; // Use volatile to ensure ISR and main code share the value properly
unsigned long previousLapTime = 0;
std::vector<unsigned long> lapTimes;

// void setup()
// {
//     Serial.begin(9600);
//     pinMode(RSSIPIN, INPUT);
//     pinMode(LED, OUTPUT);
//     digitalWrite(LED, LOW);
//     // can0.begin();
//     // can0.setBaudRate(1000000);
//     delay(100);

//     // createSDFile();
//     //  createSDFileRSSIOnly();
//     delay(100);

//     // myTimer.begin(readRXValue, 1000);
//     //  myTimer.begin(logRSSIOnly, 1000);
//     myTimer.begin(printRSSI, 1000);
//     delay(100);

//     // tx_msg.id = 0x100;
//     // tx_msg.buf[0] = 0x01;
//     // tx_msg.buf[1] = 0x02;
//     // tx_msg.buf[2] = 0x03;
//     // tx_msg.buf[3] = 0x04;
//     // tx_msg.buf[4] = 0x05;
//     // tx_msg.buf[5] = 0x06;
//     // tx_msg.buf[6] = 0x07;
//     // tx_msg.buf[7] = 0x69;
// }

// void loop()
// {
//     blinkLED(200);
//     // transmitData(100);
//     // if (isFinished)
//     // {
//     //   processData();
//     //   isFinished = false;
//     // }

//     // simpleLapTimer();
//     // printRunningLapTime(); // Call function after ensuring it is declared and defined
//     //  delay(100);
// }

void printRSSI()
{
    unsigned int rssiValue = analogRead(RSSIPIN);
    Serial.print("RSSI Value: ");
    Serial.print(rssiValue);
    Serial.print("   \r");
}

void createSDFile()
{
    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println("SD card initialization failed!");
        return;
    }

    Serial.println("Currently writing to SD card");

    // Create file for lap times
    String lapTimeFileName = "lap_times_" + String(micros()) + ".csv";
    char lapTimeCharFileName[lapTimeFileName.length() + 1];
    lapTimeFileName.toCharArray(lapTimeCharFileName, sizeof(lapTimeCharFileName));
    lapTimeFile = SD.open(lapTimeCharFileName, FILE_WRITE);

    if (lapTimeFile)
    {
        lapTimeFile.println("Absolute Lap Times");
    }

    // Create file for RSSI values
    String rssiValueFileName = "rssi_values_" + String(micros()) + ".csv";
    char rssiValueCharFileName[rssiValueFileName.length() + 1];
    rssiValueFileName.toCharArray(rssiValueCharFileName, sizeof(rssiValueCharFileName));
    rssiValueFile = SD.open(rssiValueCharFileName, FILE_WRITE);

    if (rssiValueFile)
    {
        rssiValueFile.println("RSSI Values");
    }
}

void createSDFileRSSIOnly()
{
    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println("SD card initialization failed!");
        return;
    }

    Serial.println("Currently writing to SD card");

    // Create file for RSSI values
    String rssiValueFileName = "rssi_values_only_" + String(micros()) + ".csv";
    char rssiValueCharFileName[rssiValueFileName.length() + 1];
    rssiValueFileName.toCharArray(rssiValueCharFileName, sizeof(rssiValueCharFileName));
    rssiValueFile = SD.open(rssiValueCharFileName, FILE_WRITE);

    if (rssiValueFile)
    {
        rssiValueFile.println("RSSI Values");
    }
}

void readRXValue()
{
    unsigned int rssiValue = analogRead(RSSIPIN); // Read the RSSI value from the analog pin
    absoluteTimer++;                              // Increment the absolute timer (called every 1 ms)

    const unsigned long debounceThreshold = 5000; // Debounce threshold in milliseconds (5 seconds)

    if (rssiValue >= ADC_THRESHOLD && !overThreshold && threshold_counter >= debounceThreshold)
    {
        overThreshold = true;                         // Mark that we're over the threshold
        unsigned long currentLapTime = absoluteTimer; // Record the current lap time

        if (previousLapTime != 0 && lapIndex < MAX_LAPS) // Ensure it's not the first lap and we haven't exceeded array size
        {
            unsigned long lapDuration = currentLapTime - previousLapTime; // Calculate lap duration
            lapTimesArray[lapIndex++] = lapDuration;                      // Store lap time in the pre-allocated array
        }
        previousLapTime = currentLapTime; // Update previous lap time
        threshold_counter = 0;            // Reset debounce counter
        underThresholdCounter = 0;        // Reset the under-threshold counter
    }
    else if (rssiValue >= ADC_THRESHOLD && overThreshold)
    {
        underThresholdCounter = 0; // Reset the under-threshold counter since we're still above threshold
    }
    else if (rssiValue < ADC_THRESHOLD && overThreshold)
    {
        underThresholdCounter++;

        if (underThresholdCounter >= 5000) // 1s to declare lap finish
        {
            overThreshold = false; // Reset the over-threshold flag
            isFinished = true;     // Mark the lap as finished
        }
    }
    else
    {
        threshold_counter++;
    }
}

void logRSSIOnly()
{
    unsigned int rssiValue = analogRead(RSSIPIN);
    // Serial.println(rssiValue);
    rssiValueFile.println(rssiValue);
    flushCounter++;
    if (flushCounter >= 1000)
    { // flush every second
        rssiValueFile.flush();
        flushCounter = 0;
        Serial.println("Flushed data");
    }
}

void processData()
{
    static int lastProcessedLapIndex = 0; // Track the last index we processed

    // Check if there are new lap times in the array
    if (lastProcessedLapIndex < lapIndex)
    {
        // Transfer lap times from array to vector
        while (lastProcessedLapIndex < lapIndex)
        {
            lapTimes.push_back(lapTimesArray[lastProcessedLapIndex++]);
        }
    }
}

void saveToSDCard()
{
    // Check if the file is open before writing
    if (lapTimeFile)
    {
        // Write each lap time from the vector to the file
        for (size_t i = 0; i < lapTimes.size(); i++)
        {
            lapTimeFile.println(lapTimes[i]);
        }

        // Close the file after writing
        lapTimeFile.close();
        Serial.println("Completed writing lap times to SD card file.");
    }
    else
    {
        Serial.println("Error opening SD card file for writing.");
    }

    // Optionally close the RSSI value file if needed
    if (rssiValueFile)
    {
        rssiValueFile.close();
    }
}

void blinkLED(long unsigned int periodms)
{
    static int timer = 0;
    static int ledState = 0;
    if (millis() - timer >= periodms)
    {
        digitalWrite(LED, !ledState);
        timer = millis();
        ledState = !ledState;
    }
}

void transmitData(long unsigned int periodms)
{
    static int timer = 0;
    if (millis() - timer >= periodms)
    {
        timer = millis();
        // if (can0.write(tx_msg)) {
        //   Serial.println("CAN tx in queue");
        // } else {
        //   Serial.println("CAN tx failed");
        // }
    }
}
