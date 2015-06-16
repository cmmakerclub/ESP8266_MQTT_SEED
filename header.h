#ifndef MQTT_SEED_HEADER_H
#define MQTT_SEED_HEADER_H

// #include <Ticker.h>

#define WIFI_MAX_RETRIES 150
#define WIFI_CONNECT_DELAY_MS 1000


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

char* clientId;
char* clientTopic;
PubSubClient *client;


#ifdef DEBUG_MODE
#define PRODUCTION_MODE
#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...) {}
#define DEBUG_PRINTLN(...) {}
#endif

unsigned long prevMillisPub = 0;

StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();


void connectMqtt(void);
char* getClientId(void);
void connectWifi(void);
void subscribeMqttTopic(void);



void initPubSubClient()
{
    client = new PubSubClient("128.199.104.122", 1883);
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
    char* buff = (char* )malloc(len + result.length() + 1);
    memcpy(buff, CLIENT_ID_PREFIX, len);
    strcpy(buff + len, (char*)result.c_str());

    return buff;
}


void initHardware()
{
#ifdef DEBUG_MODE
    // Setup console
    Serial.begin(115200);
    DEBUG_PRINTLN("\n");
    delay(10);
#endif

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

void connectMqtt()
{
    DEBUG_PRINTLN(STATE_MQTT_CONNECTING);
    int result;
    MQTT::Connect connectObject = MQTT::Connect(clientId);
    // connectObject.set_auth("test3", "test3");
    // Connect to mqtt broker
    while (true)
    {
        result = client->connect(connectObject);
        if (result == 1)
        {
            break;
        }

        DEBUG_PRINTLN(result);
        DEBUG_PRINTLN(STATE_MQTT_CONNECTING);
        delay(100);
    }
    DEBUG_PRINTLN(STATE_MQTT_CONNECTED);
}

void subscribeMqttTopic()
{
    int result;
    DEBUG_PRINTLN(STATE_MQTT_SUBSCRIBING);
    // Subscibe to the topic
    while (true)
    {
        result = client->subscribe(clientId);
        if (result)
        {
            break;
        }

        DEBUG_PRINTLN(result);;
        DEBUG_PRINTLN(STATE_MQTT_SUBSCRIBING);
        delay(1000);
    }
    DEBUG_PRINTLN(STATE_MQTT_SUBSCRIBED);
    DEBUG_PRINTLN(clientId);
}

void reconnectMqtt()
{
    connectMqtt();
    subscribeMqttTopic();
}


void publishMqttData(const char* clientTopic, JsonObject &r)
{
    if (millis() - prevMillisPub < 1000)
    {
        return;
    }

    prevMillisPub = millis();

    static char payload[256];

    static long counter = 0;
    root["counter"] = ++counter;

    root.printTo(payload, sizeof(payload));

    while(!client->publish(clientTopic, String(payload)))
    {
        DEBUG_PRINTLN("PUBLISHED ERROR.");
        delay(100);
    }

    DEBUG_PRINTLN("PUBLISHED OK.");
}


#endif // MQTT_SEED_HEADER_H/
