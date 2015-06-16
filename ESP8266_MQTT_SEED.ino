#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


#define DEBUG_MODE


#define DEBUG_PRINTER Serial

#define CLIENT_ID_PREFIX "esp8266-"
#define DEVICE_NAME "NAT"


#define LED_PIN 1 // <<<==== 1 = TX0 PIN 

const char *ssid = "OpenWrt_NAT_500GP.101";
const char *pass = "activegateway";


#include "header.h"

void callback(const MQTT::Publish& pub)
{
    // MQTT SUBSCRIBE
    if (pub.payload_string() == "0")
    {
        DEBUG_PRINTLN("GOT STRING 0...");
    }
    else if (pub.payload_string() == "1")
    {
        DEBUG_PRINTLN("GOT STRING 1..");
    }
    else
    {
        DEBUG_PRINT(pub.topic());
        DEBUG_PRINT(" => ");
        DEBUG_PRINTLN(pub.payload_string());
    }
}

void fn_publisher()
{

    static unsigned long counter = 0;
    root["millis"] = millis();
    root["counter"] = ++counter;
    root["nickname"] = DEVICE_NAME;

    publishMqttData(clientTopic, root);

}

void setup()
{

    initHardware();

    connectWifi();

    initPubSubClient();
    prepareClientIdAndClientTopic();
    connectMqtt();
    subscribeMqttTopic();

    // READY
    DEBUG_PRINTLN(STATE_READY_TO_GO);
}


void loop()
{
    reconnectWifiIfLinkDown();

    client->loop();
    if (client->connected())
    {
        fn_publisher();
    }
    else
    {
        reconnectMqtt();
    }
}
