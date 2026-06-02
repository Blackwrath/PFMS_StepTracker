/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "ssd1351.h"
#include "TiledDisplayRenderer.h"
#include "tlc5973.h"
#include "StepIdentifier.h"
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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim1;

TLC5973_HandleTypeDef hTLC5973;

/* USER CODE BEGIN PV */

volatile unsigned char TDR_DMA_READY = 1;
int TDR_TPS = 0;



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */
void ST_Enable(void);
void ST_Disable(void);
uint16_t Read_ADC_Channel(uint32_t channel);
void Read_Accelerometer(uint16_t *x, uint16_t *y, uint16_t *z);
void ADXL_ST_Routine(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi->Instance == SPI1) {
		TDR_DMA_READY = 1;
		SSD1351_Unselect();
		TDR_TPS++;
	}
	return;
}

// Called when first half of buffer is filled
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {

}

// Called when buffer is completely filled
# define ADC_BUFFER_SIZE 4 * 16

volatile uint16_t ADC_Data[ADC_BUFFER_SIZE];
volatile uint16_t ADC_Data_Good[4];
volatile char usermessage[24];

volatile char ADC_DATA_REQUEST = 0;
volatile char ADC_DATA_READY = 0;

__attribute__((section(".flash_rw_area"))) Vector3 calib_data_FLASH;
Vector3 calib_data_RAM = {.x=0,.y=0,.z=0};
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
//	char data[4];
//	itoa(ADC_Data[0], data, 10);
//	strcpy(usermessage, data);
	if (ADC_DATA_REQUEST == 1)
	{
//		ADC_DATA_READY = 0;
//		ADC_DATA_REQUEST = 0;
		ADC_Data_Good[0] = 0;
		ADC_Data_Good[1] = 0;
		ADC_Data_Good[2] = 0;
		ADC_Data_Good[3] = 0;
		for (int i = 0; i < ADC_BUFFER_SIZE; i++)
		{
			ADC_Data_Good[i%4] += ADC_Data[i];
		}

//
//		ADC_Data_Good[2] = ADC_Data[0];
//		ADC_Data_Good[1] = ADC_Data[1];
//		ADC_Data_Good[0] = ADC_Data[2]; //fix here: reorder data from ZYXB to XYZB
//		ADC_Data_Good[3] = ADC_Data[3];


		ADC_Data_Good[0] = ADC_Data_Good[0] / (ADC_BUFFER_SIZE / 4);
		ADC_Data_Good[1] = ADC_Data_Good[1] / (ADC_BUFFER_SIZE / 4);
		ADC_Data_Good[2] = ADC_Data_Good[2] / (ADC_BUFFER_SIZE / 4);
		ADC_Data_Good[3] = ADC_Data_Good[3] / (ADC_BUFFER_SIZE / 4);

		uint16_t temp = ADC_Data_Good[0];
		ADC_Data_Good[0] = ADC_Data_Good[2];
		ADC_Data_Good[2] = temp;

		ADC_DATA_READY = 1;


		//data averaging
	}
}

static inline int max(int a, int b) {
    return (a > b) ? a : b;
}

void EMS_ADC_READ() //triggers read into ADC_Data_Good, waits for return
{
	ADC_DATA_REQUEST = 1;
	while(ADC_DATA_READY == 0);
}
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
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  HAL_ADCEx_Calibration_Start(&hadc1);
  MX_TIM1_Init();

  /* USER CODE BEGIN 2 */
  HAL_Delay(300);
  SSD1351_Unselect();
  SSD1351_Init();
  HAL_Delay(300);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	uint32_t steps = 0;
	uint32_t maxsteps = 1000;
	TDR_clear_screen();
	int ones = (steps) % 10;
	int tens = (steps / 10) % 10;
	int hundreds = (steps / 100) % 10;
	int thous = (steps / 1000) % 10;
	uint16_t framecounter = 0;
	uint32_t start = HAL_GetTick();
	uint32_t now = HAL_GetTick();
	uint32_t ms = 0;



	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_Data, ADC_BUFFER_SIZE);

	HAL_TIM_Base_Start(&htim1);

	/* Initialize TLC5973 */
	/* Using 2.0 us tCYCLE */
	TLC5973_Init(&hTLC5973, GPIO_SRDATA_GPIO_Port, GPIO_SRDATA_Pin, 1000);

	TDR_draw_string("CALIB DATA FLASH", 0, 0, 0);
	HAL_Delay(300);
	sprintf(usermessage, "%u", calib_data_FLASH.x);
	TDR_draw_string(usermessage, 0, 16, 0);
	HAL_Delay(50);
	sprintf(usermessage, "%u", calib_data_FLASH.y);
	TDR_draw_string(usermessage, 0, 32, 0);
	HAL_Delay(50);
	sprintf(usermessage, "%u", calib_data_FLASH.z);
	TDR_draw_string(usermessage, 0, 48, 0);
	HAL_Delay(50);

	calib_data_RAM.x = calib_data_FLASH.x;
	calib_data_RAM.y = calib_data_FLASH.y;
	calib_data_RAM.z = calib_data_FLASH.z;
	TDR_draw_string("Loaded!", 0, 64, 0);

	HAL_Delay(200);
	TDR_clear_screen();

	/* Write Channels: Ch0=Full, Ch1=Half, Ch2=Off */
	/* 12-bit range: 0 to 4095 */
	//TLC5973_WriteChannels(&hTLC5973, 4095, 4095, 4095);

