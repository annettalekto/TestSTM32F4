/*
 * flash.c
 *
 *  Created on: 3 мар. 2022 г.
 *      Author: nesterovaaa
 */

#include "flash.h"

void ReadFlash(uint32_t *RxBuf);
void EraseSectorFlash(void);
void WriteFlash(uint32_t *WxBuf);
bool WriteConfigToFlash(void);

extern CRC_HandleTypeDef hcrc;
bool StartWrite = false;

union FlashData CurrentConfig;

//void InitCurrentConfig(void)
//{
//	CurrentConfig.DeviceConfig.WriteCounter = 0;
//	CurrentConfig.DeviceConfig.Crc = 0;
//
//	CurrentConfig.DeviceConfig.ConfigCAN.BaudRate = 25;
//	CurrentConfig.DeviceConfig.ConfigCAN.ID = 0;
//	CurrentConfig.DeviceConfig.ConfigCAN.Tseg1 = 12;
//	CurrentConfig.DeviceConfig.ConfigCAN.Tseg2 = 1;
//	CurrentConfig.DeviceConfig.ConfigCAN.UpLimit = 0;
//
//	CurrentConfig.DeviceConfig.Sensor.ChangeTime = 0;
//	CurrentConfig.DeviceConfig.Sensor.HighLimit = 0; //todo нормальные установки
//	CurrentConfig.DeviceConfig.Sensor.LowLimit = 0;
//}

// Читать текущую конфигурацию из памяти
bool ReadCurrentConfigFromFlash(void)
{
	union FlashData rxBuf;
	ReadFlash(rxBuf.DataWords);

	uint32_t crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)rxBuf.DataWords, FLASH_DATA_SIZE);// для всего сообщения
	if(crc == 0)
	{
		CurrentConfig = rxBuf;
		return true;
	}
	return false;
}

// Сохранить текущую конфигурацию в память (сохранить все изменения в CurrentConfig перед этим)
void SaveCurrentConfigToFlash(void)
{
	StartWrite = true;
}

// --------------------------- конфигурация CAN ------------------------------ //

uint32_t GetConfigCANID(void)
{
	return CurrentConfig.DeviceConfig.ConfigCAN.ID;
}

CONFIG_CAN GetConfigCAN(void)
{
	return CurrentConfig.DeviceConfig.ConfigCAN;
}

void SaveConfigCAN(CONFIG_CAN newCC)
{
	CurrentConfig.DeviceConfig.ConfigCAN = newCC;
}

// --------------------------- пределы датчика ------------------------------ //

SENSOR_SETTINGS GetSensorSettings(void)
{
	return CurrentConfig.DeviceConfig.Sensor;
}

void SaveSensorSettings(SENSOR_SETTINGS newSS)
{
	CurrentConfig.DeviceConfig.Sensor = newSS;
}

// --------------------------------- FLASH --------------------------------- //

void ReadFlash(uint32_t* RxBuf)
{
	uint32_t address = FLASH_CONFIG_START_ADDR;
	int idx = 0;

	while (idx < FLASH_DATA_SIZE)
	{
		RxBuf[idx] = *(__IO uint32_t *)address;
		idx++;
		address += 4;
	}
}

void EraseSectorFlash(void)
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
  		printf("\tERROR Erase flash sector: %lu\n", SectorError);
  	}
}

void WriteFlash(uint32_t *WxBuf)
{
	uint32_t address = FLASH_CONFIG_START_ADDR;
	int words = 0;

	while (words < FLASH_DATA_SIZE)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, WxBuf[words]) != HAL_OK)
		{
			printf("\tERROR Program flash\n");
		}
		words++;
		address += 4;
	}
}

// все настройки CurrentConfig сохранить до вызова функции
bool WriteConfigToFlash(void)
{
	bool ok;
	union FlashData rxBuf;

	ReadFlash(rxBuf.DataWords);
	if(rxBuf.DataWords != CurrentConfig.DataWords)
	{
		CurrentConfig.DeviceConfig.WriteCounter++;
		CurrentConfig.DeviceConfig.Crc = 0;
		uint32_t crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)CurrentConfig.DataWords, FLASH_DATA_SIZE-1); //только для данных
		CurrentConfig.DeviceConfig.Crc = crc;

		if (HAL_FLASH_Unlock() != HAL_OK)
		{
			printf("\tERROR Flash unlock!\n");
		}
		EraseSectorFlash();
		WriteFlash(CurrentConfig.DataWords);
		HAL_FLASH_Lock();
	}

	ok = ReadCurrentConfigFromFlash();
	return ok;
}


/*uint32_t CalcCRC(uint32_t *WxBuf)
{
//	uint32_t CRCVal = HAL_CRC_Calculate(&hcrc, (uint32_t *)WxBuf, sizeof(WxBuf)/sizeof(uint32_t));
	uint32_t CRCVal = HAL_CRC_Calculate(&hcrc, (uint32_t *)WxBuf, FLASH_DATA_SIZE);
	return CRCVal;
}*/


/*
CRC(data + CRC(data)) == 0.
если мы вычислили CRC и добавили его к данным — вычисление CRC этого пакета должно дать 0.
Таким образом, на принимающей стороне достаточно проверить, что CRC принятых данных равен 0 — значит, данные пришли полностью.

	uint32_t CRCVal = HAL_CRC_Calculate(&hcrc, (uint32_t *)WxBuf, sizeof(WxBuf)/sizeof(uint32_t));

На реальном устройстве сделать 2 структуры, тк записываются они разными сообщениями в разное время и по несвязным причинам.
Взять 2 последние страницы (сектора? то что стирается за раз) под хранение этих 2х структур, чтобы во время записи одной не стирать другую.
Записывать каждую структуру в свою страницу 3 раза со смещением (с запасом), для каждой структуры crc и счетчик записей.
При чтении читать первую структуру, если ее crc говорит о повреждении данных, читать вторую структуру.
После записи высылать ответрое сообщение об окончании сохранения в память, проверки, чтении и тд.
Обязательно прописать в инструкции, чтобы не рубали питание сразу после отправки сообщения о записи в память (ждать ответного сообщения).

Ответное сообщение 2 байта: код ошибки (000 - нет ош, 001 или 010 или 100 данные не считанны верно по 1,2 или 3 адресу), счетчик записи
*/

/* USER CODE BEGIN Header_StartTaskFLASH */
/**
* @brief Function implementing the TaskFLASH thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskFLASH */
void StartTaskFLASH(void const * argument)
{
  /* USER CODE BEGIN StartTaskFLASH */
	bool ok;
  /* Infinite loop */
  for(;;)
  {
	  if(StartWrite)
	  {
		  ok = WriteConfigToFlash();
		  StartWrite = false;
		  // выставить или сбросить ошибки
		  osDelay(3000);
		  // todo отправить сообщение об окончании записи
	  }

	  // сохранить в 3 местах
	  // проверить сохранение с CRC
	  // отправить ответное сообщение о завершении сохранения


    osDelay(500);
  }
  /* USER CODE END StartTaskFLASH */
}
