/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32c0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SSD1351_RES_Pin GPIO_PIN_7
#define SSD1351_RES_GPIO_Port GPIOB
#define GPIO_BUTTON1_Pin GPIO_PIN_14
#define GPIO_BUTTON1_GPIO_Port GPIOC
#define GPIO_SRCLK_Pin GPIO_PIN_15
#define GPIO_SRCLK_GPIO_Port GPIOC
#define SSD1351_CS_Pin GPIO_PIN_0
#define SSD1351_CS_GPIO_Port GPIOA
#define SSD1351_SCL_Pin GPIO_PIN_1
#define SSD1351_SCL_GPIO_Port GPIOA
#define SSD1351_SDA_Pin GPIO_PIN_2
#define SSD1351_SDA_GPIO_Port GPIOA
#define SSD1351_DC_Pin GPIO_PIN_3
#define SSD1351_DC_GPIO_Port GPIOA
#define ADXL_X_Pin GPIO_PIN_4
#define ADXL_X_GPIO_Port GPIOA
#define ADXL_Y_Pin GPIO_PIN_5
#define ADXL_Y_GPIO_Port GPIOA
#define ADXL_Z_Pin GPIO_PIN_6
#define ADXL_Z_GPIO_Port GPIOA
#define BAT_VOL_Pin GPIO_PIN_7
#define BAT_VOL_GPIO_Port GPIOA
#define ADXL_ST_Pin GPIO_PIN_8
#define ADXL_ST_GPIO_Port GPIOA
#define GPIO_SRCLR_Pin GPIO_PIN_11
#define GPIO_SRCLR_GPIO_Port GPIOA
#define GPIO_SRDATA_Pin GPIO_PIN_12
#define GPIO_SRDATA_GPIO_Port GPIOA
#define GPIO_BUTTON2_Pin GPIO_PIN_6
#define GPIO_BUTTON2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