#ifdef DEBUG
	int32_t debug_ticker = 0;
#endif
	//ATTENTION ALL CONTRIBUTORS: THE WHILE LOOP STARTS HERE
	while (steps < maxsteps) {
		EMS_ADC_READ();
		TDR_draw_number_small(ADC_Data_Good[0], 0, 0);
		TDR_draw_number_small(ADC_Data_Good[1], 0, 16);
		TDR_draw_number_small(ADC_Data_Good[2], 0, 32);
		/*************************** Self-Test Sequence**********************************/
		if (HAL_GPIO_ReadPin(GPIO_BUTTON1_GPIO_Port, GPIO_BUTTON1_Pin) == GPIO_PIN_RESET)
		//if (1)
		{
			TDR_clear_screen(); // Clear before resuming
		    ADXL_ST_Routine();

		    HAL_Delay(1000);   // Let user see result
		    TDR_clear_screen(); // Clear before resuming
		}
		/*********************************************************************************/

		/***************************Calibration Sequence**********************************/
		if (HAL_GPIO_ReadPin(GPIO_BUTTON2_GPIO_Port, GPIO_BUTTON2_Pin) == GPIO_PIN_RESET)
		{
			TDR_clear_screen(); // Clear before resuming
			ADXL_CALIB_Routine();

			HAL_Delay(1000);   // Let user see result
			TDR_clear_screen(); // Clear before resuming
		}
		/*********************************************************************************/
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		ones = (steps) % 10;
		tens = (steps / 10) % 10;
		hundreds = (steps / 100) % 10;
		thous = (steps / 1000) % 10;

		//	renderBackgroundCircle(steps, maxsteps);
		//	//HAL_Delay(1000);
		//	steps++;
		//	if(steps > 100) {steps = 0;}
		//TDR_ClearScreen();
//	  renderBackgroundSolid(0x0000);
//	  framecounter++;
//	  renderBackgroundSolid(0xFFFF);
//	  framecounter++;
		if (steps > maxsteps) {
			steps = 0;
		}


		//colours to calib data
		int32_t cx,cy,cz;
		cx = abs((int32_t)ADC_Data_Good[0] - calib_data_RAM.x);
		cy = abs((int32_t)ADC_Data_Good[1] - calib_data_RAM.y);
		cz = abs((int32_t)ADC_Data_Good[2] - calib_data_RAM.z);

		TLC5973_WriteChannels(&hTLC5973, max(cx-200,0),max(cy-200,0),(cz-200,0));

		TDR_draw_background_circle(steps, maxsteps);

		TDR_draw_number_sprite(ones, 80, 60);
		if (!ones) //ones is 0, update tens
		{
			TDR_draw_number_sprite(tens, 64, 60);
			if (!tens) //tens is 0 update hundreds
			{
				TDR_draw_number_sprite(hundreds, 48, 60);
				if (!hundreds) //tens is 0 update hundreds
				{
					TDR_draw_number_sprite(thous, 32, 60);
				}
			}
		}

		//TDR_draw_string(usermessage, 0, 0, 0);

		framecounter++;
		//HAL_Delay(20);

		now = HAL_GetTick(); //MILLISECOND TIMING CODE
//	  ms = now - start;
//	  start = now;
//	  //timing stats
//	  if (!(steps % 10))
//	  {
//		  ones = (ms) % 10;
//		  	  tens = (ms / 10) % 10;
//		  	  hundreds = (ms / 100) % 10;
//		  	  thous = (ms / 1000) % 10;
//		  	  TDR_draw_number_sprite(ones, 80, 44);
//		  	  TDR_draw_number_sprite(tens, 64, 44);
//		  	  TDR_draw_number_sprite(hundreds, 48, 44);
//		  	  TDR_draw_number_sprite(thous, 32, 44);
//	  }
		steps++;

		if (now - start > 1000) //DEBUG: FRAMERATE
				{
			//display framecounter
			ones = (framecounter) % 10;
			tens = (framecounter / 10) % 10;
			hundreds = (framecounter / 100) % 10;
			thous = (framecounter / 1000) % 10;
			TDR_draw_number_sprite(ones, 80, 44);
			TDR_draw_number_sprite(tens, 64, 44);
			TDR_draw_number_sprite(hundreds, 48, 44);
			TDR_draw_number_sprite(thous, 32, 44);
			start = now;
			framecounter = 0;
			TDR_TPS = 0;
		}



#ifdef DEBUG
		if (debug_ticker%5 == 0){
			TDR_draw_string("DEBUG BUILD", 0, 0, 0);
		}
		debug_ticker++;
#endif

	}
	TDR_draw_background_circle(10000, 10000);
	//END WHILE
	int counter = 0;
	while(1)
	{
		//TDR_draw_string("According to all known laws of aviation, there is no way a bee should be able to fly. Its wings are too small to get its fat little body off the ground. The bee, of course, flies anyway because bees don't care what humans think is impossible. Yellow, black. Yellow, black. Yellow, black. Yellow, black. Ooh, black and yellow! Let's shake it up a little. Barry! Breakfast is ready! Coming! Hang on a second. Hello? Barry? Adam? Can you believe this is happening? I can't. I'll pick you up. Looking sharp. Use the stairs, Your father paid good money for those. Sorry. I'm excited. Here's the graduate. We're very proud of you, son. A perfect report card, all B's. Very proud. Ma! I got a thing going here. You got lint on your fuzz. Ow! That's me! Wave to us! We'll be in row 118,000. Bye! Barry, I told you, stop flying in the house! Hey, Adam. Hey, Barry. Is that fuzz gel? A little. Special day, graduation. Never thought I'd make it. Three days grade school, three days high school. Those were awkward. Three days college. I'm glad I took a day and hitchhiked around The Hive. You did come back different. Hi, Barry. Artie, growing a mustache? Looks good. Hear about Frankie? Yeah. You going to the funeral? No, I'm not going. Everybody knows, sting someone, you die. Don't waste it on a squirrel. Such a hothead. I guess he could have just gotten out of the way. I love this incorporating an amusement park into our day. That's why we don't need vacations. Boy, quite a bit of pomp under the circumstances.", 0, -counter*16, 1);
		//TDR_draw_string("step count\nreached!", cosf((double)HAL_GetTick()/1000.0) * 24 + 24, sinf((double)HAL_GetTick()/1000.0) * 48 + 48, 0);
		//TDR_clear_screen();
		TDR_draw_string("FONT TEST\n !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", 0, -counter*16, 1);
		HAL_Delay(3000);
		TDR_clear_screen();
		ADXL_ST_Routine();
		HAL_Delay(3000);   // Let user see result
		TDR_clear_screen();
		//counter++;
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

  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_0);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};
  ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_SEQ_FIXED;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = ENABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T1_TRGO2;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_160CYCLES_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.Oversampling.Ratio = 1;
  hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_NONE;
  hadc1.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4; //Z channel
  sConfig.Rank = ADC_REGULAR_RANK_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5; //Y channel
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6; //X channel
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_7; //BAT channel
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analog WatchDog 1
  */
  AnalogWDGConfig.WatchdogNumber = ADC_ANALOGWATCHDOG_1;
  AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
  AnalogWDGConfig.Channel = ADC_CHANNEL_4;
  AnalogWDGConfig.ITMode = DISABLE;
  AnalogWDGConfig.HighThreshold = 0;
  AnalogWDGConfig.LowThreshold = 0;
  if (HAL_ADC_AnalogWDGConfig(&hadc1, &AnalogWDGConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 64;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 128;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SSD1351_RES_GPIO_Port, SSD1351_RES_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
//  HAL_GPIO_WritePin(GPIO_SRCLK_GPIO_Port, GPIO_SRCLK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SSD1351_CS_GPIO_Port, SSD1351_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SSD1351_DC_Pin|ADXL_ST_Pin|GPIO_SRDATA_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : SSD1351_RES_Pin */
  GPIO_InitStruct.Pin = SSD1351_RES_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SSD1351_RES_GPIO_Port, &GPIO_InitStruct);



//  /*Configure GPIO pin : GPIO_SRCLK_Pin */
//  GPIO_InitStruct.Pin = GPIO_SRCLK_Pin;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//  HAL_GPIO_Init(GPIO_SRCLK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SSD1351_CS_Pin SSD1351_DC_Pin ADXL_ST_Pin GPIO_SRCLR_Pin
                           GPIO_SRDATA_Pin */
  GPIO_InitStruct.Pin = SSD1351_CS_Pin|SSD1351_DC_Pin|ADXL_ST_Pin
                          |GPIO_SRDATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO_BUTTON1_Pin */
  GPIO_InitStruct.Pin = GPIO_BUTTON1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIO_BUTTON1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO_BUTTON2_Pin */
  GPIO_InitStruct.Pin = GPIO_BUTTON2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIO_BUTTON2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO_BUTTON2_Pin */
  GPIO_InitStruct.Pin = GPIO_BUTTON3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIO_BUTTON3_GPIO_Port, &GPIO_InitStruct);


  /*Configure GPIO pin : GPIO_BUTTON2_Pin */
  GPIO_InitStruct.Pin = GPIO_BUTTON4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIO_BUTTON4_GPIO_Port, &GPIO_InitStruct);


  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void ST_Enable(void)
{
    HAL_GPIO_WritePin(GPIOA, ADXL_ST_Pin, GPIO_PIN_SET);
}

