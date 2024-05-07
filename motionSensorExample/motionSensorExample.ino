#include "WIFIconfig.h"
#include "MQTTconfig.h"

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Timer.h>

// CONSTANTS:
const boolean START = true;
const boolean RESET = false;

// TIMERS:
TON *tPublishState;
TON *tCheckConnection;

const unsigned long ONE_SECOND = 1000;
const unsigned long FIVE_SECOND = 5000;
const unsigned long ONE_MINUTE = 60000;

// INPUTS

// OUTPUTS

// Objects
WiFiClient espClient;
PubSubClient client(espClient);

// STATE VARIABLES
boolean commandReceived = false;
char *state = "";
boolean deviceConfigured = false;

//TODO: Parece que esta prueba funciona
void buildDeviceConfig(){
  StaticJsonDocument<512> doc;
  
  String payload = "";
  doc["name"] = nullptr;
  doc["device_class"] = "motion";
  doc["state_topic"] = "homeassistant/binary_sensor/garden/state";
  doc["unique_id"] = "motion01ad";
  doc["device"]["identifiers"][0] = "01ad";
  doc["device"]["name"] = "Garden movement sensor";
  
  serializeJson(doc, payload);

  Serial.print("Send config: ");
  Serial.println((char *)payload.c_str());
  
  client.publish(topicConfig, (char *)payload.c_str());
}

void setup()
{
  Serial.begin(115200);

  WIFIConnection();

  OTAConfig();

  MQTTConnection();

  Serial.println("Connected to the WiFi network");
  
  tPublishState = new TON(FIVE_SECOND);
  tCheckConnection = new TON(ONE_MINUTE);
}

void publishInfo()
{
  if (tPublishState->IN(START))
  {
    String payload = "OFF";
    
    client.publish(topicState, (char *)payload.c_str());

    tPublishState->IN(RESET);
  }
}

void checkMqttConnection()
{
  if (tCheckConnection->IN(START))
  {
    if (!client.connected())
    {
      ESP.restart();
    }

    tCheckConnection->IN(RESET);
  }
}

void loop()
{
  ArduinoOTA.handle();
  client.loop();
  yield();
  checkMqttConnection();

  if (commandReceived)
  {
    
  }

  if(client.connected() && !deviceConfigured){
    buildDeviceConfig();
    deviceConfigured = true;
  }

  publishInfo();
}

void WIFIConnection()
{
  // connecting to a WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi..");
    delay(5000);
    ESP.restart();
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

void OTAConfig()
{
  ArduinoOTA.setHostname(client_name);
  ArduinoOTA.onStart([]()
                     {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    } });
  ArduinoOTA.begin();
}

void MQTTConnection()
{
  // connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  while (!client.connected())
  {
    String client_id = client_name;
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Public emqx mqtt broker connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
      ESP.restart();
    }
  }

  client.subscribe(topicCommand);
}

void callback(char *topicCommand, byte *payload, unsigned int length)
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topicCommand);
  Serial.print("Message:");
  String payload_n;

  for (int i = 0; i < length; i++)
  {
    payload_n += (char)payload[i];
  }

  Serial.println(payload_n);
  Serial.println("-----------------------");

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payload_n);

  if (error)
    return;

  String command = doc["portal"]["orden"];
}
