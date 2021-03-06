/*
 * flash.h
 *
 *  Created on: 3 мар. 2022 г.
 *      Author: nesterovaaa
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "main.h"

#define FLASH_BANK2_SECTOR_14			((uint32_t)0x08108000) // 0x08108000 - 0x0810BFFF
#define FLASH_BANK2_SECTOR_15			((uint32_t)0x0810C000) // 0x0810C000 - 0x0810FFFF

#define FLASH_CONFIG_START_ADDR	FLASH_BANK2_SECTOR_15

typedef struct ConfigDataCAN_
{
  uint32_t ID;			// идентификатор для запроса данных с датчика
  uint32_t BaudRate;	// скорость в кбит/с
  uint32_t Tseg1;		// TimeSeg1 - 1
  uint32_t Tseg2;		// TimeSeg1 - 1
  uint32_t UpLimit;		// не используется (зарезервировано)
} CONFIG_CAN, *PCONFIG_CAN;

typedef struct SensorSettings_
{
  uint32_t HighLimit;	// верхний предел измерения
  uint32_t LowLimit;	// нижний предел измерения
  uint32_t ChangeTime;	// последняя подстройка (дата изменения пределов измерения)
} SENSOR_SETTINGS, *PSENSOR_SETTINGS;

typedef struct
{
	uint32_t WriteCounter;
	SENSOR_SETTINGS Sensor;
	CONFIG_CAN ConfigCAN;
	uint32_t Crc;
} DeviceConfigType;

#define FLASH_DATA_SIZE	10	// 10 по 32
union FlashData {			// кратно 32!
	DeviceConfigType DeviceConfig;
	uint32_t DataWords[FLASH_DATA_SIZE];
};

//void InitCurrentConfig(void);
bool ReadCurrentConfigFromFlash(void);
void SaveCurrentConfigToFlash(void);

uint32_t GetConfigCANID(void);
CONFIG_CAN GetConfigCAN(void);
void SaveConfigCAN(CONFIG_CAN newCC);
SENSOR_SETTINGS GetSensorSettings(void);
void SaveSensorSettings(SENSOR_SETTINGS newSS);

#endif /* INC_FLASH_H_ */
