/**
 * @file fsm_display.h
 * @brief Header for fsm_display.c file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-04-09
 */

#ifndef FSM_DISPLAY_SYSTEM_H_
#define FSM_DISPLAY_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

#include "fsm.h"

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
/**
 * @brief Minimum distance in cm to show the DANGER status constantly red.
 *
 */
#define HIGH_DANGER_MIN_CM 0

/**
 * @brief Minimum distance in cm to show the DANGER status applying linear interpolation.
 *
 */
#define DANGER_MIN_CM 5

/**
 * @brief Minimum distance in cm to show the WARNING status.
 *
 */
#define WARNING_MIN_CM 25

/**
 * @brief Minimum distance in cm to show the NO_PROBLEM status.
 *
 */
#define NO_PROBLEM_MIN_CM 50

/**
 * @brief Minimum distance in cm to show the INFO status.
 *
 */
#define INFO_MIN_CM 150

/**
 * @brief Minimum distance in cm to show the OK status.
 *
 */
#define OK_MIN_CM 175

/**
 * @brief Maximum distance in cm to show the OK status.
 *
 */
#define OK_MAX_CM 200

/* Enums */
/**
 * @brief Enumerator for the display system finite state machine.
 *
 * This enumerator defines the different states that the display system finite state machine can be in. Each state represents a specific condition of the display system: waiting for a status of display, or showing the status of the display.
 *
 */
enum FSM_DISPLAY_SYSTEM
{
    WAIT_DISPLAY = 0, /**< Starting state. Also comes here when there is no status of display, i.e. the display system is inactive. */
    SET_DISPLAY       /**< State to show the status of the display system. */
};

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure of the Display FSM.
 *
 */
typedef struct fsm_display_t fsm_display_t;

/* Function prototypes and explanation -------------------------------------------------*/
/**
 * @brief Create a new display FSM.
 *
 * This function creates a new display FSM with the given display ID.
 *
 * @param display_id 	Display ID. Must be unique.
 * @return fsm_display_t*  Pointer to the display FSM.
 */
fsm_display_t *fsm_display_new(uint32_t display_id);

/**
 * @brief Destroy a display FSM.
 *
 * This function destroys a display FSM and frees the memory.
 *
 * @param p_fsm Pointer to an `fsm_display_t` struct.
 */
void fsm_display_destroy(fsm_display_t *p_fsm);

/**
 * @brief Set the display system to show the distance in cm.
 *
 * This function is used to set the display system to show the distance in cm.
 *
 * @param p_fsm 	Pointer to an `fsm_display_t` struct.
 * @param distance_cm Distance in cm to show in the display system.
 */
void fsm_display_set_distance(fsm_display_t *p_fsm, uint32_t distance_cm);

/**
 * @brief Fire the display FSM.
 *
 * This function is used to fire the display FSM. It is used to check the transitions and execute the actions of the display FSM.
 *
 * @param p_fsm Pointer to an `fsm_display_t` struct.
 */
void fsm_display_fire(fsm_display_t *p_fsm);

/**
 * @brief Get the status of the display FSM.
 *
 * This function returns the status of the display system. This function might be used for testing and debugging purposes.
 *
 * @param p_fsm Pointer to an `fsm_display_t` struct.
 * @return true If the display system has been indicated to be active.
 * @return false If the display system has been indicated to be paused.
 */
bool fsm_display_get_status(fsm_display_t *p_fsm);

/**
 * @brief Set the status of the display FSM.
 *
 * This function is used to set the status of the display system. Indicating if the display system is active or paused.
 *
 * @param p_fsm Pointer to an `fsm_display_t` struct.
 * @param pause 	Status of the display system. `true` if the display system is paused, `false` if the display system is active.
 */
void fsm_display_set_status(fsm_display_t *p_fsm, bool pause);

/**
 * @brief Check if the display system is active.
 *
 * This function checks if the display system is active.
 *
 * @param p_fsm 	Pointer to an `fsm_display_t` struct.
 * @return true If the display system is active.
 * @return false If the display system is inactive.
 */
bool fsm_display_check_activity(fsm_display_t *p_fsm);

/**
 * @brief Get the inner FSM of the display.
 *
 * This function returns the inner FSM of the display.
 *
 * @param p_fsm 	Pointer to an `fsm_display_t` struct.
 * @return fsm_t* Pointer to the inner FSM.
 */
fsm_t *fsm_display_get_inner_fsm(fsm_display_t *p_fsm);

/**
 * @brief Get the state of the display FSM.
 *
 * This function returns the current state of the display FSM.
 *
 * @param p_fsm Pointer to an `fsm_display_t` struct.
 * @return uint32_t Current state of the display FSM.
 */
uint32_t fsm_display_get_state(fsm_display_t *p_fsm);

/**
 * @brief Set the state of the display FSM.
 *
 * This function sets the current state of the display FSM.
 *
 * @param p_fsm Pointer to an `fsm_display_t` struct.
 * @param state New state of the display FSM.
 */
void fsm_display_set_state(fsm_display_t *p_fsm, int8_t state);

#endif /* FSM_DISPLAY_SYSTEM_H_ */