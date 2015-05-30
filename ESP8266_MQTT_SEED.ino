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
#define STATE_RESET              8
//-------

#define LED_PIN 1 // <<<==== 1 = TX0 PIN 

const char *ssid = "OpenWrt_NAT_500GP.101";
const char *pass = "activegateway";

char* clientId;
char* clientTopic;

IPAddress server(128,199,104,122);

PubSubClient client(server);

void callback(const MQTT::Publish& pub) {
  // MQTT SUBSCRIBE
  if (pub.payload_string() == "0") {
    #ifdef DEBUG_MODE
      Serial.println("GOT STRING 0...");
    #endif
  }
  else if (pub.payload_string() == "1"){
    #ifdef DEBUG_MODE
      Serial.println("GOT STRING 1..");
    #endif
  }
  else {
    #ifdef DEBUG_MODE
      Serial.print(pub.topic());
      Serial.print(" => ");
      Serial.println(pub.payload_string());
    #endif
  }
  
  #ifdef DEBUG_MODE
    MQTT::Publish newpub("/pao/esp8266", pub.payload(), pub.payload_len());
    client.publish(newpub);
  #endif
  
}

char* getClientId()
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
  char* buff = (char* )malloc(len+result.length()+1);
  memcpy(buff, CLIENT_ID_PREFIX, len);
  strcpy(buff+len, (char*)result.c_str());

  return buff;
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
    else if (state == STATE_MQTT_CONNECTING) {
      #ifdef DEBUG_MODE  
        Serial.println("\nMQTT connecting...");
      #else  
        blink_ms(30);
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
    else if (state == STATE_RESET) {
      #ifdef DEBUG_MODE
        Serial.println("\nReset due to WIFI_MAX_RETRIES");
      #else
        
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
      visualNotify(STATE_RESET);
      abort();
    }
    retries++;
    delay(WIFI_CONNECT_DELAY_MS); 
  }

  visualNotify(STATE_WIFI_CONNECTED);

  clientId = getClientId();

  clientTopic = (char* )malloc(strlen(clientId) + 10);
  memcpy(clientTopic, clientId, strlen(clientId));
  strcpy(clientTopic+strlen(clientId), "/data");


  // Connect to mqtt broker
  while(!client.connect(clientId)){
    visualNotify(STATE_MQTT_CONNECTING);
    delay(500);
  }
  visualNotify(STATE_MQTT_CONNECTED);
  
  // Subscibe to the topic
  while(!client.subscribe(clientId)){
    visualNotify(STATE_MQTT_SUBSCRIBING);
    delay(500);
  }

  visualNotify(STATE_MQTT_SUBSCRIBED);

  // READY
  visualNotify(STATE_READY_TO_GO);
  
}

void loop()
{
    static unsigned long counter = 0;
    
    String payload = "{\"millis\":";
    payload += millis();
    payload += ",\"counter\":";
    payload += counter;
    payload += "}";
    
    if (client.publish(clientTopic, payload)) {
      #ifdef DEBUG_MODE
        Serial.println("PUBLISHED OK");
      #endif
    }
    else {
      #ifdef DEBUG_MODE
        Serial.println("PUBLISHED ERROR");
      #endif
    }
  
  client.loop();
}

