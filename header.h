#ifndef MQTT_SEED_HEADER_H
#define MQTT_SEED_HEADER_H

// #include <Ticker.h>

#define WIFI_MAX_RETRIES 1500
#define WIFI_CONNECT_DELAY_MS 20


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

MQTT::Connect *connOpts;

void connectMqtt(void);
char* getClientId(void);
void connectWifi(void);
void subscribeMqttTopic(void);



void initPubSubClient()
{
    client = new PubSubClient(MQTT_HOST, MQTT_PORT);
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
        yield();
    }

    DEBUG_PRINTLN(STATE_WIFI_CONNECTED);
}

char* getDefaultTopic() {
    clientId = getClientId();
    clientTopic = (char* )malloc(strlen(clientId) + 6);
    memcpy(clientTopic, clientId, strlen(clientId));
    strcpy(clientTopic+strlen(clientId), "/data");
    return clientTopic;
}

void prepareClientIdAndClientTopic()
{
    clientId = getClientId();
    clientTopic = getDefaultTopic();

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
    uint8_t retries = 0;
    prepareClientIdAndClientTopic();

    connOpts = new MQTT::Connect(clientId);
    connOpts->set_auth(MQTT_USER, MQTT_PASS);
    connOpts->set_keepalive(30);

    int result;
    // Connect to mqtt broker
    while (true)
    {
        DEBUG_PRINT(STATE_MQTT_CONNECTING);
        DEBUG_PRINT(" [");
        DEBUG_PRINT(clientId);
        DEBUG_PRINT(", ");
        DEBUG_PRINT(clientTopic);
        DEBUG_PRINTLN(" ]");
        yield();
        result = client->connect(*connOpts);
        if (result == 1)
        {
            break;
        }


        DEBUG_PRINT(retries++);
        DEBUG_PRINT(" ");
        if (retries == 30) {
            abort();
        }

        delay(200);
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
        yield();
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
    if (millis() - prevMillisPub < DELAY_PUBLISH)
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
        yield();        
    }

    DEBUG_PRINTLN("PUBLISHED OK.");
}


#endif // MQTT_SEED_HEADER_H/