void ST_Disable(void)
{
    HAL_GPIO_WritePin(GPIOA, ADXL_ST_Pin, GPIO_PIN_RESET);
}

void ADXL_ST_Routine(void)
{
    uint16_t x_normal, y_normal, z_normal;
    uint16_t x_st, y_st, z_st;
    int16_t dx, dy, dz;

    uint8_t retry = 1;

    while (retry)
    {
        // Normal mode
        ST_Disable();
        HAL_Delay(20);
        EMS_ADC_READ();
        HAL_Delay(20);
        //Read_Accelerometer(&x_normal, &y_normal, &z_normal);

        x_normal = ADC_Data_Good[0];
        y_normal = ADC_Data_Good[1];
        z_normal = ADC_Data_Good[2];


        // Enable self-test
        ST_Enable();
        HAL_Delay(20);
        EMS_ADC_READ();
        HAL_Delay(20);
        //Read_Accelerometer(&x_st, &y_st, &z_st);

        x_st = ADC_Data_Good[0];
        y_st = ADC_Data_Good[1];
        z_st = ADC_Data_Good[2];

        // Calculate difference
        dx = x_st - x_normal;
        dy = y_st - y_normal;
        dz = z_st - z_normal;

        TDR_draw_number_small(x_normal, 0, 16);
        TDR_draw_number_small(y_normal, 0, 32);
        TDR_draw_number_small(z_normal, 0, 48);

        TDR_draw_number_small(x_st, 40, 16);
        TDR_draw_number_small(y_st, 40, 32);
        TDR_draw_number_small(z_st, 40, 48);

        TDR_draw_number_small(dx, 80, 16);
        TDR_draw_number_small(dy, 80, 32);
        TDR_draw_number_small(dz, 80, 48);

        // Check limits
        if ((dx > 50 && dx < 800) &&
            (dy > 50 && dy < 800) &&
            (dz > 50 && dz < 800))
        {
            //TDR_clear_screen();
            TDR_draw_string("SELF TEST PASS", 0, 0, 1);

            ST_Disable();
            retry = 0;
        }
        else
        {
            //TDR_clear_screen();
            TDR_draw_string("SELF TEST FAIL", 0, 0, 1);

            if (HAL_GPIO_ReadPin(GPIO_BUTTON1_GPIO_Port, GPIO_BUTTON1_Pin) == GPIO_PIN_RESET)
            {
                retry = 1;
            }
            else
            {
                ST_Disable();
                retry = 0;
            }
        }
    }
}

