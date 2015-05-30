#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#define WIFI_MAX_RETRIES 150
#define WIFI_CONNECT_DELAY_MS 100
const char *ssid = "OpenWrt_NAT_500GP.101";
const char *pass = "activegateway";

IPAddress server(128,199,191,223);

PubSubClient client(server);

// Callback function
void callback(const MQTT::Publish& pub) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.

  // Copy the payload to a new message
  MQTT::Publish newpub("/pao/esp8266", pub.payload(), pub.payload_len());
  client.publish(newpub);
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


  if (client.connect("arduinoClient")) {
    client.publish("/pao/esp8266","hello world");
    client.subscribe("/pao/esp8266");
  }
}

void loop()
{
  client.loop();
}

