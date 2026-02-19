/**
 * @file port_button.h
 * @brief Header for the portable functions to interact with the HW of the buttons. The functions must be implemented in the platform-specific code.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-03-26
 */

#ifndef PORT_BUTTON_H_
#define PORT_BUTTON_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
// Define here all the button identifiers that are used in the system
/**
 * @brief User button identifier that represents the rear button (activation of the parking aid system).
 * 
 */
#define PORT_PARKING_BUTTON_ID 0

/**
 * @brief Button debounce time in milliseconds.
 * 
 */
#define PORT_PARKING_BUTTON_DEBOUNCE_TIME_MS 100

/* Function prototypes and explanation -------------------------------------------------*/
/**
 * @brief Configure the HW specifications of a given button.
 * 
 * @param button_id Button ID. This index is used to select the element of the `buttons_arr[]` array.
 */
void 	port_button_init(uint32_t button_id);

/**
 * @brief Return the status of the button (pressed or not).
 * 
 * @param button_id Button ID. This index is used to get the correct button status structure.
 * @return true If the button has been pressed.
 * @return false If the button has not been pressed.
 */
bool 	port_button_get_pressed (uint32_t button_id);

/**
 * @brief Get the value of the GPIO connected to the button.
 * 
 * @param button_id Button ID. This index is used to select the element of the `buttons_arr[]` array.
 * @return true 
 * @return false 
 */
bool 	port_button_get_value (uint32_t button_id);

/**
 * @brief Set the status of the button (pressed or not).
 * 
 * This function is used to force the status of the button.
 * 
 * @param button_id Button ID. This index is used to get the correct button structure of the `buttons_arr[]` array.
 * @param pressed Status of the button.
 */
void 	port_button_set_pressed (uint32_t button_id, bool pressed);

/**
 * @brief Get the status of the interrupt line connected to the button.
 * 
 * This function is used to check if the interrupt line of the button is pending. It is called from the ISR to check if the button has been pressed.
 * 
 * @param button_id Button ID. This index is used to get the correct button structure of the `buttons_arr[]` array.
 * @return true 
 * @return false 
 */
bool 	port_button_get_pending_interrupt (uint32_t button_id);

/**
 * @brief Clear the pending interrupt of the button.
 * 
 * This function is used to clear the pending interrupt of the button. It is called from the ISR to avoid unwanted interrupts.
 * 
 * @param button_id Button ID. This index is used to get the correct button structure of the `buttons_arr[]` array.
 */
void 	port_button_clear_pending_interrupt (uint32_t button_id);

/**
 * @brief Disable the interrupts of the button.
 * 
 *This function is used to disable the interrupts of the button. It is used in the unit tests to avoid unwanted interrupts.
 * 
 * @param button_id Button ID. This index is used to get the correct button structure of the `buttons_arr[]` array.
 */
void 	port_button_disable_interrupts (uint32_t button_id);

#endif