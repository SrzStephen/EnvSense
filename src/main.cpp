#include <Arduino.h>
#include <SoftwareSerial.h>
#include "MHZ19.h"
#include "PMS.h"
#include "ArduinoJson.h"
#include "Adafruit_BME680.h"
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "string.h"
#include "bsec.h"
#include "sensors.h"

MHZ19 myMHZ19;
SoftwareSerial SoftSerial;
PMS pms(Serial2);
Bsec iaqSensor;
String output;


void checkIaqSensorStatus(void)
{
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
    } else {
      output = "BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      output = "BME680 error code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
    } else {
      output = "BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
    }
  }
}


void init_sensors()
{

  // MHZ19
  SoftSerial.begin(9600, SWSERIAL_8N1, 18, 19, false, 256);
  myMHZ19.begin(SoftSerial);
  // don't use interrupts on tx
  SoftSerial.enableIntTx(false);
  // PMS Serial
  Serial2.begin(9600,SERIAL_8N1,16,17);

  Wire.begin();
  sleep(1);

  iaqSensor.begin(0x77, Wire);

  bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };

  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();  
  Serial.println("All good?");
}


void setup()
{
  // terminal
  Serial.begin(9600);
init_sensors();
// sensors need time to warm up.


}

void loop()
{
  Serial.println("Begin loop");
  Serial.println(myMHZ19.getCO2(true, true));
  Serial.println(myMHZ19.errorCode);
  Serial.println(myMHZ19.getAccuracy(false));
  if (iaqSensor.run())
  {
    output += ", " + String(iaqSensor.rawTemperature);
    output += ", " + String(iaqSensor.pressure);
    output += ", " + String(iaqSensor.rawHumidity);
    output += ", " + String(iaqSensor.gasResistance);
    output += ", " + String(iaqSensor.iaq);
    output += ", " + String(iaqSensor.iaqAccuracy);
    output += ", " + String(iaqSensor.temperature);
    output += ", " + String(iaqSensor.humidity);
    output += ", " + String(iaqSensor.staticIaq);
    output += ", " + String(iaqSensor.co2Equivalent);
    output += ", " + String(iaqSensor.breathVocEquivalent);
    Serial.println(output);
    sleep(30);
  }
}