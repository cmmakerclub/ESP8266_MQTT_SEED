#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_MAX_RETRIES 150
#define WIFI_CONNECT_DELAY_MS 100

#define CLIENT_ID_PREFIX "esp8266-"

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



void setup()
{
  // Setup console
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();

  client.set_callback(callback);

  WiFi.begin(ssid, pass);
  
  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED)) {
    Serial.print(".");   
    if(retries > WIFI_MAX_RETRIES){
      Serial.println("Reset due to WIFI_MAX_RETRIES");
      abort();
    }
    retries++;
    delay(WIFI_CONNECT_DELAY_MS); 
  }

  Serial.println("\n WiFi connected");

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

