/**
 * @file stm32f4_display.h
 * @brief Header for stm32f4_display.c file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-04-02
 */
#ifndef STM32F4_DISPLAY_SYSTEM_H_
#define STM32F4_DISPLAY_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>

/* HW dependent includes */
#include "stm32f4xx.h"

/* Defines and enums ----------------------------------------------------------*/
/**
 * @brief REAR Red LED GPIO port
 * 
 */
#define 	STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO GPIOB
 
/**
 * @brief REAR Red LED GPIO pin
 * 
 */
#define 	STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN 6
 
/**
 * @brief REAR Green LED GPIO port
 * 
 */
#define 	STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO GPIOB
 
/**
 * @brief REAR Green LED GPIO pin
 * 
 */
#define 	STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN 8
 
/**
 * @brief REAR Blue LED GPIO port
 * 
 */
#define 	STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO GPIOB
 
/**
 * @brief REAR Blue LED GPIO pin
 * 
 */
#define 	STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN 9

/**
 * @brief FRONT Red LED GPIO port
 * 
 */
#define 	STM32F4_FRONT_PARKING_DISPLAY_RGB_R_GPIO GPIOB
 
/**
 * @brief FRONT Red LED GPIO pin
 * 
 */
#define 	STM32F4_FRONT_PARKING_DISPLAY_RGB_R_PIN 4
 
/**
 * @brief FRONT Green LED GPIO port
 * 
 */
#define 	STM32F4_FRONT_PARKING_DISPLAY_RGB_G_GPIO GPIOB
 
/**
 * @brief FRONT Green LED GPIO pin
 * 
 */
#define 	STM32F4_FRONT_PARKING_DISPLAY_RGB_G_PIN 0
 
/**
 * @brief FRONT Blue LED GPIO port
 * 
 */
#define 	STM32F4_FRONT_PARKING_DISPLAY_RGB_B_GPIO GPIOB
 
/**
 * @brief FRONT Blue LED GPIO pin
 * 
 */
#define 	STM32F4_FRONT_PARKING_DISPLAY_RGB_B_PIN 1

#endif /* STM32F4_DISPLAY_SYSTEM_H_ */