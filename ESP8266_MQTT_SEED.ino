#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_MAX_RETRIES 150
#define WIFI_CONNECT_DELAY_MS 100

const char *ClientId  = "ESP8266_MQTT";
const char *ssid = "OpenWrt_NAT_500GP.101";
const char *pass = "activegateway";

IPAddress server(128,199,191,223);

PubSubClient client(server);

void callback(const MQTT::Publish& pub) {

  Serial.print(pub.topic());
  Serial.print(" => ");
  Serial.println(pub.payload_string());
  
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
  
  Serial.println("\nWiFi connected");
  
  while(!client.connect(ClientId)){
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

