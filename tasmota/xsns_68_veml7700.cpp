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
#define XI2C_68                          49     // See I2CDEVICES.md

#define VEML7700_ADDR                    0x10   //< I2C address


#define VEML7700_ALS_CONFIG          0x00  ///< Light configuration register
#define VEML7700_ALS_THREHOLD_HIGH   0x01  ///< Light high threshold for irq
#define VEML7700_ALS_THREHOLD_LOW    0x02  ///< Light low threshold for irq
#define VEML7700_ALS_POWER_SAVE      0x03  ///< Power save regiester
#define VEML7700_ALS_DATA            0x04  ///< The light data output
#define VEML7700_WHITE_DATA          0x05  ///< The white light data output
#define VEML7700_INTERRUPTSTATUS     0x06  ///< What IRQ (if any)

#define VEML7700_INTERRUPT_HIGH     0x4000 ///< Interrupt status for high threshold
#define VEML7700_INTERRUPT_LOW      0x8000 ///< Interrupt status for low threshold

#define VEML7700_GAIN_1             0x00  ///< ALS gain 1x
#define VEML7700_GAIN_2             0x01  ///< ALS gain 2x
#define VEML7700_GAIN_1_8           0x02  ///< ALS gain 1/8x
#define VEML7700_GAIN_1_4           0x03  ///< ALS gain 1/4x

#define VEML7700_IT_100MS           0x00  ///< ALS intetgration time 100ms
#define VEML7700_IT_200MS           0x01  ///< ALS intetgration time 200ms
#define VEML7700_IT_400MS           0x02  ///< ALS intetgration time 400ms
#define VEML7700_IT_800MS           0x03  ///< ALS intetgration time 800ms
#define VEML7700_IT_50MS            0x08  ///< ALS intetgration time 50ms
#define VEML7700_IT_25MS            0x0C  ///< ALS intetgration time 25ms

#define VEML7700_PERS_1             0x00  ///< ALS irq persisance 1 sample
#define VEML7700_PERS_2             0x01  ///< ALS irq persisance 2 samples
#define VEML7700_PERS_4             0x02  ///< ALS irq persisance 4 samples
#define VEML7700_PERS_8             0x03  ///< ALS irq persisance 8 samples

#define VEML7700_POWERSAVE_MODE1    0x00  ///< Power saving mode 1
#define VEML7700_POWERSAVE_MODE2    0x01  ///< Power saving mode 2
#define VEML7700_POWERSAVE_MODE3    0x02  ///< Power saving mode 3
#define VEML7700_POWERSAVE_MODE4    0x03  ///< Power saving mode 4



#define VEML7700_CONTINUOUS_HIGH_RES_MODE2 0x11  // Start measurement at 0.5 lx resolution. Measurement time is approx 120ms.
#define VEML7700_CONTINUOUS_HIGH_RES_MODE  0x10  // Start measurement at 1   lx resolution. Measurement time is approx 120ms.
#define VEML7700_CONTINUOUS_LOW_RES_MODE   0x13  // Start measurement at 4   lx resolution. Measurement time is approx 16ms.

#define VEML7700_MEASUREMENT_TIME_HIGH     0x40  // Measurement Time register high 3 bits
#define VEML7700_MEASUREMENT_TIME_LOW      0x60  // Measurement Time register low 5 bits

struct VEML7700DATA {
  uint8_t address;
  uint8_t gain[4] = {VEML7700_GAIN_1,VEML7700_GAIN_2,VEML7700_GAIN_1_8, VEML7700_GAIN_1_4};
  uint8_t integration_time[6] = {VEML7700_IT_25MS,VEML7700_IT_50MS,VEML7700_IT_100MS, VEML7700_IT_200MS,VEML7700_IT_400MS, VEML7700_IT_800MS};
  uint8_t power_save_mode[4] = {VEML7700_POWERSAVE_MODE1,VEML7700_POWERSAVE_MODE2, VEML7700_POWERSAVE_MODE3,VEML7700_POWERSAVE_MODE4};
    VEML7700_POWERSAVE_MODE1
  //uint8_t type = 0;
  //uint8_t valid = 0;
  //uint8_t mtreg = 69;                          // Default Measurement Time
  uint16_t illuminance = 0;
  char types[8] = "VEML7700";
} VEML7700;

/*********************************************************************************************/

bool VEML7700SetResolution(void)
{
  Wire.beginTransmission(VEML7700.address);
  Wire.write(VEML7700.resolution[Settings.SensorBits1.VEML7700_resolution]);
  return (!Wire.endTransmission());
}

