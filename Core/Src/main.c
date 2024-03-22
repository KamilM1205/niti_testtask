/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include <string.h>
#include "queue.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef struct
{
  uint8_t data[5];
} rx_data_t;

typedef struct
{
  uint8_t data[5];
} tx_data_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define RX_QUEUE_SIZE   2
#define TX_QUEUE_SIZE   2
#define RX_ITEM_SIZE    sizeof(rx_data_t)
#define TX_ITEM_SIZE    sizeof(tx_data_t)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes =
  { .name = "defaultTask", .stack_size = 128 * 4, .priority =
      (osPriority_t) osPriorityNormal, };
/* USER CODE BEGIN PV */

osThreadId_t pingTaskHandle;
const osThreadAttr_t pingTask_attributes =
  { .name = "pingTask", .stack_size = 128 * 4, .priority =
      (osPriority_t) osPriorityNormal, };
osThreadId_t sendDataTaskHandle;
const osThreadAttr_t sendDataTask_attributes =
  { .name = "sendDataTask", .stack_size = 128 * 4, .priority =
      (osPriority_t) osPriorityNormal, };

osThreadId_t handleDataTaskHandle;
const osThreadAttr_t handleDataTask_attributes =
  { .name = "handleDataTask", .stack_size = 128 * 4, .priority =
      (osPriority_t) osPriorityHigh, };

/* Data receiving variables */
rx_data_t tmp;
uint8_t data_buf[9]; /* Buffer for recived data */
uint8_t data_recv_flag; /* True if we handled start bytes */
uint8_t check_sum; /* Check sum of received bytes */
uint8_t add_item_flag; /* If true we allocating new item to list */

QueueHandle_t rx_queue_handle;
QueueHandle_t tx_queue_handle;

uint8_t send_data;
volatile uint8_t ping;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void
SystemClock_Config (void);
static void
MX_GPIO_Init (void);
static void
MX_DMA_Init (void);
static void
MX_USART1_UART_Init (void);
static void
MX_TIM1_Init (void);
void
StartDefaultTask (void *argument);

/* USER CODE BEGIN PFP */

void
PingTask (void *arg);
void
ReceiveDataTask (void *arg);
void
SendDataTask (void *arg);
void
HandleDataTask (void *arg);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int
main (void)
{
  /* USER CODE BEGIN 1 */

  send_data = 1;

//  rx_queue_handle = xQueueCreateStatic(RX_QUEUE_SIZE, RX_ITEM_SIZE,
//				       &rx_queue_storage[0], &rx_queue_buff);
//  tx_queue_handle = xQueueCreateStatic(TX_QUEUE_SIZE, TX_ITEM_SIZE,
//				       &tx_queue_storage[0], &tx_queue_buff);

  rx_queue_handle = xQueueCreate(RX_QUEUE_SIZE, RX_ITEM_SIZE);
  tx_queue_handle = xQueueCreate(TX_QUEUE_SIZE, TX_ITEM_SIZE);

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init ();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config ();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init ();
  MX_DMA_Init ();
  MX_USART1_UART_Init ();
  MX_TIM1_Init ();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin (LED13_GPIO_Port, LED13_Pin, RESET);

  HAL_TIM_Base_Start_IT (&htim1);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize ();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew (StartDefaultTask, NULL,
				   &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  pingTaskHandle = osThreadNew (PingTask, NULL, &pingTask_attributes);
  sendDataTaskHandle = osThreadNew (SendDataTask, NULL,
				    &sendDataTask_attributes);
  handleDataTaskHandle = osThreadNew (HandleDataTask, NULL,
				      &handleDataTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart ();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
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
void
SystemClock_Config (void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct =
    { 0 };
  RCC_ClkInitTypeDef RCC_ClkInitStruct =
    { 0 };

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig (&RCC_OscInitStruct) != HAL_OK)
    {
      Error_Handler ();
    }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
      | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig (&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
      Error_Handler ();
    }
}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void
MX_TIM1_Init (void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig =
    { 0 };
  TIM_MasterConfigTypeDef sMasterConfig =
    { 0 };

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 8399;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 9999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init (&htim1) != HAL_OK)
    {
      Error_Handler ();
    }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource (&htim1, &sClockSourceConfig) != HAL_OK)
    {
      Error_Handler ();
    }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization (&htim1, &sMasterConfig) != HAL_OK)
    {
      Error_Handler ();
    }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void
