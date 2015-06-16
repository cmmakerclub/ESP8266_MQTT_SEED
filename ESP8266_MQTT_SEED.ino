#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>


#define DEBUG_MODE


#define DEBUG_PRINTER Serial

#define CLIENT_ID_PREFIX "esp8266-"
#define DEVICE_NAME "NAT"


#define LED_PIN 1 // <<<==== 1 = TX0 PIN 

const char *ssid = "OpenWrt_NAT_500GP.101";
const char *pass = "activegateway";


#include "header.h"

Ticker publisher;

unsigned long prevMillisPub = 0;

// IPAddress server(128,199,104,122);

// PubSubClient client("m20.cloudmqtt.com", 17380);
// PubSubClient client("128.199.104.122", 1883);


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
    if (millis() - prevMillisPub < 3000)
    {
        return;
    }

    prevMillisPub = millis();
    static unsigned long counter = 0;

    String payload = "{\"millis\":";
    payload += millis();
    payload += ",\"counter\":";
    payload += ++counter;
    payload += ", \"nickname\":";
    payload += " \"";
    payload += DEVICE_NAME;
    payload += "\"";
    payload += "}";


    if (client.publish(clientTopic, payload))
    {
        DEBUG_PRINTLN("PUBLISHED OK.");
    }
    else
    {
        DEBUG_PRINTLN("PUBLISHED ERROR.");
    }
}

void setup()
{
    client.set_callback(callback);

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
    client.loop();
    if (client.connected())
    {
        reconnectWifiIfLinkDown();
        fn_publisher();
    }
    else
    {
        connectMqtt();
        subscribeMqttTopic();
    }

}