bool VEML7700SetMTreg(void)
{
  Wire.beginTransmission(VEML7700.address);
  uint8_t data = VEML7700_MEASUREMENT_TIME_HIGH | ((VEML7700.mtreg >> 5) & 0x07);
  Wire.write(data);
  if (Wire.endTransmission()) { return false; }
  Wire.beginTransmission(VEML7700.address);
  data = VEML7700_MEASUREMENT_TIME_LOW | (VEML7700.mtreg & 0x1F);
  Wire.write(data);
  if (Wire.endTransmission()) { return false; }
  return VEML7700SetResolution();
}

bool VEML7700Read(void)
{
  if (VEML7700.valid) { VEML7700.valid--; }

  if (2 != Wire.requestFrom(VEML7700.address, (uint8_t)2)) { return false; }
  float illuminance = (Wire.read() << 8) | Wire.read();
  illuminance /= (1.2 * (69 / (float)VEML7700.mtreg));
  if (1 == Settings.SensorBits1.VEML7700_resolution) {
    illuminance /= 2;
  }
  VEML7700.illuminance = illuminance;

  VEML7700.valid = SENSOR_MAX_MISS;
  return true;
}

/********************************************************************************************/

void VEML7700Detect(void)
{
  for (uint32_t i = 0; i < sizeof(VEML7700.addresses); i++) {
    VEML7700.address = VEML7700.addresses[i];
    if (I2cActive(VEML7700.address)) { continue; }

    if (VEML7700SetMTreg()) {
      I2cSetActiveFound(VEML7700.address, VEML7700.types);
      VEML7700.type = 1;
      break;
    }
  }
}

void VEML7700EverySecond(void)
{
  // 1mS
  if (!VEML7700Read()) {
    AddLogMissed(VEML7700.types, VEML7700.valid);
  }
}

/*********************************************************************************************\
 * Command Sensor10
 *
 * 0       - High resolution mode (default)
 * 1       - High resolution mode 2
 * 2       - Low resolution mode
 * 31..254 - Measurement Time value (not persistent, default is 69)
\*********************************************************************************************/

bool VEML7700CommandSensor(void)
{
  if (XdrvMailbox.data_len) {
    if ((XdrvMailbox.payload >= 0) && (XdrvMailbox.payload <= 2)) {
      Settings.SensorBits1.VEML7700_resolution = XdrvMailbox.payload;
      VEML7700SetResolution();
    }
    else if ((XdrvMailbox.payload > 30) && (XdrvMailbox.payload < 255)) {
      VEML7700.mtreg = XdrvMailbox.payload;
      VEML7700SetMTreg();
    }
  }
  Response_P(PSTR("{\"" D_CMND_SENSOR "10\":{\"Resolution\":%d,\"MTime\":%d}}"), Settings.SensorBits1.VEML7700_resolution, VEML7700.mtreg);

  return true;
}

void VEML7700Show(bool json)
{
  if (VEML7700.valid) {
    if (json) {
      ResponseAppend_P(JSON_SNS_ILLUMINANCE, VEML7700.types, VEML7700.illuminance);
#ifdef USE_DOMOTICZ
      if (0 == tele_period) {
        DomoticzSensor(DZ_ILLUMINANCE, VEML7700.illuminance);
      }
#endif  // USE_DOMOTICZ
#ifdef USE_WEBSERVER
    } else {
      WSContentSend_PD(HTTP_SNS_ILLUMINANCE, VEML7700.types, VEML7700.illuminance);
#endif  // USE_WEBSERVER
    }
  }
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xsns10(uint8_t function)
{
  if (!I2cEnabled(XI2C_68)) { return false; }

  bool result = false;

  if (FUNC_INIT == function) {
    VEML7700Detect();
  }
  else if (VEML7700.type) {
    switch (function) {
      case FUNC_EVERY_SECOND:
        VEML7700EverySecond();
        break;
      case FUNC_COMMAND_SENSOR:
        if (XSNS_10 == XdrvMailbox.index) {
          result = VEML7700CommandSensor();
        }
        break;
      case FUNC_JSON_APPEND:
        VEML7700Show(1);
        break;
#ifdef USE_WEBSERVER
      case FUNC_WEB_SENSOR:
        VEML7700Show(0);
        break;
#endif  // USE_WEBSERVER
    }
  }
  return result;
}

#endif  // USE_VEML7700
#endif  // USE_I2C
