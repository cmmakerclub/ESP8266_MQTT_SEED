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

unsigned long prevMillisPub = 0;


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


String preparePublishData()
{
    static unsigned long counter = 0;
    root["millis"] = millis();
    root["counter"] = ++counter;
    root["nickname"] = DEVICE_NAME;
}



void fn_publisher()
{
    if (millis() - prevMillisPub < 3000)
    {
        return;
    }

    prevMillisPub = millis();
    preparePublishData();

    publishMqttData(clientTopic);

}

void setup()
{

    initPubSubClient();

#ifdef DEBUG_MODE
    // Setup console
    Serial.begin(115200);
    DEBUG_PRINTLN("\n");
#endif

    delay(10);

    connectWifi();

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
