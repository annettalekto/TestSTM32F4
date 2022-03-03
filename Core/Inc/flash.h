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

typedef struct {
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
};

void ReadConfig(uint32_t *RxBuf);
void EraseFlash(void);
void WriteConfig(uint32_t *WxBuf);

#endif /* INC_FLASH_H_ */
