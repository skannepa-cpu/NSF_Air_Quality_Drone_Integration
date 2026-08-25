#include <Arduino.h>
#include <SensirionI2cSps30.h>
#include <Wire.h>

// macro definitions
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

SensirionI2cSps30 sensor;

static char errorMessage[64];
static int16_t error;

void setup() {

    Serial.begin(9600);
    while (!Serial) {
        delay(100);
    }
    Wire.begin();
    sensor.begin(Wire, SPS30_I2C_ADDR_69);
    sensor.stopMeasurement();
    sensor.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_UINT16);
    delay(100);
    Serial.print("Beginning Measurements");
}

void loop() {

    uint16_t dataReadyFlag = 0;
    uint16_t mc1p0 = 0;
    uint16_t mc2p5 = 0;
    uint16_t mc4p0 = 0;
    uint16_t mc10p0 = 0;
    uint16_t nc0p5 = 0;
    uint16_t nc1p0 = 0;
    uint16_t nc2p5 = 0;
    uint16_t nc4p0 = 0;
    uint16_t nc10p0 = 0;
    uint16_t typicalParticleSize = 0;
    delay(1000);
    error = sensor.readDataReadyFlag(dataReadyFlag);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readDataReadyFlag(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }    
    error = sensor.readMeasurementValuesUint16(mc1p0, mc2p5, mc4p0, mc10p0,
                                               nc0p5, nc1p0, nc2p5, nc4p0,
                                               nc10p0, typicalParticleSize);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readMeasurementValuesUint16(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    Serial.print("mc1p0: ");
    Serial.print(mc1p0);
    Serial.print("\t");
    Serial.print("mc2p5: ");
    Serial.print(mc2p5);
    Serial.print("\t");
    Serial.print("mc4p0: ");
    Serial.print(mc4p0);
    Serial.print("\t");
    Serial.print("mc10p0: ");
    Serial.print(mc10p0);
    Serial.print("\t");
    Serial.print("nc0p5: ");
    Serial.print(nc0p5);
    Serial.print("\t");
    Serial.print("nc1p0: ");
    Serial.print(nc1p0);
    Serial.print("\t");
    Serial.print("nc2p5: ");
    Serial.print(nc2p5);
    Serial.print("\t");
    Serial.print("nc4p0: ");
    Serial.print(nc4p0);
    Serial.print("\t");
    Serial.print("nc10p0: ");
    Serial.print(nc10p0);
    Serial.print("\t");
    Serial.print("typicalParticleSize: ");
    Serial.print(typicalParticleSize);
    Serial.println();
}
