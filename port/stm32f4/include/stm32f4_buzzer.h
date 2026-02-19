/**
 * @file stm32f4_buzzer.h
 * @brief Header for stm32f4_buzzer.c file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-05-21
 */
#ifndef STM32F4_BUZZER_SYSTEM_H_
#define STM32F4_BUZZER_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>

/* HW dependent includes */
#include "stm32f4xx.h"

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
/**
 * @brief Buzzer GPIO port
 * 
 */
#define 	STM32F4_PARKING_BUZZER_GPIO GPIOA
 
/**
 * @brief Buzzer GPIO pin
 * 
 */
#define 	STM32F4_PARKING_BUZZER_PIN 0

#endif /* STM32F4_BUZZER_SYSTEM_H_ */