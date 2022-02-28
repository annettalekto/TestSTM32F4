/*
 * can.c
 *
 *  Created on: Feb 21, 2022
 *      Author: nesterovaaa
 */

#include "can.h"


static CAN_HandleTypeDef *canHandle = NULL;

void CAN_Start(CAN_HandleTypeDef *hcan)
{
	canHandle = hcan;
	if (HAL_CAN_ActivateNotification(canHandle, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE) != HAL_OK)
	{
		printf("Enable interrupts CAN ERROR\n");
	}
	if (HAL_CAN_Start(canHandle) != HAL_OK)
	{
		printf("Start CAN ERROR\n");
	}
}
