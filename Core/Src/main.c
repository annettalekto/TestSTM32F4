/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

CRC_HandleTypeDef hcrc;

osThreadId MsgProcessingTaHandle;
osThreadId TaskZSCHandle;
osThreadId TaskFLASHHandle;
osMessageQId RxQueueCANHandle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_CRC_Init(void);
void StartMsgProcessingTask(void const * argument);
extern void StartTaskZSC(void const * argument);
extern void StartTaskFLASH(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN1_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */

  StartCAN(&hcan1);
  // todo получить данные из памяти
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of RxQueueCAN */
  osMessageQDef(RxQueueCAN, 16, CAN_MSG);
  RxQueueCANHandle = osMessageCreate(osMessageQ(RxQueueCAN), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  CAN_RegisterRxQueue(RxQueueCANHandle);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of MsgProcessingTa */
  osThreadDef(MsgProcessingTa, StartMsgProcessingTask, osPriorityNormal, 0, 128);
  MsgProcessingTaHandle = osThreadCreate(osThread(MsgProcessingTa), NULL);

  /* definition and creation of TaskZSC */
  osThreadDef(TaskZSC, StartTaskZSC, osPriorityNormal, 0, 128);
  TaskZSCHandle = osThreadCreate(osThread(TaskZSC), NULL);

  /* definition and creation of TaskFLASH */
  osThreadDef(TaskFLASH, StartTaskFLASH, osPriorityIdle, 0, 128);
  TaskFLASHHandle = osThreadCreate(osThread(TaskFLASH), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  printf("START\n");

  HAL_GPIO_WritePin(RedLight_GPIO_Port, RedLight_Pin, 1);
  HAL_GPIO_WritePin(GreenLigh_GPIO_Port, GreenLigh_Pin, 1);

  uint32_t startTime = HAL_GetTick();
  while((HAL_GetTick() - startTime) < 1500)
  {
	  // ждем прихода кода для настройки
	  if (CheckConfigurationCANCode())
	  {
		  ResetCAN(); // переинициализация, ждем настройки
		  ResetConfigurationCANCode();
	  }
  }
  HAL_GPIO_WritePin(RedLight_GPIO_Port, RedLight_Pin, 0);
  HAL_GPIO_WritePin(GreenLigh_GPIO_Port, GreenLigh_Pin, 0);

  // если код настройки не пришел, инициализируемя из памяти
  InitCAN_FlashConfig();

  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  union FlashData1 WriteBuf;
//  union FlashData1 ReadBuf;
//  WriteBuf.DeviceConfig.ConfigCAN.ID = 1;
//    WriteBuf.DeviceConfig.Sensor.Level = 2;
//    WriteBuf.DeviceConfig.WriteCounter = 3;
//    WriteBuf.DeviceConfig.Crc = 4;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  // после вкл питания на ~1 сек делаем 25 кбит/сек  ИД 0, фильтр на прием кодов 7FF.
  // предделитель 80 при APB1 = 32МГц, на скорость 25: 32 000/25 = 1280; 1280 = 80 * 16.

	CAN_FilterTypeDef  sFilterConfig;

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */
  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 80;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */
  // для расширенного  ИД сдвигать по другому!
  sFilterConfig.FilterBank = 0; 
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x07FF << 5;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x07FF << 5; //оставить 0?
  sFilterConfig.FilterMaskIdLow = 0x000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;

  if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
  {
	  printf("Config CAN ERROR\n");
	  Error_Handler();
  }

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GreenLigh_Pin|RedLight_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : GreenLigh_Pin RedLight_Pin */
  GPIO_InitStruct.Pin = GreenLigh_Pin|RedLight_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
int _write(int fd, char* ptr, int len) {
#if 0
  HAL_StatusTypeDef hstatus;

  hstatus = HAL_UART_Transmit(&huart8, (uint8_t *) ptr, len, HAL_MAX_DELAY);
  if (hstatus == HAL_OK)
    return len;

  return -1;
#else
  for (int i = 0; i < len; i++) {
    ITM_SendChar((*ptr++));
  }

  return len;
#endif
}

/*int _write ( int file, char *ptr, int len )
{
  int i = 0;
  for ( i = 0; i < len; i++ )
    ITM_SendChar ( *ptr++ );
  return len;
}*/
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartMsgProcessingTask */
/**
  * @brief  Function implementing the MsgProcessingTa thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartMsgProcessingTask */
void StartMsgProcessingTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
	CAN_MSG msg;
  /* Infinite loop */
  for(;;)
  {
	  //3
	  ReadCurrentConfigFromFlash();
	  SENSOR_SETTINGS newSS = GetSensorSettings();
	  newSS.ChangeTime++;
	  newSS.HighLimit++;
	  newSS.LowLimit++;
	  SaveSensorSettings(newSS);
	  SaveCurrentConfigToFlash(); //+

	  // а это рабочее
	  /*if (pdTRUE == xQueueReceive(RxQueueCANHandle, &msg, 0))
	  {
		  printf("Message processing task: 0x%X\n", msg.id);
		  CodeProcessing(&msg);
	  }*/
		osDelay(1000);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM14 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM14) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

