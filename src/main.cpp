#include <Arduino.h>
#include "interruptTimer.h"
#include "millisTimer.h"

// Usage: to use the millisLapTimer, please comment out interruptLapTimer.setup(); and timer.begin(...) in setup(), and the isLapComplete
// conditional check in loop
// To use the interruptLapTimer, please comment out millisLapTimer.update() and millisLapTimer.printRunningLapTimer() in loop()

MillisLapTimer millisLapTimer(/* rssiPin */ A4, /* adcThreshold */ 340);
InterruptLapTimer interruptLapTimer(/* ledPin */ 13, /* rssiPin */ A4, /* sdPin */ BUILTIN_SDCARD, /* adcThreshold */ 340, /* maxLaps */ 100);

// Built-In Teensy IntervalTimer
IntervalTimer timer;

void setup()
{
  Serial.begin(9600);
  interruptLapTimer.setup();

  // Start the timer, calling readRXValue on lapTimer every 1 ms
  timer.begin([]()
              { interruptLapTimer.readRXValue(); }, 1000);
}

void loop()
{
  millisLapTimer.update(); // Print RSSI value and manage laptiming

  millisLapTimer.printRunningLapTime(); // Optionally, display the running lap time

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
}
