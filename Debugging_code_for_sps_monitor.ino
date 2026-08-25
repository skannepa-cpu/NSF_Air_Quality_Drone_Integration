#include <Arduino.h>
#include <SensirionI2cSps30.h>
#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include <RTClib.h>

#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

SensirionI2cSps30 sensor;
RTC_DS3231 rtc;
SdFat sd; 
File dataFile;

static int16_t error;

// --- Control Pin Definitions ---
#define OE_PIN 5           
#define CS_PIN 10          
#define CD_PIN 7           

// --- System Status Green LED Pins ---
#define SPS_LED_PIN 4    // Solid Green = SPS30 Active & OK
#define RTC_LED_PIN 6    // Solid Green = RTC Active & OK
#define SD_LED_PIN  8    // Solid Green = SD Card Active & OK

char logFileName[13] = "LOG000.CSV";

// Function to log formatted data to the SD card stream
void logDataStream(Print& out, DateTime now, uint16_t m1, uint16_t m25, uint16_t m4, uint16_t m10, 
                   uint16_t n05, uint16_t n1, uint16_t n25, uint16_t n4, uint16_t n10, uint16_t tps) {
    
    // --- Date Format: MM/DD/YYYY ---
    if (now.month() < 10) out.print(F("0")); 
    out.print(now.month(), DEC); out.print(F("/"));
    if (now.day() < 10) out.print(F("0")); 
    out.print(now.day(), DEC); out.print(F("/"));
    out.print(now.year(), DEC); out.print(F(" "));
    
    // --- 12-Hour Time Conversion (hh:mm:ss AM/PM) ---
    uint8_t hour24 = now.hour();
    uint8_t hour12 = hour24 % 12;
    if (hour12 == 0) hour12 = 12; 
    
    if (hour12 < 10) out.print(F("0")); 
    out.print(hour12, DEC); out.print(F(":"));
    
    if (now.minute() < 10) out.print(F("0")); 
    out.print(now.minute(), DEC); out.print(F(":"));
    
    if (now.second() < 10) out.print(F("0")); 
    out.print(now.second(), DEC);

    if (hour24 >= 12) {
        out.print(F(" PM"));
    } else {
        out.print(F(" AM"));
    }

    char sep = ',';
    
    out.print(sep); out.print(m1);
    out.print(sep); out.print(m25);
    out.print(sep); out.print(m4);
    out.print(sep); out.print(m10);
    out.print(sep); out.print(n05);
    out.print(sep); out.print(n1);
    out.print(sep); out.print(n25);
    out.print(sep); out.print(n4);
    out.print(sep); out.print(n10);
    out.print(sep); out.println(tps); 
}

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        delay(100);
    }
    
    // Configure System Status LED Pins
    pinMode(SPS_LED_PIN, OUTPUT);
    pinMode(RTC_LED_PIN, OUTPUT);
    pinMode(SD_LED_PIN, OUTPUT);
    
    // Start with all system LEDs OFF
    digitalWrite(SPS_LED_PIN, LOW);
    digitalWrite(RTC_LED_PIN, LOW);
    digitalWrite(SD_LED_PIN, LOW);

    Serial.println(F("\n=========================================="));
    Serial.println(F("       Initializing Air Quality Monitor    "));
    Serial.println(F("=========================================="));

    // Ensure OE_PIN starts LOW so I2C lines stay clean
    pinMode(OE_PIN, OUTPUT);
    digitalWrite(OE_PIN, LOW);
    pinMode(CD_PIN, INPUT_PULLUP);

    Wire.begin();
    delay(50);

    // --- 1. SPS30 Setup ---
    Serial.print(F("[INFO]  Connecting to SPS30... "));
    sensor.begin(Wire, SPS30_I2C_ADDR_69);
    sensor.stopMeasurement();
    delay(50);
    error = sensor.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_UINT16);
    
    if (error != NO_ERROR) {
        Serial.println(F("FAILED!"));
        digitalWrite(SPS_LED_PIN, LOW);
    } else {
        Serial.println(F("OK!"));
        digitalWrite(SPS_LED_PIN, HIGH); // SPS30 Green Light ON
    }

    // --- 2. RTC Setup ---
    Serial.print(F("[INFO]  Connecting to RTC... "));
    if (!rtc.begin()) {
        Serial.println(F("FAILED!"));
        digitalWrite(RTC_LED_PIN, LOW);
    } else {
        Serial.println(F("OK!"));
        digitalWrite(RTC_LED_PIN, HIGH); // RTC Green Light ON
    }

    // --- 3. SD Card Setup ---
    Serial.print(F("[INFO]  Initializing SD Card... "));
    bool sdSuccess = true;
    if (digitalRead(CD_PIN) == HIGH) {
        Serial.println(F("FAILED! (Card Not Inserted)"));
        sdSuccess = false;
    } else {
        digitalWrite(OE_PIN, HIGH); // Enable SD Shifter
        delay(10);
        if (!sd.begin(CS_PIN)) {
            Serial.println(F("FAILED! (SPI Init Error)"));
            sdSuccess = false;
        }
        digitalWrite(OE_PIN, LOW); // Disable Level Shifter
    }
    
    if (sdSuccess) {
        digitalWrite(OE_PIN, HIGH); // Enable SD Shifter
        int fileIndex = 0;
        while (fileIndex < 1000) {
            snprintf(logFileName, sizeof(logFileName), "LOG%03d.CSV", fileIndex);
            if (!sd.exists(logFileName)) {
                break; 
            }
            fileIndex++;
        }

        dataFile = sd.open(logFileName, FILE_WRITE);
        if (dataFile) {
            dataFile.println(F("Timestamp,PM1.0,PM2.5,PM4.0,PM10.0,NC0.5,NC1.0,NC2.5,NC4.0,NC10.0,TypicalSize"));
            dataFile.close();
            Serial.print(F("OK! File created: "));
            Serial.println(logFileName);
            digitalWrite(SD_LED_PIN, HIGH); // SD Card Green Light ON
        } else {
            Serial.println(F("FAILED! (Could not create file)"));
            digitalWrite(SD_LED_PIN, LOW);
        }
        digitalWrite(OE_PIN, LOW); // Disable SD Shifter
    } else {
        digitalWrite(SD_LED_PIN, LOW);
    }

    Serial.println(F("------------------------------------------"));
    Serial.println(F(">>> Setup complete. Logging actively to SD card... <<<"));
    Serial.println(F("------------------------------------------"));
    
    delay(1500); 
}

