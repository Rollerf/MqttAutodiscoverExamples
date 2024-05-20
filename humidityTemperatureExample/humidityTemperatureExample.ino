#include "WIFIconfig.h"
#include "MQTTconfig.h"

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// CONSTANTS:
const boolean START = true;
const boolean RESET = false;

// Objects
WiFiClient espClient;
PubSubClient client(espClient);

// STATE VARIABLES
boolean commandReceived = false;
char *state = "";
boolean deviceConfigured = false;

void buildTemperatureDeviceConfig() {
  JsonDocument doc;
  doc["device_class"] = "temperature";
  doc["state_topic"] = "homeassistant/sensor/sensorBedroom/state";
  doc["unit_of_measurement"] = "°C";
  doc["value_template"] = "{{ value_json.temperature}}";
  doc["unique_id"] = "temp01ae";

  JsonObject device = doc["device"].to<JsonObject>();
  device["identifiers"][0] = "bedroom01ae";
  device["name"] = "Bedroom";
  device["manufacturer"] = "Example sensors Ltd.";
  device["model"] = "K9";
  device["serial_number"] = "12AE3010545";
  device["hw_version"] = "1.01a";
  device["sw_version"] = "2024.1.0";
  device["configuration_url"] = "https://example.com/sensor_portal/config";

  String output;

  doc.shrinkToFit();  // optional

  serializeJson(doc, output);

  Serial.print("Send config: ");
  Serial.println((char *)output.c_str());

  client.publish(topicTemperatureConfig, (char *)output.c_str());
}

void buildHumidityDeviceConfig() {
  JsonDocument doc;
  String payload = "";

  doc["device_class"] = "humidity";
  doc["state_topic"] = "homeassistant/sensor/sensorBedroom/state";
  doc["unit_of_measurement"] = "%";
  doc["value_template"] = "{{ value_json.humidity}}";
  doc["unique_id"] = "hum01ae";

  JsonObject device = doc.createNestedObject("device");

  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add("bedroom01ae");

  doc.shrinkToFit();  // optional

  serializeJson(doc, payload);

  Serial.print("Send config: ");
  Serial.println((char *)payload.c_str());

  client.publish(topicHumidityConfig, (char *)payload.c_str());
}

void setup()
{
  Serial.begin(115200);

  WIFIConnection();

  OTAConfig();

  delay(1000);

  MQTTConnection();

  delay(1000);

  Serial.println("Connected to the WiFi network");


  //Set maximum size of mqtt messages
  client.setBufferSize(512);
}

void publishInfo()
{
  StaticJsonDocument<512> doc;

  String payload;
  doc["temperature"] = 23.20;
  doc["humidity"] = 43.70;
  serializeJson(doc, payload);

  Serial.print("Send state: ");
  Serial.println((char *)payload.c_str());

  client.publish(topicState, (char *)payload.c_str());
}

void checkMqttConnection()
{
  if (!client.connected())
  {
    ESP.restart();
  }
}

void loop()
{
  ArduinoOTA.handle();
  client.loop();
  yield();
  delay(1000);
  checkMqttConnection();
  delay(1000);

  if (client.connected() && !deviceConfigured) {
    buildTemperatureDeviceConfig();
    delay(1000);
    buildHumidityDeviceConfig();
    delay(1000);
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
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]()
  {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
  {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
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
    }
  });
  ArduinoOTA.begin();
}

void MQTTConnection()
{
  // connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
//  client.setCallback(callback);

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
}
