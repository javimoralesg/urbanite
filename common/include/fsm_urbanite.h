/**
 * @file fsm_urbanite.h
 * @brief Header for fsm_urbanite.c file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-04-23
 */

#ifndef FSM_URBANITE_H_
#define FSM_URBANITE_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>

/* Other includes */
#include "fsm_button.h"
#include "fsm_ultrasound.h"
#include "fsm_display.h"
#include "fsm_buzzer.h"

/* Defines and enums ----------------------------------------------------------*/
/**
 * @brief Enumerator for the Urbanite finite state machine.
 *
 * This enumerator defines the different states that the Urbanite finite state machine can be in. Each state represents a specific condition or step in the process of showing the distance to the nearest obstacle.
 *
 */
enum FSM_URBANITE
{
    OFF = 0,                /**< Starting state. Also comes here when the button has been pressed for the required time to turn OFF the Urbanite */
    MEASURE_FRONT,          /**< State to measure the distance to the obstacles with the front ultrasound*/
    MEASURE_REAR,           /**< State to measure the distance to the obstacles with the rear ultrasound*/
    SLEEP_WHILE_OFF,        /**< State to start the low power mode while the Urbanite is OFF */
    SLEEP_WHILE_ON_FRONT,   /**< State to start the low power mode while the Urbanite is ON */
    SLEEP_WHILE_ON_REAR     /**< State to start the low power mode while the Urbanite is ON */
};

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief 	Structure that contains the information of the Urbanite FSM.
 *
 */
typedef struct fsm_urbanite_t fsm_urbanite_t;

/* Function prototypes and explanation -------------------------------------------------*/
/**
 * @brief Create a new Urbanite FSM.
 * 
 * This function creates a new Urbanite FSM with the given button, ultrasound, display FSMs and the required times for configuration.
 * 
 * @param p_fsm_button Pointer to the button FSM to interact with the Urbanite.
 * @param on_off_press_time_ms      Button press time in milliseconds to turn on or off the system.
 * @param change_press_time_ms      Time in milliseconds to change the current gear (front or rear).
 * @param pause_display_time_ms Time in ms to pause the display system.
 * @param p_fsm_ultrasound_front Pointer to the front ultrasound FSM.
 * @param p_fsm_display_front Pointer to the front display FSM.
 * @param p_fsm_ultrasound_rear Pointer to the rear ultrasound FSM.
 * @param p_fsm_display_rear Pointer to the rear display FSM.
 * @param p_fsm_buzzer Pointer to the buzzer.
 * @return fsm_urbanite_t*  Pointer to the Urbanite FSM.
 */
fsm_urbanite_t *fsm_urbanite_new(fsm_button_t *p_fsm_button, uint32_t on_off_press_time_ms, uint32_t change_press_time_ms, uint32_t pause_display_time_ms, fsm_ultrasound_t *p_fsm_ultrasound_front, fsm_display_t *p_fsm_display_front, fsm_ultrasound_t *p_fsm_ultrasound_rear, fsm_display_t *p_fsm_display_rear, fsm_buzzer_t *p_fsm_buzzer);

/**
 * @brief Fire the Urbanite FSM.
 * 
 * This function is used to check the transitions and execute the actions of the Urbanite FSM.
 * 
 * @param p_fsm Pointer to the `fsm_urbanite_t` struct.
 */
void fsm_urbanite_fire(fsm_urbanite_t *p_fsm);

/**
 * @brief Destroy an Urbanite FSM.
 * 
 * This function destroys an Urbanite FSM and frees the memory.
 * 
 * @param p_fsm Pointer to an `fsm_urbanite_t` struct.
 */
void fsm_urbanite_destroy(fsm_urbanite_t *p_fsm);

#endif /* FSM_URBANITE_H_ */