HAL_StatusTypeDef DANGEROUS_Flash_Calib_Data()
{
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef erasetype;
	erasetype.NbPages = 1;
	erasetype.Page = 15;
	erasetype.TypeErase = FLASH_TYPEERASE_PAGES;
	uint32_t pageerror = 0;
	HAL_StatusTypeDef status;

	status = HAL_FLASHEx_Erase(&erasetype, &pageerror);
	if (status != HAL_OK) {
		HAL_FLASH_Lock();
		return status; // Erase failed
	}



	uint32_t address = &calib_data_FLASH.x;
	uint64_t data_buffer = ((uint64_t)calib_data_RAM.x) | ((uint64_t)calib_data_RAM.y << 32);
	status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data_buffer);
	if (status != HAL_OK) {
		HAL_FLASH_Lock();
		return status; // Write failed
	}

	address = &calib_data_FLASH.z;
	data_buffer = ((uint64_t)calib_data_RAM.z);
	status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data_buffer);
	if (status != HAL_OK) {
		HAL_FLASH_Lock();
		return status; // Write failed
	}

	HAL_FLASH_Lock();
	return HAL_OK;


}

void ADXL_CALIB_Routine()
{
	TDR_draw_string("Please place the\ndevice on a flat\nsurface, with\nthe screen\npointing upwards", 0, 0, 1);
	for(int8_t ticker = 5; ticker > 0; ticker--)
	{
		TDR_draw_number_sprite(ticker, 111, 111);
		HAL_Delay(1000);
	}
	//start sampling data
	uint32_t sx = 0;
	uint32_t sy = 0;
	uint32_t sz = 0;
	char buffer[32];
	TDR_clear_screen();
	uint32_t samplecount = 1;
	for (samplecount = 1; samplecount <= 1024; samplecount++)
	{
		EMS_ADC_READ();

		sx += ADC_Data_Good[0];
		sy += ADC_Data_Good[1];
		sz += ADC_Data_Good[2];
		if (samplecount%16 == 0)
		{

			sprintf(buffer, "Samples: %u", samplecount);
			TDR_draw_string(buffer, 0, 48-16,0);
			sprintf(buffer, "X: %u", sx);
			TDR_draw_string(buffer, 48, 48, 0);
			sprintf(buffer, "Y: %u", sy);
			TDR_draw_string(buffer, 48, 48+16, 0);
			sprintf(buffer, "Z: %u", sz);
			TDR_draw_string(buffer, 48, 48+32, 0);
		}
	}
	TDR_clear_screen();
	HAL_Delay(500);
	sx /= samplecount;
	sy /= samplecount;
	sz /= samplecount;
	sprintf(buffer, "Average:");
	TDR_draw_string(buffer, 0, 48-16,0);
	sprintf(buffer, "X: %u", sx);
	TDR_draw_string(buffer, 48, 48, 0);
	sprintf(buffer, "Y: %u", sy);
	TDR_draw_string(buffer, 48, 48+16, 0);
	sprintf(buffer, "Z: %u", sz);
	TDR_draw_string(buffer, 48, 48+32, 0);
	HAL_Delay(100);
	TDR_draw_string("saved to\ncalib_data...",0,48+48,0);
	calib_data_RAM.x = sx;
	calib_data_RAM.y = sy;
	calib_data_RAM.z = sz;
	DANGEROUS_Flash_Calib_Data();
	HAL_Delay(500);

}



