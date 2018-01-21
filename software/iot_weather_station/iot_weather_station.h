/*
  File:    iot_weather_station.h
  Author:  Karl Kangur <karl.kangur@gmail.com>
  Licnece: GNU General Public License
  URL:     https://github.com/Nurgak/IoT-Weather-Station
*/

#pragma once

// Transition structure for the state machine
struct transition_t
{
  STATUS (*state_source)(void);
  STATUS code;
  STATUS (*state_destination)(void);
};

// State functions returning a state value
STATUS state_wifi_connect(void);
STATUS state_ota(void);
STATUS state_mqtt_connect(void);
STATUS state_mqtt_publish(void);
