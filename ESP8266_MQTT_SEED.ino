#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_MAX_RETRIES 150
#define WIFI_CONNECT_DELAY_MS 100

// #define DEBUG_MODE

#define CLIENT_ID_PREFIX "esp8266-"

#define STATE_WIFI_CONNECTING   0
#define STATE_WIFI_CONNECTED    1
#define STATE_MQTT_CONNECTED    2
#define STATE_MQTT_SUBSCRIBED   3
#define STATE_READY_TO_GO       4


#define LED_PIN 1 // <<<==== 1 = TX0 PIN 

const char *ssid = "OpenWrt_NAT_500GP.101";
const char *pass = "activegateway";

char clientId[35];

IPAddress server(128,199,191,223);

PubSubClient client(server);

void callback(const MQTT::Publish& pub) {

  Serial.print(pub.topic());
  Serial.print(" => ");
  Serial.println(pub.payload_string());
  
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


void blink_ms(uint8_t millisecs) {
  digitalWrite(LED_PIN, HIGH);
  delay(millisecs);
  digitalWrite(LED_PIN, LOW);  
  delay(millisecs);
}

void visualNotify(uint8_t state) {


    if (state == STATE_WIFI_CONNECTING) {
      #ifdef DEBUG_MODE      
        Serial.print(".");
      #else
        blink_ms(30);
      #endif  
    }
    else if (state == STATE_MQTT_CONNECTED) {
        blink_ms(30);
    }
    else if (state == STATE_MQTT_SUBSCRIBED) {
        blink_ms(30);
    }
    else if (state == STATE_WIFI_CONNECTED) {
      #ifdef DEBUG_MODE      
        Serial.print("Wifi connected.");
      #else
        blink_ms(100);
        delay(50);
        blink_ms(100);
        delay(50);
        blink_ms(100);
      #endif  
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
      Serial.println("Reset due to WIFI_MAX_RETRIES");
      abort();
    }
    retries++;
    delay(WIFI_CONNECT_DELAY_MS); 
  }


  visualNotify(STATE_WIFI_CONNECTED);


  macToStr(clientId);

  Serial.println(clientId);

  
  while(!client.connect(clientId)){
    Serial.print("Connect...");
    delay(500);
  }
  
  Serial.println("MQTT Connected");
  
  while(!client.subscribe("/pao/esp8266")){
    Serial.println("Subscribe...");
    delay(500);
  }
  
}

void loop()
{
  client.loop();
}

