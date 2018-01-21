/*
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
  File:    iot_weather_station.ino
  Author:  Karl Kangur <karl.kangur@gmail.com>
  Licnece: GNU General Public License
  URL:     https://github.com/Nurgak/IoT-Weather-Station
*/

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <DHTesp.h>
#include <PubSubClient.h>
#include "config.h"
#include "iot_weather_station.h"

DHTesp dht;
WiFiClient wifi_client;
PubSubClient client(MQTT_SERVER, MQTT_PORT, wifi_client);

// State transition matrix
const transition_t state_transitions[] = {
  {state_wifi_connect, OK,   state_ota},
  {state_wifi_connect, BUSY, state_wifi_connect},
  {state_wifi_connect, FAIL, state_wifi_connect},
  {state_ota,          OK,   state_mqtt_connect},
  {state_mqtt_connect, OK,   state_mqtt_publish},
  {state_mqtt_connect, FAIL, state_wifi_connect},
  {state_mqtt_publish, OK,   state_wifi_connect},
  {state_mqtt_publish, FAIL, state_wifi_connect}
};

#define START_STATE state_wifi_connect

void setup(void)
{
  Serial.begin(115200);
  // Set to "client-only mode"
  WiFi.mode(WIFI_STA);
  ArduinoOTA.begin();
  dht.setup(DHT_PIN, DHTesp::DHT11);
}

void loop()
{
  // Pointer to current state function, will start by failing and call start
  static STATUS (*current_state)(void) = NULL;
  static STATUS code;
  static const uint16_t transitions = sizeof(state_transitions) / sizeof(state_transitions[0]);
  static uint16_t i;

  // Run the state machine
  for(i = 0; i < transitions; ++i)
  {
    if(state_transitions[i].state_source == current_state && state_transitions[i].code == code)
    {
      current_state = state_transitions[i].state_destination;
      // Call the state function, get the return code back for next transition
      code = (current_state)();
      // Return immediately
      return;
    }
  }

  // Exception: no transition from state was found, start again
  current_state = START_STATE;
  code = (current_state)();
}

STATUS ICACHE_FLASH_ATTR state_wifi_connect()
{
  static STATUS status_return = OK;
  static uint32_t time_wifi_connect = 0;
  static bool wifi_configured = false;

  // If connected and configuration is correct return immediately
  if(wifi_configured && WiFi.status() == WL_CONNECTED)
  {
    if(status_return != OK)
    {
      time_wifi_connect = 0;
      status_return = OK;

      Serial.print(F("Connected to WiFi: "));
      Serial.println(WiFi.localIP());
    }
  }
  else if(status_return == BUSY)
  {
    // After a maximum timeout return a failure state
    if(millis() - time_wifi_connect >= TIME_WIFI_CONNECT)
    {
      // WiFi failed to connect
      time_wifi_connect = 0;
      wifi_configured = false;
      status_return = FAIL;
    }
    else
    {
      // Return a busy state indicating progress
      status_return = BUSY;
    }
  }
  else
  {
    Serial.println(F("Connecting to WiFi"));

#ifdef ENABLE_STATIC_IP
    WiFi.config(WIFI_IP, WIFI_GATEWAY, WIFI_SUBNET);
#endif
    WiFi.begin((char*)WIFI_SSID, WIFI_PASSWORD);

    // Without this the WiFi might connect too fast using an old configuration
    wifi_configured = true;

    time_wifi_connect = millis();
    status_return = BUSY;
  }

  return status_return;
}

STATUS ICACHE_FLASH_ATTR state_ota()
{
  // Handle OTA updates
  ArduinoOTA.handle();

  // This state can only return true, the particular cases are taken care by OTA
  // callbacks (start, progress, end...) which must be defined in the setup
  return OK;
}

STATUS ICACHE_FLASH_ATTR state_mqtt_connect()
{
  static STATUS status_return = OK;
  static uint32_t time_mqtt_connect = 0;

  if(!client.connected() && millis() - time_mqtt_connect >= TIME_MQTT_CONNECT)
  {
    // Wait 5 seconds before trying again if it failed before
    while(!client.connected())
    {
      Serial.println(F("Attempting MQTT connection..."));
      // Blocking attempt
      if(client.connect(HOST_NAME, MQTT_USERNAME, MQTT_PASSWORD))
      {
        status_return = OK;

        Serial.println(F("Connected"));
        time_mqtt_connect = 0;
      }
      else
      {
        Serial.print(F("Failed to connect to MQTT broker, state: "));
        Serial.println(client.state());
        status_return = FAIL;
        time_mqtt_connect = millis();
      }
    }
  }
  
  return status_return;
}

STATUS ICACHE_FLASH_ATTR state_mqtt_publish()
{
  static STATUS status_return = OK;
  static uint32_t time_dht11_update = 0;
  char buffer[16];
  
  if(client.connected())
  {
    // Minimum update rate set by the sensor
    if(
      millis() - time_dht11_update >= TIME_DHT11_UPDATE &&
      millis() - time_dht11_update >= dht.getMinimumSamplingPeriod()
    )
    {
      float humidity = dht.getHumidity();
      float temperature = dht.getTemperature();
      float heatindex = dht.computeHeatIndex(temperature, humidity, false);
    
      client.publish(MQTT_TOPIC_STATUS, dht.getStatusString());
      Serial.print(F("Status: "));
      Serial.println(buffer);
      
      String(temperature).toCharArray(buffer, sizeof(buffer));
      client.publish(MQTT_TOPIC_TEMPERATURE, buffer);
      Serial.print(F("Temperature: "));
      Serial.println(buffer);
      
      String(humidity).toCharArray(buffer, sizeof(buffer));
      client.publish(MQTT_TOPIC_HUMIDITY, buffer);
      Serial.print(F("Humidity: "));
      Serial.println(buffer);
      
      String(heatindex).toCharArray(buffer, sizeof(buffer));
      client.publish(MQTT_TOPIC_HEATINDEX, buffer);
      Serial.print(F("Heat index: "));
      Serial.println(buffer);
      
      time_dht11_update = millis();
      status_return = OK;
    }
    else
    {
      status_return = FAIL;
    }
    
    client.loop();
  }

  return status_return;
}
