#include "conf.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "MHZ19.h"
#include "PMS.h"
#include "ArduinoJson.h"
#include <Wire.h>
#include "string.h"
#include "bsec.h"
#include "sensors.h"
#include <WiFiUdp.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <iot.h>
SoftwareSerial SoftSerial;
PMS pms(Serial2);
Bsec iaqSensor;
String output;
MHZ19 myMHZ19;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
Sensors sensor(pms,iaqSensor,myMHZ19,timeClient);
#define MQTT_MAX_PACKET_SIZE 2048
WiFiClientSecure secureClient = WiFiClientSecure();
PubSubClient mqttClient(secureClient);
IOT iotclient(secureClient,mqttClient);

void setup()
{
  sleep(1);
  Serial.begin(9600);
  Serial.println("STARTING");
    // MHZ19
  SoftSerial.begin(9600, SWSERIAL_8N1, 18, 19, false, 256);
  myMHZ19.begin(SoftSerial);
  // don't use interrupts on tx
  SoftSerial.enableIntTx(false);
  // PMS Serial
  Serial2.begin(9600,SERIAL_8N1,16,17);
  // i2c stuff for BME680
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
  sleep(10);
  Serial.println("Starting WiFi");
  //Setup WiFi
   //WiFi.persistent(false);
   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
   while (WiFi.status() !=WL_CONNECTED )
   {
     Serial.println("Failed to connect to WiFi");
     
     sleep(5);
   }
  Serial.print("Connected to WiFi with IP address ");
  Serial.println(WiFi.localIP());
  Serial.println("Updating time");
  timeClient.forceUpdate();
  iotclient.setup();
  //give sensors time to setup before checking their status.
  Serial.println("Checking sensors");
  myMHZ19.getCO2();
  sensor.check_all_sensors_status();
  sleep(5);
  iotclient.print_on_publish(true);
  //sleep(600);
}
String payload_string;
void loop()
{
payload_string = sensor.generate_payload();

//Serial.println(payload_string);
//Serial.print(mqttClient.publish("topic/",payload_string.c_str()));
Serial.println(iotclient.publish("foo/",payload_string));
sleep(30);
}