MX_USART1_UART_Init (void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init (&huart1) != HAL_OK)
    {
      Error_Handler ();
    }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
 * Enable DMA controller clock
 */
static void
MX_DMA_Init (void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority (DMA2_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ (DMA2_Stream7_IRQn);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void
MX_GPIO_Init (void)
{
  GPIO_InitTypeDef GPIO_InitStruct =
    { 0 };

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin (LED13_GPIO_Port, LED13_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED13_Pin */
  GPIO_InitStruct.Pin = LED13_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init (LED13_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Button_Pin */
  GPIO_InitStruct.Pin = Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init (Button_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */

uint8_t
count_check_sum (uint8_t *data, size_t size)
{
  check_sum = 0;
  for (int i = 0; i < size; i++)
    {
      check_sum ^= data[i];
    }

  return check_sum;
}

void
PingTask (void *arg)
{
  tx_data_t data =
    { .data =
      { 0x01, 0x02, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5A }, };

  for (;;)
    {
      if (ping != 0)
	{
	  memmove (&data.data[2], &"png", 3);
	  data.data[6] = HAL_GPIO_ReadPin (LED13_GPIO_Port, LED13_Pin);
	  data.data[7] = count_check_sum (&data.data[2], 5);
	  xQueueSendToBack(tx_queue_handle, &data, portMAX_DELAY);
	  ping--;
	}
      osDelay (1);
    }
}

void
SendDataTask (void *arg)
{
  tx_data_t item;

  HAL_UART_Transmit_DMA (&huart1, (uint8_t*) "System started\n", 15);
  for (;;)
    {
      if (send_data != 0)
	{
	  if (xQueueReceive (tx_queue_handle, &item, 0) == pdTRUE)
	    {
	      HAL_UART_Transmit_DMA (&huart1, (uint8_t*) &item.data, 9);
	      send_data = 0;
	    }
	}
      osDelay (1);
    }
}

/* Test data to send: 01026c656400006d5a 01 02 65 63 68 00 65 0B 5A*/

void
HandleDataTask (void *arg)
{
  uint8_t data[9] =
    { 0x01, 0x02, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x5A };
  rx_data_t item;
  char cmd[5];

  for (;;)
    {
      xQueueReceive (rx_queue_handle, &item, portMAX_DELAY);
      memcpy (&cmd, item.data, 4);
      if (strcmp (cmd, "ech") == 0)
	{
	  data[2] = item.data[6];
	  data[7] = count_check_sum (&data[2], 5);
	  xQueueSendToBack(tx_queue_handle, &item, portMAX_DELAY);
	}
      if (strcmp (cmd, "led") == 0)
	{
	  HAL_GPIO_TogglePin (LED13_GPIO_Port, LED13_Pin);
	}

      osDelay (1);
    }
}

void
HAL_UART_RxCpltCallback (UART_HandleTypeDef *huart)
{
  if (huart == &huart1)
    {
      BaseType_t xHigherPriotity;

      if (data_buf[0] == 0x01 && data_buf[1] == 0x02 && data_buf[8] == 0x5A)
	{
	  if (data_buf[7] != count_check_sum (&data_buf[2], 5))
	    {
	      goto end;
	    }

	  memcpy (&tmp.data, &data_buf[2], 5);
	  xQueueSendToBackFromISR(rx_queue_handle, &tmp, &xHigherPriotity);
	  portYIELD_FROM_ISR(xHigherPriotity);
	}

end:  HAL_UART_Receive_IT (&huart1, (uint8_t*) &data_buf, 9);
    }
}

void
HAL_UART_TxCpltCallback (UART_HandleTypeDef *huart)
{
  if (huart == &huart1)
    {
      send_data = 1;
    }
}

/* USER CODE END Header_StartDefaultTask */
void
StartDefaultTask (void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for (;;)
    {
      osDelay (1);
    }
  /* USER CODE END 5 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void
Error_Handler (void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq ();
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