void loop() {
    uint16_t mc1p0 = 0, mc2p5 = 0, mc4p0 = 0, mc10p0 = 0;
    uint16_t nc0p5 = 0, nc1p0 = 0, nc2p5 = 0, nc4p0 = 0, nc10p0 = 0;
    uint16_t typicalParticleSize = 0;
    
    delay(1000);

    // 1. Keep level shifter OFF during I2C sensor reads
    digitalWrite(OE_PIN, LOW);

    error = sensor.readMeasurementValuesUint16(mc1p0, mc2p5, mc4p0, mc10p0,
                                               nc0p5, nc1p0, nc2p5, nc4p0,
                                               nc10p0, typicalParticleSize);
    if (error != NO_ERROR) {
        digitalWrite(SPS_LED_PIN, LOW); // Turn off SPS30 LED on read error
        return; 
    } else {
        digitalWrite(SPS_LED_PIN, HIGH); // Ensure SPS30 LED stays ON
    }

    // 2. Read Real-Time Clock
    DateTime now = rtc.now();

    // 3. Turn ON level shifter briefly to write to SD Card
    digitalWrite(OE_PIN, HIGH);
    dataFile = sd.open(logFileName, FILE_WRITE);
    if (dataFile) {
        logDataStream(dataFile, now, mc1p0, mc2p5, mc4p0, mc10p0, nc0p5, nc1p0, nc2p5, nc4p0, nc10p0, typicalParticleSize);
        dataFile.close();
        digitalWrite(SD_LED_PIN, HIGH); // Ensure SD LED stays ON
    } else {
        digitalWrite(SD_LED_PIN, LOW);  // Turn off SD LED if write fails
    }
    digitalWrite(OE_PIN, LOW); // Turn OFF level shifter
}