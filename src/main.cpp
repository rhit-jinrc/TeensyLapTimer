#include <Arduino.h>
#include "interruptTimer.h"
#include "millisTimer.h"

// Toggle between using InterruptLapTimer and MillisLapTimer
#define USE_INTERRUPT_TIMER // Comment this out to use MillisLapTimer

MillisLapTimer millisLapTimer(/* rssiPin */ A4, /* adcThreshold */ 340);
InterruptLapTimer interruptLapTimer(/* ledPin */ 13, /* rssiPin */ A4, /* sdPin */ BUILTIN_SDCARD, /* adcThreshold */ 340, /* maxLaps */ 100);

// Built-In Teensy IntervalTimer
IntervalTimer timer;

void setup()
{
  Serial.begin(9600);

#ifdef USE_INTERRUPT_TIMER
  interruptLapTimer.setup();

  // Start the timer, calling readRXValue on interruptLapTimer every 1 ms
  timer.begin([]()
              { interruptLapTimer.readRXValue(); },
              1000);
#endif
}

void loop()
{
#ifndef USE_INTERRUPT_TIMER
  millisLapTimer.update();              // Print RSSI value and manage lap timing
  millisLapTimer.printRunningLapTime(); // Optionally, display the running lap time
#endif

#ifdef USE_INTERRUPT_TIMER
  if (interruptLapTimer.isSingleLapComplete())
  {
    // TODO: send CAN message of lap time here
    uint32_t lapTime = interruptLapTimer.getLapTime();
    Serial.println(lapTime);
    interruptLapTimer.printTime(lapTime);
  }

  // Check if the lap is complete for the interrupt-based timer
  if (interruptLapTimer.isLapComplete())
  {
    // Stop the interval timer
    timer.end();

    // Process and save the data
    interruptLapTimer.processData();
    interruptLapTimer.saveToSDCard();

    Serial.println("Lap timing complete, data saved.");
  }
#endif
}
