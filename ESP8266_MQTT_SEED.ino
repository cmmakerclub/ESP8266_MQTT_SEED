#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_MAX_RETRIES 150
#define WIFI_CONNECT_DELAY_MS 100

#define DEBUG_MODE

#ifndef DEBUG_MODE
  #define PRODUCTION_MODE
#endif

#define CLIENT_ID_PREFIX "esp8266-"

#define STATE_WIFI_CONNECTING    0
#define STATE_WIFI_CONNECTED     1

#define STATE_MQTT_CONNECTING    2
#define STATE_MQTT_CONNECTED     3
#define STATE_MQTT_SUBSCRIBING   4
#define STATE_MQTT_SUBSCRIBED    5

#define STATE_READY_TO_GO        6

//-------
#define STATE_GOT_CLIENT_ID      7

//-------

#define LED_PIN 1 // <<<==== 1 = TX0 PIN 

const char *ssid = "OpenWrt_NAT_500GP.101";
const char *pass = "activegateway";

char clientId[35];

IPAddress server(128,199,191,223);

PubSubClient client(server);

void callback(const MQTT::Publish& pub) {

//  Serial.print(pub.topic());
//  Serial.print(" => ");
//  Serial.println(pub.payload_string());
  
}

String macToStr(char* target)
{
  uint8_t mac[6];
  WiFi.macAddress(mac);  
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  
  uint8_t len = strlen(CLIENT_ID_PREFIX);
  memcpy(target, CLIENT_ID_PREFIX, len);
  strcpy(target+len, (char*)result.c_str());  

  return result;
}

#ifndef DEBUG_MODE
void blink_ms(uint8_t millisecs) {
  digitalWrite(LED_PIN, HIGH);
  delay(millisecs);
  digitalWrite(LED_PIN, LOW);  
  delay(millisecs);
}
#endif

void visualNotify(uint8_t state) {

    if (state == STATE_WIFI_CONNECTING) {
      #ifdef DEBUG_MODE      
        Serial.print(".");
      #else
        blink_ms(30);
      #endif  
    }
    else if (state == STATE_WIFI_CONNECTED) {
      #ifdef DEBUG_MODE      
        Serial.print("\nWifi connected.");
      #else
        //  DO NOTTHING.
      #endif  
    }
    else if (state == STATE_GOT_CLIENT_ID) {
      #ifdef DEBUG_MODE
        Serial.println(clientId);
      #endif
    }
    else if (state == STATE_MQTT_CONNECTED) {
      #ifdef DEBUG_MODE  
        Serial.println("\nMQTT Connected.");
      #else  
        blink_ms(30);
      #endif
    }    
    else if (state == STATE_MQTT_SUBSCRIBING) {
      #ifdef DEBUG_MODE
        Serial.println("Subscibing");
      #else
        blink_ms(30);
      #endif
    }
    else if (state == STATE_MQTT_SUBSCRIBED) {
      #ifdef DEBUG_MODE
        Serial.println("Subscribed...");
      #else  
        blink_ms(30);
      #endif
    }    
    else if (state == STATE_READY_TO_GO) {
      #ifdef DEBUG_MODE
        Serial.println("READY TO GO");
      #else
        blink_ms(100);
        delay(50);
        blink_ms(100);
        delay(50);
        blink_ms(100);        
      #endif
    }
    else {
      // UN-HANDLED
      // IMPOSIBLE TO REACH
    }
  
  
}

void setup()
{
  client.set_callback(callback);

  #ifdef DEBUG_MODE
    // Setup console
    Serial.begin(115200);
    Serial.println();
    Serial.println();  
  #else
    pinMode(LED_PIN, OUTPUT);
  #endif
  
  delay(10);

  WiFi.begin(ssid, pass);
  
  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED)) {
    visualNotify(STATE_WIFI_CONNECTING);    
    if(retries > WIFI_MAX_RETRIES){
      Serial.println("\nReset due to WIFI_MAX_RETRIES");
      abort();
    }
    retries++;
    delay(WIFI_CONNECT_DELAY_MS); 
  }

  visualNotify(STATE_WIFI_CONNECTED);
  macToStr(clientId);
  
  // Connect to mqtt broker
  while(!client.connect(clientId)){
    visualNotify(STATE_MQTT_CONNECTING);
    delay(500);
  }
  visualNotify(STATE_MQTT_CONNECTED);
  
  // Subscibe to the topic
  while(!client.subscribe("/pao/esp8266")){
    visualNotify(STATE_MQTT_SUBSCRIBING);
    delay(500);
  }
  visualNotify(STATE_MQTT_SUBSCRIBED);

  visualNotify(STATE_READY_TO_GO);
  
}

void loop()
{
  client.loop();
}

