/*
 * can_rx.h
 *
 *  Created on: Feb 21, 2022
 *      Author: nesterovaaa
 */

#ifndef INC_CAN_RX_H_
#define INC_CAN_RX_H_

#include "main.h"
#include "can.h"




void CAN_RegisterRxQueue(osMessageQId q);
bool ReadQueueCAN(CAN_MSG* pmsg);

#endif /* INC_CAN_RX_H_ */
