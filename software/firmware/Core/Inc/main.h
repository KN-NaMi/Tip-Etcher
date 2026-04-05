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

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BOARD_BTN_Pin GPIO_PIN_13
#define BOARD_BTN_GPIO_Port GPIOC
#define OLED_RESET_Pin GPIO_PIN_0
#define OLED_RESET_GPIO_Port GPIOA
#define OLED_DC_Pin GPIO_PIN_1
#define OLED_DC_GPIO_Port GPIOA
#define OLED_CS_Pin GPIO_PIN_4
#define OLED_CS_GPIO_Port GPIOA
#define M_ENABLE_Pin GPIO_PIN_6
#define M_ENABLE_GPIO_Port GPIOA
#define ENCODER_BTN_Pin GPIO_PIN_7
#define ENCODER_BTN_GPIO_Port GPIOA
#define ENCODER_BTN_EXTI_IRQn EXTI9_5_IRQn
#define LIMIT_SWITCH_1_Pin GPIO_PIN_10
#define LIMIT_SWITCH_1_GPIO_Port GPIOB
#define OLED_SPI2_SCK_Pin GPIO_PIN_13
#define OLED_SPI2_SCK_GPIO_Port GPIOB
#define OLED_SPI2_MOSI_Pin GPIO_PIN_15
#define OLED_SPI2_MOSI_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_6
#define LED_GPIO_Port GPIOC
#define ENCODER_SIGA_Pin GPIO_PIN_8
#define ENCODER_SIGA_GPIO_Port GPIOA
#define ENCODER_SIGA_EXTI_IRQn EXTI9_5_IRQn
#define ENCODER_SIGB_Pin GPIO_PIN_9
#define ENCODER_SIGB_GPIO_Port GPIOA
#define TIP_EN_Pin GPIO_PIN_12
#define TIP_EN_GPIO_Port GPIOA
#define V_SELECT_Pin GPIO_PIN_10
#define V_SELECT_GPIO_Port GPIOC
#define DIR_1_Pin GPIO_PIN_3
#define DIR_1_GPIO_Port GPIOB
#define STEP_1_Pin GPIO_PIN_4
#define STEP_1_GPIO_Port GPIOB
#define DIR_2_Pin GPIO_PIN_5
#define DIR_2_GPIO_Port GPIOB
#define STEP_2_Pin GPIO_PIN_6
#define STEP_2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
