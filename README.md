# Motion sensor example
## Description
Is an example about how to use a sensor with MQTT autodiscovery in Home Assistant. This example is based in this [documentation](https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery).

## Hardware
I use the board ESP32 Dev Module.

## Configuration motion sensor example
You'll need your variables for MQTT and WIFI connection. I use in this format:

WIFIconfig.h:
```c
const char *ssid = "";
const char *password = "";
const char *HOMEASSISTANT_URL = "";
```
MQTTconfig.h:
```c
const char *mqtt_broker = "";
const char *topicConfig = "homeassistant/binary_sensor/garden/config";
const char *topicState = "homeassistant/binary_sensor/garden/state";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = ;
const char *client_name = "";
```

# Humidity temperature example
## Description
Is an example about how to use a device with two sensor using MQTT autodiscovery in Home Assistant. This example is based in this [documentation](https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery).

## Hardware
I use the board ESP32 Dev Module.

## Configuration motion sensor example
You'll need your variables for MQTT and WIFI connection. I use in this format:

WIFIconfig.h:
```c
const char *ssid = "";
const char *password = "";
const char *HOMEASSISTANT_URL = "";
```
MQTTconfig.h:
```c
const char *mqtt_broker = "";
const char *topicState = "homeassistant/sensor/sensorBedroom/state";
const char *topicTemperatureConfig = "homeassistant/sensor/sensorBedroomT/config";
const char *topicHumidityConfig = "homeassistant/sensor/sensorBedroomH/config";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = ;
const char *client_name = "";
```