/*
 * can_rx.c
 *
 *  Created on: Feb 21, 2022
 *      Author: nesterovaaa
 */

#include "can_rx.h"

bool SendPressure(void);
bool SendLimits();
bool SendLimitsAndValueZSC();

QueueHandle_t rxQueue = NULL;

extern volatile SENSOR_SETTINGS SensorSettings; //zsc.c


void CAN_RegisterRxQueue(osMessageQId q)
{
  rxQueue = q;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[8];
	CAN_MSG msg;
	BaseType_t xHigherPriorityTaskWoken;

    if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
    {
    	msg.id = RxHeader.StdId;
    	msg.rtr = RxHeader.RTR;
        if (RxHeader.DLC <= 8)
        {
          msg.len = RxHeader.DLC;

          memcpy(msg.data, RxData, msg.len);

			if (NULL != rxQueue)
			{
				xQueueSendFromISR(rxQueue, &msg, &xHigherPriorityTaskWoken);
				printf("Receive OK, ID=0x%X \n", RxHeader.StdId);
			}
        }
    }
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    HAL_CAN_GetError(hcan);
    printf("ERROR CAN\n");
}

bool ReadQueueCAN(CAN_MSG* pmsg)
{
	if (pdTRUE == xQueueReceive(rxQueue, pmsg, 0))
	{
		return true;
	}
	return false;
}

void CodeProcessing(CAN_MSG* inMsg)
{
//	+ проверку на код конфиг и настройки

	// ответ на запрос:
	if (inMsg->rtr)
	{
		SendPressure();
		return;
	}

	if (inMsg->data[0] == ANSWER_CODE) //55
	{
		SendPressure();
		return;
	}

	// настройка пределов:
	if (inMsg->data[0] == LIMIT_CALIBRATION_CODE)
	{
		switch (inMsg->data[1])
		{
			case LIMIT_CALIBRATION_CODE: //aa aa
			{
				SendLimitsAndValueZSC();
				break;
			}

			case LOW_LIMIT_CODE://aa 05
			{
				SENSOR_SETTINGS newSS = GetSensorSettings();
				newSS.LowLimit = (uint16_t)(inMsg->data[2]<<8) + (uint16_t)inMsg->data[3];
				SaveSensorSettings(newSS);
				SendLimitsAndValueZSC();
				break;
			}

			case HIGH_LIMIT_CODE://aa 15
			{
				SENSOR_SETTINGS newSS = GetSensorSettings();
				newSS.HighLimit = (uint16_t)(inMsg->data[2]<<8) + (uint16_t)inMsg->data[3];
				SaveSensorSettings(newSS);
				SendLimitsAndValueZSC();
				break;
			}

			case LOW_LIMIT_SAVE_CODE://aa 25
			{
				SENSOR_SETTINGS newSS = GetSensorSettings();
				newSS.LowLimit = (uint16_t)(inMsg->data[2]<<8) + (uint16_t)inMsg->data[3];
				newSS.ChangeTime = (uint16_t)(inMsg->data[4]<<8) + (uint16_t)inMsg->data[5];

				SaveFlashSensorSettings(newSS);
				SendLimitsAndValueZSC();
				break;
			}

			case HIGH_LIMIT_SAVE_CODE://aa 35
			{
				SENSOR_SETTINGS newSS = GetSensorSettings();
				newSS.HighLimit = (uint16_t)(inMsg->data[2]<<8) + (uint16_t)inMsg->data[3];
				newSS.ChangeTime = (uint16_t)(inMsg->data[4]<<8) + (uint16_t)inMsg->data[5];

				SaveFlashSensorSettings(newSS);
				SendLimitsAndValueZSC();
				break;
			}

			case ALL_LIMIT_SAVE_CODE://aa 45
			{
				SENSOR_SETTINGS newSS = GetSensorSettings();
				newSS.LowLimit = (uint16_t)(inMsg->data[2]<<8) + (uint16_t)inMsg->data[3];
				newSS.HighLimit = (uint16_t)(inMsg->data[4]<<8) + (uint16_t)inMsg->data[5];
				newSS.ChangeTime = (uint16_t)(inMsg->data[6]<<8) + (uint16_t)inMsg->data[7];

				SaveFlashSensorSettings(newSS);
				SendLimitsAndValueZSC();
				break;
			}
      // default: ;
		}
	}
}

/*
ответные сообщения
1 ответ на запрос (3 байта: пресчитанные данные 2 байта + ошибка)- отправлять в прерывании? где хранить данные?
2 ответ границами в режиме настройки
3 ответ границами и сырыми данными
*/

// Отправить ответ на удаленный запрос (давление + ошибка)
bool SendPressure(void)
{
  CAN_MSG msg;
  DATA_ZSC pressure = GetPressureZSC();

  msg.id = GetConfigCANID();
  msg.len = 3;
  msg.rtr = false;

  msg.data[0] = pressure.Error;
  msg.data[1] = pressure.HighByte;
  msg.data[2] = pressure.LowByte;

  return CAN_Send(&msg);
}


// ответ 2 границы
bool SendLimits()
{
  CAN_MSG msg;
  SENSOR_SETTINGS ss = GetSensorSettings();

  msg.id = GetConfigCANID();
  msg.len = 8;
  msg.rtr = false;

  msg.data[0] = (uint8_t)(ss.LowLimit >> 8);
  msg.data[1] = (uint8_t)ss.LowLimit;
  msg.data[2] = (uint8_t)(ss.HighLimit >> 8);
  msg.data[3] = (uint8_t)ss.HighLimit;
  msg.data[4] = (uint8_t)(ss.LowLimit >> 8);
  msg.data[5] = (uint8_t)ss.LowLimit;
  msg.data[6] = (uint8_t)(ss.ChangeTime >> 8);
  msg.data[7] = (uint8_t)(ss.ChangeTime);

  return CAN_Send(&msg);
}

// ответ 3 границы и сырые данные
bool SendLimitsAndValueZSC()
{
  CAN_MSG msg;
  DATA_ZSC zsc = GetZSC();
  SENSOR_SETTINGS ss = GetSensorSettings();

  msg.id = GetConfigCANID();
  msg.len = 8;
  msg.rtr = false;

  msg.data[0] = (uint8_t)(ss.LowLimit >> 8);
  msg.data[1] = (uint8_t)ss.LowLimit;
  msg.data[2] = (uint8_t)(ss.HighLimit >> 8);
  msg.data[3] = (uint8_t)ss.HighLimit;
  msg.data[4] = (uint8_t)(zsc.HighByte);
  msg.data[5] = (uint8_t)(zsc.LowByte);
  msg.data[6] = (uint8_t)(ss.ChangeTime >> 8);
  msg.data[7] = (uint8_t)(ss.ChangeTime);

  return CAN_Send(&msg);
}
