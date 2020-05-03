#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "conf.h"
#include <iot.h>
#include <ArduinoJson.h>

IOT::IOT(WiFiClientSecure& wifi_client, PubSubClient& pubsub_client)
{
    this -> _wifi_client = &wifi_client;
    this -> _pubsub_client = &pubsub_client;


}

void IOT::setup(void)
{
    _pubsub_client->setServer(IOT_ENDPOINT,8883);
    _wifi_client->setCACert(CA_CERT);
    _wifi_client->setCertificate(IOT_CERT);
    _wifi_client->setPrivateKey(IOT_PRIVATE_KEY);
    _pubsub_client->connect(DEVICE_NAME);
    while(!_pubsub_client->connected())
    {
        _pubsub_client->connect(DEVICE_NAME);
        sleep(5);
    }
}

void IOT::print_on_publish(bool goal)
{
    _PRINT = goal;
}


bool IOT::publish(String topic, String message)
{
    bool status;
    char * c_topic = new char [topic.length()+1];
    strcpy(c_topic,topic.c_str());

    char * c_message = new char [message.length()+1];
    strcpy(c_message,message.c_str());

    while(!_pubsub_client->connected())
    {
        _pubsub_client->connect(DEVICE_NAME);
        sleep(5);
    }
    Serial.println(c_message);
    Serial.println(strlen(c_message));
    Serial.println(String("Max payload size ") + String(MQTT_MAX_PACKET_SIZE));
    status = _pubsub_client->publish(c_topic,c_message);
    delete c_topic;
    delete c_message;

    return status;
}

void IOT::print_callback(char* topic, byte* message, unsigned int length)
{
    Serial.println(String("Message arrived on topic") + String(topic));
    if (_PRINT)
    {
        for (int ii =0; ii< length; ii++)
        {
            Serial.print((char)message[ii]);
        }
        Serial.println();
    }
}