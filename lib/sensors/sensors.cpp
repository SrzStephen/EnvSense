#include "sensors.h"
#include "string.h"
#include "MHZ19.h"
#include "PMS.h"
#include "bsec.h"
#include "ArduinoJson.h"
#include "conf.h"
Sensors::Sensors(PMS& pms7003,Bsec& bme680,MHZ19& mhz19, NTPClient& ntpclient)
{
    this ->_BME680 = &bme680;
    _MHZ19 = &mhz19;
    this ->_PMS7003 = &pms7003;
    this -> _ntpclient = &ntpclient;
    _first_report_time = 0;
    _messages = 0;
}


JsonObject Sensors::generate_mhz19(JsonObject mhz19)
{
JsonObject mhz19_fields = mhz19.createNestedObject("fields");
mhz19_fields["co2_ppm"] = _MHZ19->getCO2();
mhz19_fields["co2_raw"] = _MHZ19->getCO2Raw();
mhz19_fields["temperature"] = _MHZ19->getTemperature();
return mhz19;

}
JsonObject Sensors::generate_pms(JsonObject pms)
{
    PMS::DATA data;
    bool success = _PMS7003->readUntil(data,30);
    if(!success)
    {
        // return a blank object
        return pms;
    }

    JsonObject pms_fields = pms.createNestedObject("fields");
    pms_fields["pm_1.0"] = data.PM_AE_UG_1_0;
    pms_fields["pm_2.5"] = data.PM_AE_UG_2_5;
    pms_fields["pm_10.0"] = data.PM_AE_UG_10_0;
    pms_fields["pm_1.0_std"] = data.PM_SP_UG_1_0;
    pms_fields["pm_2.5_std"] = data.PM_SP_UG_2_5;
    pms_fields["pm_10.0_std"] = data.PM_SP_UG_10_0;
    return pms;

}
JsonObject Sensors::generate_bme680(JsonObject bme680)
{
    int attempts = 0;
    // try to generate the message twice
    while (attempts  < MAX_ATTEMPTS)
    {
        attempts +=1;
        if (_BME680->run())
        {
            JsonObject bme680_fields = bme680.createNestedObject("fields");
            bme680_fields["raw_temperature"] = _BME680->rawTemperature;
            bme680_fields["pressure"] = _BME680->pressure;
            bme680_fields["raw_humidity"] = _BME680->rawHumidity;
            bme680_fields["gas_resistance"] = _BME680->gasResistance;
            bme680_fields["IAQ"] = _BME680->iaq;
            bme680_fields["IAQ_Accuracy"] = _BME680->iaqAccuracy;
            bme680_fields["static_IAQ"] = _BME680->staticIaq;
            bme680_fields["CO2"] = _BME680->co2Equivalent;
            bme680_fields["breath_VOC"] = _BME680->breathVocEquivalent;
            bme680_fields["temperature"] = _BME680->temperature;
            bme680_fields["humidity"] = _BME680->humidity;
            return bme680;
        }
        sleep(1);
    }
    // default if failed retun a blank object 
    return bme680;
}


String Sensors::generate_payload()
{
const size_t capacity = JSON_ARRAY_SIZE(3) + 3*JSON_OBJECT_SIZE(1) + 4*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(11) + JSON_OBJECT_SIZE(12);
DynamicJsonDocument doc(capacity);

doc["collector"] = "EnvSense";
doc["platform"] = "arduino";
JsonArray data = doc.createNestedArray("data");

JsonObject o_bme680 = generate_bme680(data.createNestedObject());
JsonObject bme680_tags = o_bme680.createNestedObject("tags");
bme680_tags["source"] = "BME680";


JsonObject o_mhz19 = generate_mhz19(data.createNestedObject());
JsonObject mhz19_tags = o_mhz19.createNestedObject("tags");
mhz19_tags["source"] = "MH-Z19";


JsonObject o_pms7003 = generate_pms(data.createNestedObject());
JsonObject pms_tags = o_pms7003.createNestedObject("tags");
pms_tags["source"] = "PMS7003";


JsonObject tags = doc.createNestedObject("tags");
tags["arduino_version"] = __VERSION__;
tags["build_datetimetime"] = compile_datetime;
doc["destination_type"] = "MQTT";
doc["timestamp"] = _ntpclient->getEpochTime();
doc["version"] = SOFTWARE_VERSION;

// calculated fields
if (_first_report_time ==0)
{
    _first_report_time = _ntpclient->getEpochTime();
}
//ntpclinet returns millis, we want seconds.
doc["uptime"] = (_first_report_time - _ntpclient->getEpochTime())/1000;
_messages +=1;
doc["messages"] = _messages;
String output;
serializeJson(doc, output);
return output;
}

bool Sensors::check_all_sensors_status(void)
{
    if(!check_mhz19() || !check_ntp() || !check_bme() || !check_mhz19())
    {
        return false;
    }
    return true;
}
bool Sensors::check_bme(void)
{
    // treating BME and BSEC warnings as errors.
    String output;
    Serial.println("Checking BME680");
  if (_BME680->status != BSEC_OK) 
  {
        output = "BSEC library error " + String(_BME680->status);
        Serial.println(output);
        return false;
  }
  if (_BME680->bme680Status != BME680_OK)
  {
        output = "BSEC library error " + String(_BME680->bme680Status);
        Serial.println(output);
        return false;
  }
    return true;
}

bool Sensors::check_mhz19(void)
{
    Serial.println("Checking MHZ19");
    String output;
    // dummy call
    _MHZ19->getCO2();
    // check the error code produced

    if (_MHZ19->errorCode != 1)
    {
        output = "MHZ19 failure with error code" + String(_MHZ19->errorCode);
        Serial.println(output);
        return false;
    }

    return true;
}

bool Sensors::check_pms(void)
{
    Serial.println("Checking PMS7003");
    PMS::DATA data;
    bool success = _PMS7003->readUntil(data,30);
    if(!success)
    {
    String output;
    output = "PMS7003 failed to gather data within 30 seconds";
    Serial.print(output);
    }
    return success;
}

bool Sensors::check_ntp(void)
{
    Serial.println("Checking NTP");
    // get epoch time and check whether it's greater than some time in the past (5/2/2020).
    long epochtime = _ntpclient->getEpochTime();
    bool success= epochtime > 1588431841;
    if(!success)
    {
        String output;
        output = "NTP didn't give a correct unix epoch time, got " + String(epochtime);
    }
    return success;
}