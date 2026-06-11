/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern DAC_HandleTypeDef hdac1;

extern I2C_HandleTypeDef hi2c2;

extern CRC_HandleTypeDef hcrc;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CS_BP_Pin GPIO_PIN_1
#define CS_BP_GPIO_Port GPIOF
#define OC_Pin GPIO_PIN_0
#define OC_GPIO_Port GPIOA
#define OC_EXTI_IRQn EXTI0_IRQn
#define nCLR_OC_Pin GPIO_PIN_1
#define nCLR_OC_GPIO_Port GPIOA
#define DRV_BP_Pin GPIO_PIN_2
#define DRV_BP_GPIO_Port GPIOA
#define OC_Setpoint_Pin GPIO_PIN_4
#define OC_Setpoint_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_6
#define LED_GPIO_Port GPIOA
#define INT_Pin GPIO_PIN_10
#define INT_GPIO_Port GPIOA
#define ALERT_Pin GPIO_PIN_11
#define ALERT_GPIO_Port GPIOA
#define nEN_Pin GPIO_PIN_12
#define nEN_GPIO_Port GPIOA
#define ESTOP_Pin GPIO_PIN_15
#define ESTOP_GPIO_Port GPIOA
#define nCLR_ESTOP_Pin GPIO_PIN_4
#define nCLR_ESTOP_GPIO_Port GPIOB
#define DRV_PC_Pin GPIO_PIN_7
#define DRV_PC_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
