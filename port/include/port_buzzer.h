/**
 * @file port_buzzer.h
 * @brief Header for the portable functions to interact with the HW of the buzzer system. The functions must be implemented in the platform-specific code.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-05-21
 */
#ifndef PORT_BUZZER_SYSTEM_H_
#define PORT_BUZZER_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
/**
 * @brief Buzzer system identifier for the rear parking sensor
 * 
 */
#define 	PORT_PARKING_BUZZER_ID   0

/**
 * @brief Maximum value for the buzzer
 * 
 */
#define 	PORT_BUZZER_MAX_VALUE 255

/**
 * @brief Minimum value for the buzzer
 * 
 */
#define 	PORT_BUZZER_MIN_VALUE 0
 


/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Configure the HW specifications of a given buzzer.
 * 
 * @param buzzer_id Buzzer ID. This index is used to select the element of the `buzzers_arr[]` array
 */
void port_buzzer_init	(uint32_t buzzer_id);	

/**
 * @brief Set the Capture/Compare register values for the buzzer.
 * 
 * This function disables the timer associated to the buzzer, sets the Capture/Compare register values for the buzzer, and enables the timer.
 * 
 * @param buzzer_id Buzzer system identifier number.
 * @param sound 	Sound to set.
 */
void port_buzzer_set_sound(uint32_t 	buzzer_id, uint8_t sound );


#endif /* PORT_BUZZER_SYSTEM_H_ */