void number_to_rgb(uint16_t input, uint8_t *r, uint8_t *g, uint8_t *b) {
    // 1. Scale 0-4095 to 0-1535 (6 segments of 256 steps)
    // 4095 / 1535 = ~2.667
	input = input%4096;
    uint16_t scaled = input / 3;

    // 2. Extract the segment (0 to 5) and phase/position within the segment (0 to 255)
    uint8_t segment = scaled / 256;
    uint8_t phase = scaled % 256;

    switch (segment) {
        case 0: // Red -> Yellow (Red=255, Green rises, Blue=0)
            *r = 255;
            *g = phase;
            *b = 0;
            break;
        case 1: // Yellow -> Green (Red falls, Green=255, Blue=0)
            *r = 255 - phase;
            *g = 255;
            *b = 0;
            break;
        case 2: // Green -> Cyan (Red=0, Green=255, Blue rises)
            *r = 0;
            *g = 255;
            *b = phase;
            break;
        case 3: // Cyan -> Blue (Red=0, Green falls, Blue=255)
            *r = 0;
            *g = 255 - phase;
            *b = 255;
            break;
        case 4: // Blue -> Magenta (Red rises, Green=0, Blue=255)
            *r = phase;
            *g = 0;
            *b = 255;
            break;
        case 5: // Magenta -> Red (Red=255, Green=0, Blue falls)
            *r = 255;
            *g = 0;
            *b = 255 - phase;
            break;
    }
}

/* USER CODE END 4 */

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
#ifdef USE_FULL_ASSERT
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
