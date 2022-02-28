/*
 * can_rx.c
 *
 *  Created on: Feb 21, 2022
 *      Author: nesterovaaa
 */

#include "can_rx.h"

QueueHandle_t rxQueue = NULL;

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
    uint32_t err = HAL_CAN_GetError(&hcan);
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
