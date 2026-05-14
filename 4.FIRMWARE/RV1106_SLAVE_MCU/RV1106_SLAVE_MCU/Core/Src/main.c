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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "user.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEBUG_EN 0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
SYS_STAMP_ sys_stamp_now;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
  int8_t red_en = 0;
  int8_t green_en = 0;
  int8_t blue_en = 0;
  int8_t power_state = 0;
  int8_t download_state = 0;
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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  USER_MODE_ user_mode = POWER_DOWN_;
  // Start TIM2 global interrupt 
  HAL_TIM_Base_Start_IT(&htim2);
  
  // SOC POWER ENABLE SEQUENCE
//  HAL_GPIO_WritePin(VCC0V9_EN_GPIO_Port, VCC0V9_EN_Pin, GPIO_PIN_SET);
//  HAL_Delay(10);
//  HAL_GPIO_WritePin(VCC1V35_EN_GPIO_Port, VCC1V35_EN_Pin, GPIO_PIN_SET);
//  HAL_Delay(10);
//  HAL_GPIO_WritePin(VCC1V8_EN_GPIO_Port, VCC1V8_EN_Pin, GPIO_PIN_SET);
//  HAL_Delay(10);
//  HAL_GPIO_WritePin(VCC3V3_EN_GPIO_Port, VCC3V3_EN_Pin, GPIO_PIN_SET);
//  HAL_Delay(10);

