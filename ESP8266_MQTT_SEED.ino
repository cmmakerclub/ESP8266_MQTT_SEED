#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

#define WIFI_MAX_RETRIES 150
#define WIFI_CONNECT_DELAY_MS 1000

#define DEBUG_MODE


#define DEBUG_PRINTER Serial

#ifdef DEBUG_MODE
#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...) {}
#define DEBUG_PRINTLN(...) {}
#endif

#ifndef DEBUG_MODE
#define PRODUCTION_MODE
#endif

#define CLIENT_ID_PREFIX "esp8266-"
#define DEVICE_NAME "NAT"


#define STATE_WIFI_CONNECTING   "WiFi connecting..."
#define STATE_WIFI_CONNECTED    "WiFi connected"

#define STATE_MQTT_CONNECTING    "MQTT connecting..."
#define STATE_MQTT_CONNECTED     "MQTT connected!"
#define STATE_MQTT_SUBSCRIBING   "MQTT subscribing..."
#define STATE_MQTT_SUBSCRIBED    "MQTT subscribed!"

#define STATE_READY_TO_GO        "Ready!"

//-------
#define STATE_GOT_CLIENT_ID      "client id prepared"
#define STATE_RESET              "reset!"
//-------

#define LED_PIN 1 // <<<==== 1 = TX0 PIN 

const char *ssid = "OpenWrt_NAT_500GP.101";
const char *pass = "activegateway";

Ticker publisher;

char* clientId;
char* clientTopic;

unsigned long prevMillisPub = 0;

// IPAddress server(128,199,104,122);

// PubSubClient client("m20.cloudmqtt.com", 17380);
PubSubClient client("128.199.104.122", 1883);


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

char* getClientId()
{
    uint8_t mac[6];
    WiFi.macAddress(mac);
    String result;
    for (int i = 0; i < 6; ++i)
    {
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

#ifdef PRODUCTION_MODE
void blink_ms(uint8_t millisecs)
{
    digitalWrite(LED_PIN, HIGH);
    delay(millisecs);
    digitalWrite(LED_PIN, LOW);
    delay(millisecs);
}
#endif

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


void connectMqtt()
{
    DEBUG_PRINTLN(STATE_MQTT_CONNECTING);
    int result;
    MQTT::Connect connectObject = MQTT::Connect(clientId);
    // connectObject.set_auth("test3", "test3");
    // Connect to mqtt broker
    while(true)
    {
        result = client.connect(connectObject);
        if (result == 1)
        {
            break;
        }

        DEBUG_PRINTLN(result);
        DEBUG_PRINTLN(STATE_MQTT_CONNECTING);
        delay(500);
    }
    DEBUG_PRINTLN(STATE_MQTT_CONNECTED);
}

void subscribeMqttTopic()
{
    int result;
    DEBUG_PRINTLN(STATE_MQTT_SUBSCRIBING);
    // Subscibe to the topic
    while(true)
    {
        result = client.subscribe(clientId);
        if (result)
        {
            break;
        }

        DEBUG_PRINTLN(result);;
        DEBUG_PRINTLN(STATE_MQTT_SUBSCRIBING);
        delay(1000);
    }
    DEBUG_PRINTLN(STATE_MQTT_SUBSCRIBED);
}


void connectWifi()
{
    WiFi.begin(ssid, pass);

    int retries = 0;
    while ((WiFi.status() != WL_CONNECTED))
    {
        DEBUG_PRINTLN(STATE_WIFI_CONNECTING);
        if(retries > WIFI_MAX_RETRIES)
        {
            DEBUG_PRINTLN(STATE_RESET);
            abort();
        }
        retries++;
        delay(WIFI_CONNECT_DELAY_MS);
    }

    DEBUG_PRINTLN(STATE_WIFI_CONNECTED);
    delay(1000);
}

void prepareClientIdAndClientTopic()
{
    clientId = getClientId();

    clientTopic = (char* )malloc(strlen(clientId) + 10);
    memcpy(clientTopic, clientId, strlen(clientId));
    strcpy(clientTopic+strlen(clientId), "/data");

    DEBUG_PRINTLN(STATE_GOT_CLIENT_ID);
}


void reconnectWifiIfLinkDown()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG_PRINTLN("WIFI DISCONNECTED!!");
        connectWifi();
        connectMqtt();
        subscribeMqttTopic();
    }
}

void setup()
{
    client.set_callback(callback);

#ifdef DEBUG_MODE
    // Setup console
    Serial.begin(115200);
    DEBUG_PRINTLN();
    DEBUG_PRINTLN();
#else

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
