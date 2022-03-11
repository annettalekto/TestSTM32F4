/*
 * flash.h
 *
 *  Created on: 3 мар. 2022 г.
 *      Author: nesterovaaa
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "main.h"

#define FLASH_CONFIG_START_ADDR	((uint32_t)0x0810C000) // 0x0810C000 - 0x0810FFFF

/*typedef struct {
	uint32_t Level;
} SensorType;

typedef struct {
	uint32_t ID;
} ConfigCanType;

typedef struct {
	uint32_t WriteCounter;
	uint32_t Crc;
	SensorType Sensor;
	ConfigCanType ConfigCAN;
} DeviceConfigType;

union FlashData {
	DeviceConfigType DeviceConfig; // кратно 64
	uint32_t DataWords[4];
};*/

/////////////////////////////////// структуры стек:

#define FLASH_DATA_SIZE	8	//  8 по 32

typedef struct ConfigDataCAN_
{
  uint32_t ID;              // при расширенноми идентификаторе старший бит старшего байта = 1
  uint32_t BaudRate;        // скорость в кбит/с
  uint32_t Tseg1;            // (Tseg1 - 1) точно -1?
  uint32_t Tseg2;            // (Tseg2 - 1)
  uint16_t UpLimit;          // не используется (зарезервировано)
} CONFIG_CAN, *PCONFIG_CAN;

typedef struct SensorSettings_
{
  uint16_t HighLimit;  // верхний предел измерения
  uint16_t LowLimit;   // нижний предел измерения
  uint16_t ChangeTime; // последняя подстройка (дата изменения пределов измерения)
} SENSOR_SETTINGS, *PSENSOR_SETTINGS;

typedef struct
{
	uint32_t WriteCounter;
	uint32_t Crc;
	SENSOR_SETTINGS Sensor;
	CONFIG_CAN ConfigCAN;
} DeviceConfigType;

union FlashData {		// кратно 64!
	DeviceConfigType DeviceConfig;
	uint32_t DataWords[8];
};

uint32_t GetConfigCANID(void);

SENSOR_SETTINGS GetSensorSettings(void);
void SaveSensorSettings(SENSOR_SETTINGS newSS);
void SaveFlashSensorSettings(SENSOR_SETTINGS newSS);

void ReadConfig(uint32_t *RxBuf);
void EraseFlash(void);
void WriteConfig(uint32_t *WxBuf);

#endif /* INC_FLASH_H_ */
