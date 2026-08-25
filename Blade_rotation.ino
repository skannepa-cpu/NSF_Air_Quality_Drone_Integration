#include <Servo.h>

Servo ESC;

void setup() {
  Serial.begin(9600);
  ESC.attach(9, 1000, 2000);

  // 1. ARMING SEQUENCE
  Serial.println("Arming ESC... Please wait.");
  ESC.writeMicroseconds(1000); // Send minimum throttle signal
  delay(3000);                 // Wait 3 seconds for arming beeps

  Serial.println("=========================================");
  Serial.println("ESC Armed and Ready!");
  Serial.println("Enter a percentage from 0 to 100 and press Enter.");
  Serial.println("=========================================");
}

void loop() {
  // Check if a percentage value was entered in the Serial Monitor
  if (Serial.available() > 0) {
    // Read the integer typed by the user
    int percent = Serial.parseInt();

    // Clear any leftover characters (like newline '\n') in the buffer
    while (Serial.available() > 0) {
      Serial.read();
    }

    // Safety constraint: keep percentage strictly between 0 and 100
    percent = constrain(percent, 0, 100);

    // Map 0% - 100% to 1000us - 2000us PWM signal
    int pwmSignal = map(percent, 0, 100, 1000, 2000);

    // Apply speed signal to motor
    ESC.writeMicroseconds(pwmSignal);

    // Output status back to the Serial Monitor
    Serial.print("Throttle set to: ");
    Serial.print(percent);
    Serial.print("%  -->  Output Signal: ");
    Serial.print(pwmSignal);
    Serial.println(" us");
  }
}