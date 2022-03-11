/*
 * can.h
 *
 *  Created on: Feb 21, 2022
 *      Author: nesterovaaa
 */

#include "main.h"
#include "can_rx.h"

#ifndef INC_CAN_H_
#define INC_CAN_H_

#define CAN_MAX_DATA_SIZE   8
typedef struct can_msg
{
  uint32_t id;
  uint32_t len;
  uint32_t rtr;
  uint8_t data[CAN_MAX_DATA_SIZE];
} CAN_MSG;

void StartCAN(CAN_HandleTypeDef *hcan);
bool InitCAN(PCONFIG_CAN pdata);
bool ConfigFilterCAN(PCONFIG_CAN pdata);
bool CheckConfigData(PCONFIG_CAN pdata);
void ResetConfigCAN(void);
bool CAN_Send(CAN_MSG * msg);

#endif /* INC_CAN_H_ */
