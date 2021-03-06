/*
 * can.c
 *
 *  Created on: Feb 21, 2022
 *      Author: nesterovaaa
 */

#include "can.h"

static CAN_HandleTypeDef *canHandle = NULL;

void StartCAN(CAN_HandleTypeDef *hcan)
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

// Инициализация CAN
bool InitCAN(PCONFIG_CAN pdata)
{
	bool ok = true;

	// если предделитель 80 и APB1 = 32МГц, то на скорость 25: 32 000/25 = 1280; 1280 = 80 * 16
	uint32_t clk = HAL_RCC_GetPCLK1Freq()/1000;									// APB1 = 32000
	uint32_t quanta = (uint32_t)(1 + (pdata->Tseg1 + 1) + (pdata->Tseg2 + 1));	// 16
	uint32_t prescaler = clk / (quanta * pdata->BaudRate);						// prescaler = (clk/1000) / (BR * Q)  -> 32 000 / 50 * 16 = 40

	canHandle->Instance = CAN1;
	canHandle->Init.Prescaler = prescaler;			//80;
	canHandle->Init.Mode = CAN_MODE_NORMAL;
	canHandle->Init.SyncJumpWidth = CAN_SJW_1TQ;	//? для каких то скоростей понадобиться todo решить этот вопрос таблицей
	canHandle->Init.TimeSeg1 = (pdata->Tseg1 << CAN_BTR_TS1_Pos); // CAN_BS1_13TQ = 0xC0000
	canHandle->Init.TimeSeg2 = (pdata->Tseg2 << CAN_BTR_TS2_Pos);
	canHandle->Init.TimeTriggeredMode = DISABLE;
	canHandle->Init.AutoBusOff = ENABLE;
	canHandle->Init.AutoWakeUp = DISABLE;
	canHandle->Init.AutoRetransmission = DISABLE;
	canHandle->Init.ReceiveFifoLocked = DISABLE;
	canHandle->Init.TransmitFifoPriority = DISABLE;

	if (HAL_CAN_Init(canHandle) != HAL_OK)
	{
		printf("Init CAN ERROR\n");
		ok = false;
	}

	// конфиг. фильтров на прием только одного ИД
	CAN_FilterTypeDef sFilterConfig;
	uint32_t id = (uint32_t)pdata->ID;

	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = id << 5;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = id << 5; // повторяется чтобы не принимался 0
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;

	if(HAL_CAN_ConfigFilter(canHandle, &sFilterConfig) != HAL_OK)
	{
		printf("Config filter CAN ERROR\n");
		ok = false;
	}

	return ok;
}

// Инициализировать CAN на начальные параметры: 25кбит/с ИД 0
void ResetCAN(void)
{
	CONFIG_CAN configData;

	// Для предделителя 80 и APB1 = 32МГц, скорость 25 кбит\с: 32 000/25 = 1280; 1280 = 80 * 16
	configData.BaudRate = 25;
	configData.ID = CONFIGURATION_CAN_ID;
	configData.Tseg1 = 12;	// CAN_BS1_13TQ
	configData.Tseg2 = 1;	// CAN_BS2_2TQ
	configData.UpLimit = 0;
	// записать в память? todo при ош. данных в памяти?
	InitCAN(&configData);
}

void InitCAN_FlashConfig(void)
{
	bool ok;

	CONFIG_CAN conf = GetConfigCAN();
	ok = CheckConfigCAN(&conf);
	if(ok)
		InitCAN(&conf);
	else
		ResetCAN();

	//todo выдать сообщ. об ош. по else
	// если код пришел то резет, если не пришел, но данные в памяти испорчены, тоже резет
}

// Проверка данных для инициализации CAN.
bool CheckConfigCAN(PCONFIG_CAN pdata)
{
  uint32_t id = pdata->ID;
  uint32_t bitSeg1 = pdata->Tseg1 + 1; // 1 - 16
  uint32_t bitSeg2 = pdata->Tseg2 + 1; // 1 - 8
  uint32_t quanta = 1 + bitSeg1 + bitSeg2; //

  bool ok = false;
  if((quanta >= 5) && (quanta <= 25)) //todo magical numbers
  {
    if((bitSeg1 > 1) && (bitSeg1 <= 16)) // magical namber todo
    {
      if((bitSeg2 >= 1) && (bitSeg2 <= 8))
      {
        ok = true;
      }
    }
  }

  if(id > (uint32_t)0x7FF)
    ok = false;

  if (!ok)
    printf("Данные для инициализации CAN не верны.\r\n");

  return ok;
}

// Получить данные для конфигурации CAN из сообщения
bool GetConfigCANfromMsg(CAN_MSG* msg, PCONFIG_CAN confData)
{
	bool ok;
	CONFIG_CAN cd;

	cd.ID = (uint32_t)( ((uint32_t)msg->data[0] << 24) | ((uint32_t)msg->data[1] << 16) | ((uint32_t)msg->data[2] << 8) | ((uint32_t)msg->data[3]) );
	cd.BaudRate = ( ((uint32_t) msg->data[4] << 8) | ((uint32_t) msg->data[5]) );
	cd.Tseg1 = ((uint32_t) msg->data[6] >> 4);
	cd.Tseg2 = ((uint32_t) msg->data[6] & 0x07);
	cd.UpLimit = ((uint32_t) msg->data[7]);
	// обработка расширенного ИД опущена todo

	ok = CheckConfigCAN(&cd);
	if(ok)
	{
		*confData = cd;
	}
	// SaveConfigCAN(confData);

	return ok;
}

bool CAN_Send(CAN_MSG* msg)
{
  // if (!isActiveCAN() || (NULL == msg) || !readyTransmitCAN()) { //todo
  //   return false;
  // }

  CAN_TxHeaderTypeDef TxHeader;
  uint32_t TxMailbox;

  TxHeader.StdId = msg->id;
  TxHeader.ExtId = 0;
  TxHeader.RTR = CAN_RTR_DATA;	// нет запросов
  TxHeader.IDE = CAN_ID_STD;	// нет расширенных
  TxHeader.DLC = msg->len;
  TxHeader.TransmitGlobalTime = DISABLE;

  //printf("send ID = %3X, len = %d\r\n", msg->id, msg->length);

  if (HAL_OK != HAL_CAN_AddTxMessage(canHandle, &TxHeader, msg->data, &TxMailbox)) {
    return false;
  }

  return true;
}

