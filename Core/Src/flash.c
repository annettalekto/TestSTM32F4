/*
 * flash.c
 *
 *  Created on: 3 мар. 2022 г.
 *      Author: nesterovaaa
 */

#include "flash.h"

SENSOR_SETTINGS SensorSettings;
CONFIG_CAN ConfigCAN;

extern CRC_HandleTypeDef hcrc;

// --------------------------- конфигурация CAN ------------------------------ //
uint32_t GetConfigCANID(void)
{
	return ConfigCAN.ID;
}
// --------------------------- пределы датчика ------------------------------ //
SENSOR_SETTINGS GetSensorSettings(void)
{
	return SensorSettings;
}

void SaveSensorSettings(SENSOR_SETTINGS newSS)
{
	SensorSettings = newSS;
}

void SaveFlashSensorSettings(SENSOR_SETTINGS newSS)
{
	SensorSettings = newSS;// todo
}

// --------------------------------- FLASH --------------------------------- //
/*void ReadConfig(uint32_t* RxBuf)
{
	uint32_t address = FLASH_CONFIG_START_ADDR;
	int idx = 0;

	while (idx < 4)
	{
		RxBuf[idx] = *(__IO uint32_t *)address;
		idx++;
		address += 4;
	}
}

void EraseFlash(void)
{
  	FLASH_EraseInitTypeDef EraseInitStruct;
  	uint32_t SectorError;

  	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS; 	// массовое или секторное стирание
  	EraseInitStruct.Banks = FLASH_BANK_2; 					// для многобанковых МК, 1, 2 или оба
  	EraseInitStruct.Sector = FLASH_SECTOR_15;				// последний сектор 16 Кбайт
  	EraseInitStruct.NbSectors = 1; 							// количество секторов которые нужно стереть, начиная с заданного поле Sector
  	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;	//

  	// стирание в режиме опроса (HAL_FLASHEx_Erase_IT -- в реж. прерывания(можно получить уведомление о завершеннии по прерыванию))
  	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK )
  	{
  		printf("\tERROR Erase flash sector: %d\n", SectorError);
  	}
}

void WriteConfig(uint32_t *WxBuf)
{
	uint32_t address = FLASH_CONFIG_START_ADDR;
	int idx = 0;

	while (idx < 4)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, WxBuf[idx]) != HAL_OK)
//		if (FLASH_Program_Word(FLASH_TYPEPROGRAM_WORD, address, *WxBuf) != HAL_OK)
		{
			printf("\tERROR Program flash\n");
		}
		idx++;
		address += 4;
	}
}
*/


/*
CRC(data + CRC(data)) == 0.
если мы вычислили CRC и добавили его к данным — вычисление CRC этого пакета должно дать 0. Таким образом, на принимающей стороне достаточно проверить, что CRC принятых данных равен 0 — значит, данные пришли полностью.

	uint32_t CRCVal = HAL_CRC_Calculate(&hcrc, (uint32_t *)WxBuf, sizeof(WxBuf)/sizeof(uint32_t));

Писать структуру можно сразу в нескольких местах с некоторым смещением по сектору, если не считанно с 0 смещения правильно, считать с 0+number
*/
