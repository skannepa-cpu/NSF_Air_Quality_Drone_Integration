#include <Wire.h>
#include "RTClib.h"

// Change to RTC_DS1307 if you are using the older DS1307 module
RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);

  // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Check if the RTC lost power and needs the time set
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // Sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  // Read the current date and time from the RTC
  DateTime now = rtc.now();

  // Print the date
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  
  // Print the time
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  // Wait 1 second before printing again
  delay(1000);
}