#ifndef MQTT_SEED_HEADER_H
#define MQTT_SEED_HEADER_H

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
PubSubClient client;


#ifdef DEBUG_MODE
#define PRODUCTION_MODE
#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...) {}
#define DEBUG_PRINTLN(...) {}
#endif



void connectMqtt(void);
char* getClientId(void);
void connectWifi(void);
void subscribeMqttTopic(void);

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
    while (true)
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


#endif // MQTT_SEED_HEADER_H/
