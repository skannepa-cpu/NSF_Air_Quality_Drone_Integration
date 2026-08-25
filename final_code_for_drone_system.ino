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

// --- 3 LED Pin Configuration ---
#define RTC_LED_PIN  4   // ON = RTC Working, OFF = Failed
#define SD_LED_PIN   6   // ON = SD Working,  OFF = Failed
#define SPS_LED_PIN  8   // ON = SPS30 Working, OFF = Failed

SensirionI2cSps30 sensor;
RTC_DS3231 rtc;
SdFat sd; 
File dataFile;

static int16_t error;

#define OE_PIN 5           
#define CS_PIN 10          
#define CD_PIN 7           
const int LLS = 1;         

// Store dynamic filename globally (LOG000.CSV through LOG999.CSV)
char logFileName[13] = "LOG000.CSV";

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
    pinMode(RTC_LED_PIN, OUTPUT);
    pinMode(SD_LED_PIN, OUTPUT);
    pinMode(SPS_LED_PIN, OUTPUT);

    Wire.begin();
    
    // --- RTC Setup ---
    if (!rtc.begin()) {
        digitalWrite(RTC_LED_PIN, LOW); 
    } else {
        digitalWrite(RTC_LED_PIN, HIGH); 
        
        // UNCOMMENT the line below to FORCE set time to computer time right now upon upload:
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        
        // Or hardcode your exact current time manually (Year, Month, Day, Hour, Min, Sec):
        // rtc.adjust(DateTime(2026, 7, 22, 14, 33, 0));
    }
    
    pinMode(OE_PIN, OUTPUT);
    pinMode(CD_PIN, INPUT_PULLUP);
    
    // --- SD Card Setup ---
    bool sdSuccess = true;
    if (digitalRead(CD_PIN) == HIGH) {
        sdSuccess = false;
    } else {
        digitalWrite(OE_PIN, LLS ? HIGH : LOW);
        if (!sd.begin(CS_PIN)) {
            sdSuccess = false;
        }
    }
    
    if (!sdSuccess) {
        digitalWrite(SD_LED_PIN, LOW);
    } else {
        // --- Generate unique file name on each boot ---
        int fileIndex = 0;
        while (fileIndex < 1000) {
            snprintf(logFileName, sizeof(logFileName), "LOG%03d.CSV", fileIndex);
            if (!sd.exists(logFileName)) {
                break; // Found a file number that doesn't exist yet!
            }
            fileIndex++;
        }

        // Create the new file and print CSV header
        dataFile = sd.open(logFileName, FILE_WRITE);
        if (dataFile) {
            dataFile.println(F("Timestamp,PM1.0,PM2.5,PM4.0,PM10.0,NC0.5,NC1.0,NC2.5,NC4.0,NC10.0,TypicalSize"));
            dataFile.close();
            digitalWrite(SD_LED_PIN, HIGH);
        } else {
            digitalWrite(SD_LED_PIN, LOW);
        }
    }

    // --- SPS30 Setup ---
    sensor.begin(Wire, SPS30_I2C_ADDR_69);
    error = sensor.stopMeasurement();
    if (error == NO_ERROR) {
        error = sensor.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_UINT16);
    }
    
    if (error != NO_ERROR) {
        digitalWrite(SPS_LED_PIN, LOW);
    } else {
        digitalWrite(SPS_LED_PIN, HIGH);
    }
    
    delay(100);
}

void loop() {
    uint16_t dataReadyFlag = 0;
    uint16_t mc1p0 = 0, mc2p5 = 0, mc4p0 = 0, mc10p0 = 0;
    uint16_t nc0p5 = 0, nc1p0 = 0, nc2p5 = 0, nc4p0 = 0, nc10p0 = 0;
    uint16_t typicalParticleSize = 0;
    
    delay(1000);
    
    error = sensor.readDataReadyFlag(dataReadyFlag);
    if (error == NO_ERROR) {
        error = sensor.readMeasurementValuesUint16(mc1p0, mc2p5, mc4p0, mc10p0,
                                                   nc0p5, nc1p0, nc2p5, nc4p0,
                                                   nc10p0, typicalParticleSize);
    }
    
    if (error != NO_ERROR) {
        digitalWrite(SPS_LED_PIN, LOW); 
        return; 
    } else {
        digitalWrite(SPS_LED_PIN, HIGH);
    }
    
    DateTime now = rtc.now();

    // Open the unique log file created in setup()
    dataFile = sd.open(logFileName, FILE_WRITE);
    if (dataFile) {
        logDataStream(dataFile, now, mc1p0, mc2p5, mc4p0, mc10p0, nc0p5, nc1p0, nc2p5, nc4p0, nc10p0, typicalParticleSize);
        dataFile.close();
        digitalWrite(SD_LED_PIN, HIGH);
    } else {
        digitalWrite(SD_LED_PIN, LOW); 
    }
}