//	HAL_GPIO_WritePin(USER_POR_GPIO_Port, USER_POR_Pin, GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(USER_BOOT_GPIO_Port, USER_BOOT_Pin, GPIO_PIN_RESET);
	
		
//		HAL_GPIO_WritePin(USER_POR_GPIO_Port, USER_POR_Pin, GPIO_PIN_SET);
//		HAL_Delay(100);
//		HAL_GPIO_WritePin(USER_POR_GPIO_Port, USER_POR_Pin, GPIO_PIN_RESET);
//		HAL_Delay(100);
//		HAL_GPIO_WritePin(USER_BOOT_GPIO_Port, USER_BOOT_Pin, GPIO_PIN_RESET);
//		HAL_Delay(500);
//		HAL_GPIO_WritePin(USER_BOOT_GPIO_Port, USER_BOOT_Pin, GPIO_PIN_RESET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	 // USER_KEY SCAN
	 static SYS_STAMP_ sys_stamp_last_user_key_scan = {.sys_stamp_sec = 0, .sys_stamp_msec = 0};
	 static int16_t user_key_press_time_count = 0;
	 SYS_STAMP_ sys_stamp_duration_ = sys_stamp_duration(&sys_stamp_now, &sys_stamp_last_user_key_scan);
	 if(sys_stamp_duration_.sys_stamp_sec > 0 && sys_stamp_duration_.sys_stamp_sec*1000+sys_stamp_duration_.sys_stamp_msec > USER_KEY_SCAN_DURATION){
		if(HAL_GPIO_ReadPin(USER_KEY_GPIO_Port, USER_KEY_Pin) == 0){
			user_key_press_time_count++;
		}else{
			if(user_key_press_time_count > USER_KEY_SHORT_TIME && user_key_press_time_count <= 10){
				switch(user_mode){
					case POWER_DOWN_:{ user_mode = POWER_ON_; break; }
					case POWER_ON_:{ user_mode = POWER_DOWN_; break; }
					case DEBUG_EN_:{ user_mode = DOWNLOAD_EN_; break; }
					case DOWNLOAD_EN_:{ user_mode = POWER_ON_; break; }
					default:{ break; }
				}
			}else if(user_key_press_time_count > USER_KEY_LONG_TIME){
				switch(user_mode){
					case POWER_ON_:{ user_mode = DEBUG_EN_; break; }
					case DEBUG_EN_:{ user_mode = POWER_ON_; break; }
					default:{ break; }
				}
			}
			user_key_press_time_count = 0;
		}
		sys_stamp_last_user_key_scan.sys_stamp_sec = sys_stamp_now.sys_stamp_sec;
		sys_stamp_last_user_key_scan.sys_stamp_msec = sys_stamp_now.sys_stamp_msec;
	 }
	 // POWER_DOWN_
	 if (user_mode == POWER_DOWN_) {
		static SYS_STAMP_ sys_stamp_last_power_down = {.sys_stamp_sec = 0, .sys_stamp_msec = 0};
		SYS_STAMP_ sys_stamp_duration_ = sys_stamp_duration(&sys_stamp_now, &sys_stamp_last_power_down);
		if(sys_stamp_duration_.sys_stamp_sec > 0 && sys_stamp_duration_.sys_stamp_sec*1000+sys_stamp_duration_.sys_stamp_msec > USER_LED_BLINK_DURATION){
			red_en = !red_en;
			green_en = 0;
			blue_en = 0;
			user_led(red_en, green_en, blue_en);
			
			
			HAL_GPIO_WritePin(VCC3V3_EN_GPIO_Port, VCC3V3_EN_Pin, GPIO_PIN_RESET);
			HAL_Delay(10);
			HAL_GPIO_WritePin(VCC1V8_EN_GPIO_Port, VCC1V8_EN_Pin, GPIO_PIN_RESET);
			HAL_Delay(10);
			HAL_GPIO_WritePin(VCC1V35_EN_GPIO_Port, VCC1V35_EN_Pin, GPIO_PIN_RESET);
			HAL_Delay(10);
			HAL_GPIO_WritePin(VCC0V9_EN_GPIO_Port, VCC0V9_EN_Pin, GPIO_PIN_RESET);
			HAL_Delay(10);
			
			sys_stamp_last_power_down.sys_stamp_sec = sys_stamp_now.sys_stamp_sec;
			sys_stamp_last_power_down.sys_stamp_msec = sys_stamp_now.sys_stamp_msec;
		}
		power_state = 0;
	 }
	 // POWER_ON_
	 if(user_mode == POWER_ON_){
		HAL_GPIO_WritePin(DEBUG_EN_GPIO_Port, DEBUG_EN_Pin, DEBUG_EN);
		if(power_state == 0){
			red_en = 1;
			green_en = 0;
			blue_en = 0;
			user_led(red_en, green_en, blue_en);
			HAL_Delay(200);
			// SOC Power on sequence: 0V9->1V35(DDR)->1V8->3V3
			HAL_GPIO_WritePin(VCC0V9_EN_GPIO_Port, VCC0V9_EN_Pin, GPIO_PIN_SET);
			HAL_Delay(10);
			HAL_GPIO_WritePin(VCC1V35_EN_GPIO_Port, VCC1V35_EN_Pin, GPIO_PIN_SET);
			HAL_Delay(10);
			HAL_GPIO_WritePin(VCC1V8_EN_GPIO_Port, VCC1V8_EN_Pin, GPIO_PIN_SET);
			HAL_Delay(10);
			HAL_GPIO_WritePin(VCC3V3_EN_GPIO_Port, VCC3V3_EN_Pin, GPIO_PIN_SET);
			HAL_Delay(10);
			HAL_GPIO_WritePin(DEBUG_EN_GPIO_Port, DEBUG_EN_Pin, GPIO_PIN_RESET);
		}
		static SYS_STAMP_ sys_stamp_last_power_on = {.sys_stamp_sec = 0, .sys_stamp_msec = 0};
		SYS_STAMP_ sys_stamp_duration_ = sys_stamp_duration(&sys_stamp_now, &sys_stamp_last_power_on);
		if(sys_stamp_duration_.sys_stamp_sec > 0 && sys_stamp_duration_.sys_stamp_sec*1000+sys_stamp_duration_.sys_stamp_msec > USER_LED_BLINK_DURATION){
			red_en = 0;
			green_en = !green_en;
			blue_en = 0;
			user_led(red_en, green_en, blue_en);
			
			sys_stamp_last_power_on.sys_stamp_sec = sys_stamp_now.sys_stamp_sec;
			sys_stamp_last_power_on.sys_stamp_msec = sys_stamp_now.sys_stamp_msec;
		}
		power_state = 1;
		download_state = 0;
	 }
	 // DEBUG_EN
	 if(user_mode == DEBUG_EN_){
		HAL_GPIO_WritePin(DEBUG_EN_GPIO_Port, DEBUG_EN_Pin, GPIO_PIN_SET);
		static SYS_STAMP_ sys_stamp_last_debug_en = {.sys_stamp_sec = 0, .sys_stamp_msec = 0};
		SYS_STAMP_ sys_stamp_duration_ = sys_stamp_duration(&sys_stamp_now, &sys_stamp_last_debug_en);
		if(sys_stamp_duration_.sys_stamp_sec > 0 && sys_stamp_duration_.sys_stamp_sec*1000+sys_stamp_duration_.sys_stamp_msec > USER_LED_BLINK_DURATION){
			red_en = 0;
			green_en = 0;
			blue_en = !blue_en;
			user_led(red_en, green_en, blue_en);
			
			sys_stamp_last_debug_en.sys_stamp_sec = sys_stamp_now.sys_stamp_sec;
			sys_stamp_last_debug_en.sys_stamp_msec = sys_stamp_now.sys_stamp_msec;
		}
		download_state = 0;
	 }
	 // DOWNLOAD_EN
	 if(user_mode == DOWNLOAD_EN_){
		red_en = 1;
		green_en = 1;
		blue_en = 0;
		user_led(red_en, green_en, blue_en);
		if(download_state == 0){
			HAL_GPIO_WritePin(DEBUG_EN_GPIO_Port, DEBUG_EN_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(USER_BOOT_GPIO_Port, USER_BOOT_Pin, GPIO_PIN_SET);
			HAL_Delay(50);
			HAL_GPIO_WritePin(USER_POR_GPIO_Port, USER_POR_Pin, GPIO_PIN_SET);
			HAL_Delay(500);
			HAL_GPIO_WritePin(USER_POR_GPIO_Port, USER_POR_Pin, GPIO_PIN_RESET);
			HAL_Delay(500);
			HAL_GPIO_WritePin(USER_BOOT_GPIO_Port, USER_BOOT_Pin, GPIO_PIN_RESET);
		}
		download_state = 1;
	 }
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
