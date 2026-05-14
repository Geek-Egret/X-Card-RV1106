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
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum{
	POWER_DOWN_ = 0,
	POWER_ON_ = 1,
	DEBUG_EN_ = 2,
	DOWNLOAD_EN_ = 3
}USER_MODE_;

typedef struct{
	int32_t sys_stamp_sec;
	int32_t sys_stamp_msec;
}SYS_STAMP_;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern SYS_STAMP_ sys_stamp_now;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USER_KEY_Pin GPIO_PIN_0
#define USER_KEY_GPIO_Port GPIOA
#define USER_LED_R_Pin GPIO_PIN_5
#define USER_LED_R_GPIO_Port GPIOA
#define USER_LED_G_Pin GPIO_PIN_6
#define USER_LED_G_GPIO_Port GPIOA
#define USER_LED_B_Pin GPIO_PIN_7
#define USER_LED_B_GPIO_Port GPIOA
#define VCC3V3_EN_Pin GPIO_PIN_1
#define VCC3V3_EN_GPIO_Port GPIOB
#define VCC1V8_EN_Pin GPIO_PIN_8
#define VCC1V8_EN_GPIO_Port GPIOA
#define VCC0V9_EN_Pin GPIO_PIN_6
#define VCC0V9_EN_GPIO_Port GPIOC
#define VCC1V35_EN_Pin GPIO_PIN_11
#define VCC1V35_EN_GPIO_Port GPIOA
#define DEBUG_EN_Pin GPIO_PIN_12
#define DEBUG_EN_GPIO_Port GPIOA
#define USER_POR_Pin GPIO_PIN_3
#define USER_POR_GPIO_Port GPIOB
#define USER_BOOT_Pin GPIO_PIN_4
#define USER_BOOT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
// IT_TIME = 10ms
#define USER_LED_BLINK_DURATION 		(1000)		// USER_LED_BLINK_DURATION ms
#define USER_KEY_SCAN_DURATION 			(1)			// USER_KEY_SCAN_DURATION*IT_TIME ms
#define USER_KEY_SHORT_TIME				(1)			// USER_KEY_SHORT_TIME*USER_KEY_SCAN_DURATION ms
#define USER_KEY_LONG_TIME				(15)		// USER_KEY_LONG_TIME*USER_KEY_SCAN_DURATION ms
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
