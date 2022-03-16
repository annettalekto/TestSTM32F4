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

#define CONFIGURATION_LIMITS_CODE	0xC841D2F7U // код настройки пределов измерения
#define CONFIGURATION_CAN_CODE		0x7F2D148CU // код настройки скорости и ИД CAN

#define CONFIGURATION_CAN_ID		0x00

// коды для настройки пределов давления
#define ANSWER_CODE            0x55	// отправить давление (то же что запрос)
#define LIMIT_CALIBRATION_CODE 0xAA	// отправить границы и значение
#define LOW_LIMIT_CODE         0x05	// нижний предел, (без сохранения)
#define HIGH_LIMIT_CODE        0x15	// верхний предел (без сохранения)
#define LOW_LIMIT_SAVE_CODE    0x25	// нижний предел и дата, сохранить
#define HIGH_LIMIT_SAVE_CODE   0x35	// верхний предел и дата, сохранить
#define ALL_LIMIT_SAVE_CODE    0x45	// в и н пределы, дата, сохранить

bool CheckConfigurationCANCode(void);
void ResetConfigurationCANCode(void);
void CAN_RegisterRxQueue(osMessageQId q);
bool ReadQueueCAN(CAN_MSG* pmsg);
void CodeProcessing(CAN_MSG* inMsg);

#endif /* INC_CAN_RX_H_ */
