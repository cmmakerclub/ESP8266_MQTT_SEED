#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


#define DEBUG_MODE


#define DEBUG_PRINTER Serial

#define CLIENT_ID_PREFIX "esp8266-"
#define DEVICE_NAME "CMMC-CLOUD-CMMC"

const char *ssid = "OpenWrt_NAT_500GP.101";
const char *pass = "activegateway";


#define MQTT_HOST "128.199.104.122"
// #define MQTT_HOST "m20.cloudmqtt.com"
#define MQTT_PORT 1883

#define MQTT_USER ""
#define MQTT_PASS ""

#define DELAY_PUBLISH 3000

// typedef struct {
//     PubSubClient *client;
//     MQTT::Connect *connOpts;
// } config_t;

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
#define dfsdfsdf
}

void fn_publisher()
{

  root["millis"] = millis();
  root["micros"] = micros();
  root["nickname"] = DEVICE_NAME;

  publishMqttData(clientTopic, root);

}

void setup()
{

  initHardware();
  connectWifi();
  initPubSubClient();
  client->set_callback(callback);
  connectMqtt();
  subscribeMqttTopic();

  // READY
  DEBUG_PRINTLN(STATE_READY_TO_GO);
}


void loop()
{
  reconnectWifiIfLinkDown();


  if (client->loop())
  {
    fn_publisher();
  }
  else
  {
    DEBUG_PRINTLN("CLIENT DISCONNECTD");
    reconnectMqtt();
  }
}
