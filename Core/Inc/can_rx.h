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

// коды для настройки пределов давления
#define ANSWER_CODE            0x55	// сообщения относятся к настройке 0
#define LIMIT_CALIBRATION_CODE 0xAA	// сообщения относятся к настройке 1; отправить границы и значение
#define LOW_LIMIT_CODE         0x05	// нижний предел, (без сохранения)
#define HIGH_LIMIT_CODE        0x15	// верхний предел (без сохранения)
#define LOW_LIMIT_SAVE_CODE    0x25	// нижний предел и дата, сохранить
#define HIGH_LIMIT_SAVE_CODE   0x35	// верхний предел и дата, сохранить
#define ALL_LIMIT_SAVE_CODE    0x45	// в и н пределы, дата, сохранить

void CAN_RegisterRxQueue(osMessageQId q);
bool ReadQueueCAN(CAN_MSG* pmsg);
void CodeProcessing(CAN_MSG* inMsg);

#endif /* INC_CAN_RX_H_ */
