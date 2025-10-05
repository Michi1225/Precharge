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
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OC_PC_Pin GPIO_PIN_0
#define OC_PC_GPIO_Port GPIOF
#define CS_BP_Pin GPIO_PIN_1
#define CS_BP_GPIO_Port GPIOF
#define OC_BP_Pin GPIO_PIN_0
#define OC_BP_GPIO_Port GPIOA
#define nCLR_BP_Pin GPIO_PIN_1
#define nCLR_BP_GPIO_Port GPIOA
#define DRV_BP_Pin GPIO_PIN_2
#define DRV_BP_GPIO_Port GPIOA
#define CS_PC_Pin GPIO_PIN_3
#define CS_PC_GPIO_Port GPIOA
#define nCLR_PC_Pin GPIO_PIN_4
#define nCLR_PC_GPIO_Port GPIOA
#define IMON_Pin GPIO_PIN_5
#define IMON_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_6
#define LED_GPIO_Port GPIOA
#define RDY_Pin GPIO_PIN_8
#define RDY_GPIO_Port GPIOA
#define DONE_Pin GPIO_PIN_9
#define DONE_GPIO_Port GPIOA
#define OC_PC_OUT_Pin GPIO_PIN_10
#define OC_PC_OUT_GPIO_Port GPIOA
#define OC_BP_OUT_Pin GPIO_PIN_11
#define OC_BP_OUT_GPIO_Port GPIOA
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
