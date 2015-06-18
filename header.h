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

static boolean MQTT_CONNECTED_FLAG = 0;

char* clientId = NULL;
char* clientTopic = NULL;
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
#define PAYLOAD_SIZE 800

static StaticJsonBuffer<PAYLOAD_SIZE> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

MQTT::Connect *connOpts = NULL;

void connectMqtt(void);
char* getClientId(void);
void connectWifi(void);
void subscribeMqttTopic(void);



void initPubSubClient()
{
  if ( client != NULL) {
    delete client;
    client = NULL;
    DEBUG_PRINT("DELETING  ... client - ");
  }
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

  if (clientId != NULL) {
    DEBUG_PRINT("__DELETING  ... clientId - ");
    DEBUG_PRINTLN(clientId);
    free(clientId);
    clientId = NULL;
  }

  uint8_t len = strlen(CLIENT_ID_PREFIX);
  char* buff = (char* )malloc(len + result.length() + 1);
  memcpy(buff, CLIENT_ID_PREFIX, len);
  strcpy(buff + len, (char*)result.c_str());

  DEBUG_PRINT("BUFF LEN: ");
  DEBUG_PRINTLN(sizeof(buff));

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
    if (retries > WIFI_MAX_RETRIES)
    {
      DEBUG_PRINTLN(STATE_RESET);
      break;
    }
    retries++;
    delay(WIFI_CONNECT_DELAY_MS);
    yield();
  }

  DEBUG_PRINTLN(STATE_WIFI_CONNECTED);
}

char* getDefaultTopic()
{
  clientId = getClientId();

  if (clientTopic != NULL) {
    DEBUG_PRINT("___DELETING ... clientTopic - ");
    DEBUG_PRINTLN(clientTopic);
    free(clientTopic);
    clientTopic = NULL;
  }

  clientTopic = (char* )malloc(strlen(clientId) + 6);
  memcpy(clientTopic, clientId, strlen(clientId));
  strcpy(clientTopic + strlen(clientId), "/data");
  DEBUG_PRINT("_clientTopic_SIZE: ");
  DEBUG_PRINTLN(strlen(clientTopic));

  return clientTopic;
}

void prepareClientIdAndClientTopic()
{

  if (clientId != NULL) {
    free(clientId);
    clientId = NULL;
  }

  if (clientTopic != NULL) {
    free(clientTopic);
    clientTopic = NULL;
  }

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
  uint16_t retries = 0;
  prepareClientIdAndClientTopic();

  if (connOpts != NULL) {
    DEBUG_PRINTLN("___Deleting connOpts...");
    delete(connOpts);
    connOpts = NULL;
  }

  connOpts = new MQTT::Connect(clientId);
  connOpts->set_auth(MQTT_USER, MQTT_PASS);
  connOpts->set_keepalive(5);

  int result;
  // Connect to mqtt broker
  unsigned long  start_millis = millis();
  while (true)
  {
    DEBUG_PRINT(STATE_MQTT_CONNECTING);
    DEBUG_PRINT(" [");
    DEBUG_PRINT(clientId);
    DEBUG_PRINT(", ");
    DEBUG_PRINT(clientTopic);
    DEBUG_PRINTLN("]");


    yield();
    result = client->connect(*connOpts);
    MQTT_CONNECTED_FLAG = result;
    if (result == 1)
    {
      break;
    }


    if (WiFi.status() != WL_CONNECTED)
    {
      DEBUG_PRINT("WIFI IS NOT CONNECTED BREAK! ");
      DEBUG_PRINTLN(WiFi.status());
      return;
    }


    DEBUG_PRINT(retries++);
    DEBUG_PRINT(" - TIME TAKE: ");
    DEBUG_PRINTLN(millis() - start_millis);

    delay(100);

    if (millis() - start_millis > 20 * 1000)
    {
      initPubSubClient();
      WiFi.disconnect();
      delay(100);
      return;
    }

  }
  DEBUG_PRINTLN(STATE_MQTT_CONNECTED);
}

void subscribeMqttTopic()
{
  return;
  int result;
  if (!MQTT_CONNECTED_FLAG)
  {
    return;
  }

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
  if (WiFi.status() != WL_CONNECTED)
  {
    DEBUG_PRINTLN("DO NOT SUBSCRIBE... WIFI DISCONNECTED!!");
  }
  else
  {
    subscribeMqttTopic();
  }
}


void publishMqttData(const char* clientTopic, JsonObject &r)
{

  if (!MQTT_CONNECTED_FLAG)
  {
    return;
  }

  if (millis() - prevMillisPub < DELAY_PUBLISH)
  {
    return;
  }

  prevMillisPub = millis();

  static char payload[PAYLOAD_SIZE];

  static long counter = 0;
  root["counter"] = ++counter;

  root.printTo(payload, sizeof(payload));
  DEBUG_PRINT("PAYLOAD LEN: ");
  DEBUG_PRINT(strlen(payload));
  DEBUG_PRINT(" SIZE: ");
  DEBUG_PRINT(sizeof(payload));

  while (!client->publish(clientTopic, payload))
  {
    DEBUG_PRINTLN("PUBLISHED ERROR.");
    yield();
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINT(counter);
  DEBUG_PRINT(" - ");
  DEBUG_PRINTLN("PUBLISHED OK.");
}


#endif // MQTT_SEED_HEADER_H/
