/*
 * zsc.c
 *
 *  Created on: 9 мар. 2022 г.
 *      Author: nesterovaaa
 */

#include "zsc.h"

DATA_ZSC DataZSC;

void ReadZSC(void)
{
	// получить сырые данные с датчика, сохранить
	// todo

	DataZSC.Error = 0; //обнулять каждый раз
	DataZSC.HighByte = 1; // исключить изменение данных во время отправки
	DataZSC.LowByte = 2;
}

// получить сырые данные ZSC
DATA_ZSC GetZSC(void)
{
	return DataZSC;
}

// получить пересчитанные данные ZSC
DATA_ZSC GetPressureZSC(void)
{
	SENSOR_SETTINGS ss = GetSensorSettings();
	uint16_t data = (uint16_t)(DataZSC.HighByte<<8) + (uint16_t)DataZSC.LowByte;
	float low = (float)ss.LowLimit;
	float high = (float)ss.HighLimit;
	float range = high - low;

	// fOutput = ((float)10000/(float)range_limit)*(((float)(uDataMass[0]<<8) + (float)uDataMass[1]) - (float)low_limit);
	// fOutputU16 = fOutput;
	uint16_t result = (uint16_t)((10000./range) * ((float)data - low));

	DATA_ZSC pressure;
	pressure.Error = DataZSC.Error; 			// 0 byte of msg
	pressure.HighByte = (uint8_t)(result >> 8);	// 1 byte
	pressure.LowByte = (uint8_t)result; 		// 2 byte

	return pressure;
}

/* USER CODE BEGIN Header_StartTaskZSC */
/**
* @brief Function implementing the TaskZSC thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskZSC */
void StartTaskZSC(void const * argument)
{
  /* USER CODE BEGIN StartTaskZSC */
  /* Infinite loop */
  for(;;)
  {
	  // прием данных от микросхемы
    osDelay(1);
  }
  /* USER CODE END StartTaskZSC */
}
