/*
  xsns_68_VEML7700.ino - VML7700 ambient light sensor support for Tasmota

  Copyright (C) 2020  Theo Arends

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

#ifdef USE_I2C
#ifdef USE_VEML7700
/*********************************************************************************************\
 * VEML7700 - Ambient Light Intensity
 *
 * I2C Address: 0x10
\*********************************************************************************************/

#define XSNS_68                          68
#define XI2C_49                          49     // See I2CDEVICES.md

#define VEML7700_ADDR                    0x10   //< I2C address


#include "Adafruit_VEML7700.h"
Adafruit_VEML7700 veml7700_sensor;

uint8_t veml7700_ready = 0;
uint16_t lux_value;
uint16_t white_value;
uint16_t rawals_value;
uint16_t nits_value;

/********************************************************************************************/

void Veml7700Detect()
{
  if (veml7700_ready) {
    AddLog_P(LOG_LEVEL_DEBUG, PSTR("VEML7700 is ready"));
    return;
  }

  if (!veml7700_sensor.begin()) {
    AddLog_P(LOG_LEVEL_DEBUG, PSTR("Unable to init VEML7700 Sensor."));
    return;
  }

  veml7700_ready = 1;

  snprintf_P(log_data, sizeof(log_data), S_LOG_I2C_FOUND_AT, "VEML7700", VEML7700_I2CADDR_DEFAULT);
  AddLog(LOG_LEVEL_DEBUG);
}

void Veml7700_Every_Second(void) {
  if (veml7700_ready) {
    lux_value = veml7700_sensor.readLux();
    white_value = veml7700_sensor.readWhite();
    rawals_value = veml7700_sensor.readALS();
    nits_value = lux_value / 3.14159265359;
  }
}

#define D_rawals_value "Raw ALS"
#define D_lux_value "Lux"
#define D_white_value "White"
#define D_nits_value "Nits"
#define D_JSON_rawals_value "Raw ALS"
#define D_JSON_lux_value "Lux"
#define D_JSON_white_value "White"
#define D_JSON_nits_value "Nits"


#ifdef USE_WEBSERVER
const char HTTP_SNS_VEML7700[] PROGMEM =
  "{s}VEML7700 " D_lux_value "{m}%d lx{e}" // {s} = <tr><th>, {m} = </th><td>, {e} = </td></tr>
  "{s}VEML7700 " D_nits_value "{m}%d nt{e}"
  "{s}VEML7700 " D_white_value "{m}%d{e}"
  "{s}VEML7700 " D_rawals_value "{m}%d{e}";
  #endif  // USE_WEBSERVER

void Veml7700Show(boolean json)
{
  if (!veml7700_ready) {
    return;
  }
  if (json) {
    ResponseAppend_P(PSTR(",\"VEML7700\":{\"" D_JSON_lux_value "\":%d,\"" D_JSON_nits_value "\":%d,\"" D_JSON_white_value "\":%d,\"" D_JSON_rawals_value "\":%d}"), lux_value, nits_value, white_value, rawals_value);
#ifdef USE_WEBSERVER
  } else {
    WSContentSend_PD(HTTP_SNS_VEML7700, lux_value, nits_value, white_value, rawals_value);
#endif
  }
}


/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xsns68(byte function)
{
  bool result = false;

  if (i2c_flg) {
    switch (function) {
      case FUNC_INIT:
        Veml7700Detect();
        break;
      case FUNC_EVERY_SECOND:
        Veml7700_Every_Second();
        break;
      case FUNC_COMMAND:
        break;
      case FUNC_JSON_APPEND:
        Veml7700Show(1);
        break;
#ifdef USE_WEBSERVER
      case FUNC_WEB_SENSOR:
        Veml7700Show(0);
        break;
#endif  // USE_WEBSERVER
    }
  }
  return result;
}

#endif  // USE_VEML7700
#endif  // USE_I2C