# EnvSense 
![](image)
* Hey since i'm working from home more, I wonder what that's doing to me*

## About
Measures environment stuff.

## BOM

| Item          | Use                                   |
|:------         |:-----------------------------------  |
|PMS7003        | Particle Sensor                       |
|MH-Z19         | C02 Sensor                            |
|BME-680        | Temperature, Humidity, IAQ Sensor     |
|ESP32-WROOM-Dev| Microctontroller                      | 


## Future Goals
* Add in a ```MICS-6814``` to get more gas sensing
* Finalise data fomat

## Installation
To start with modify ```lib/conf/conf.h``` with required viariables including WiFi username/password, IOT certificates, keys and endpoints.

```
platformio run
platformio -t upload
```

## Expected stdout

```
STARTING
Starting WiFi
Failed to connect to WiFi
Connected to WiFi with IP address 192.168.0.180
Updating time
Checking sensors
Checking MHZ19
Checking NTP
Checking BME680
Checking MHZ19
{"collector":"EnvSense","platform":"arduino","data":[{"fields":{"raw_temperature":22.98,"pressure":101381,"raw_humidity":74.02,"gas_resistance":16106,"IAQ":25,"IAQ_Accuracy":0,"static_IAQ":25,"CO2":500,"breath_VOC":0.5,"temperature":22.98,"humidity":74.02},"tags":{"source":"BME680"}},{"fields":{"co2_ppm":702,"co2_raw":48641,"temperature":25},"tags":{"source":"MH-Z19"}},{"fields":{"pm_1.0":3,"pm_2.5":6,"pm_10.0":6,"pm_1.0_std":3,"pm_2.5_std":6,"pm_10.0_std":6},"tags":{"source":"PMS7003"}}],"tags":{"arduino_version":"5.2.0","build_datetimetime":"May  3 2020 20:44:11"},"destination_type":"MQTT","timestamp":1588509902,"version":"1.0.0","uptime":0,"messages":1}
```

## Payload Example
Payload looks a bit weird at the moment, I'm hoping to nail down a proper schema to be used with a few other projects.
```
{
  "collector": "EnvSense",
  "platform": "arduino",
  "data": [
    {
      "fields": {
        "raw_temperature": 23.2,
        "pressure": 101381,
        "raw_humidity": 73.063,
        "gas_resistance": 7986,
        "IAQ": 25,
        "IAQ_Accuracy": 0,
        "static_IAQ": 25,
        "CO2": 500,
        "breath_VOC": 0.5,
        "temperature": 23.2,
        "humidity": 73.063
      },
      "tags": {
        "source": "BME680"
      }
    },
    {
      "fields": {
        "co2_ppm": 880,
        "co2_raw": 48421,
        "temperature": 25
      },
      "tags": {
        "source": "MH-Z19"
      }
    },
    {
      "fields": {
        "pm_1.0": 2,
        "pm_2.5": 2,
        "pm_10.0": 3,
        "pm_1.0_std": 2,
        "pm_2.5_std": 2,
        "pm_10.0_std": 3
      },
      "tags": {
        "source": "PMS7003"
      }
    }
  ],
  "tags": {
    "arduino_version": "5.2.0",
    "build_datetimetime": "May  3 2020 20:37:32"
  },
  "destination_type": "MQTT",
  "timestamp": 1588509634,
  "version": "1.0.0",
  "uptime": 0,
  "messages": 1
}
```