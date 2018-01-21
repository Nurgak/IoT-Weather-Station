/*
  File:    config.h
  Author:  Karl Kangur <karl.kangur@gmail.com>
  Licnece: GNU General Public License
  URL:     https://github.com/Nurgak/IoT-Weather-Station
*/

#pragma once

static const int DHT_PIN = 2;
static const uint16_t TIME_DHT11_UPDATE = 10000; // Time between senging messages to the MQTT broker in milliseconds
static const uint16_t TIME_WIFI_CONNECT = 5000; // Maximum waiting time in seconds for Wi-Fi connection
static const uint16_t TIME_MQTT_CONNECT = 3000; // Maximum waiting time in seconds for MQTT connection

static const char* WIFI_SSID = "...";
static const char* WIFI_PASSWORD = "...";

static const char* MQTT_SERVER = "192.168.1.10"; // MQTT broker server
static const uint16_t MQTT_PORT = 1883; // MQTT broker port, usually 1883 or 8883
static const char* HOST_NAME = "IoTWeatherStation"; // Name of the device
static const char* MQTT_USERNAME = "...";
static const char* MQTT_PASSWORD = "...";

const char* MQTT_TOPIC_STATUS = "status";
const char* MQTT_TOPIC_TEMPERATURE = "temperature";
const char* MQTT_TOPIC_HUMIDITY = "humidity";
const char* MQTT_TOPIC_HEATINDEX = "heatindex";

//#define ENABLE_STATIC_IP // Comment this line for dynamic IP
static const uint8_t WIFI_IP[] = {192, 168, 1, 101}; // The IP you want to give to your device
static const uint8_t WIFI_GATEWAY[] = {192, 168, 1, 1};  // Router IP
static const uint8_t WIFI_SUBNET[] = {255, 255, 255, 0}; // You probably do not need to change this one
