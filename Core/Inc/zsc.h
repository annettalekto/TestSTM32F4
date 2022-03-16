/*
 * zsc.h
 *
 *  Created on: 9 мар. 2022 г.
 *      Author: nesterovaaa
 */

#ifndef INC_ZSC_H_
#define INC_ZSC_H_

#include "main.h"

typedef struct DataZSC_
{
  uint8_t HighByte;
  uint8_t LowByte;
  uint8_t Error; // 1 ошибка, 0 хорошо
} DATA_ZSC, *PDATA_ZSC;

DATA_ZSC GetZSC(void);
DATA_ZSC GetPressureZSC(void);

#endif /* INC_ZSC_H_ */
