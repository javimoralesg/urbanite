/**
 * @file fsm_buzzer.h
 * @brief Header for fsm_buzzer.c file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-05-21
 */

#ifndef FSM_BUZZER_SYSTEM_H_
#define FSM_BUZZER_SYSTEM_H_

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
 * @brief Maximum distance in cm to show the OK status.
 *
 */
#define OK_MAX_CM 200

/* Enums */
/**
 * @brief Enumerator for the buzzer system finite state machine.
 *
 * This enumerator defines the different states that the buzzer system finite state machine can be in. Each state represents a specific condition of the buzzer system: waiting for a status of buzzer, or showing the status of the buzzer.
 *
 */
enum FSM_BUZZER_SYSTEM
{
    WAIT_BUZZER = 0, /**< Starting state. Also comes here when there is no status of buzzer, i.e. the buzzer system is inactive. */
    SET_BUZZER       /**< State to show the status of the buzzer system. */
};

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure of the buzzer FSM.
 *
 */
typedef struct fsm_buzzer_t fsm_buzzer_t;

/* Function prototypes and explanation -------------------------------------------------*/
/**
 * @brief Create a new buzzer FSM.
 *
 * This function creates a new buzzer FSM with the given buzzer ID.
 *
 * @param buzzer_id 	Buzzer ID. Must be unique.
 * @return fsm_buzzer_t*  Pointer to the buzzer FSM.
 */
fsm_buzzer_t *fsm_buzzer_new(uint32_t buzzer_id);

/**
 * @brief Destroy a buzzer FSM.
 *
 * This function destroys a buzzer FSM and frees the memory.
 *
 * @param p_fsm Pointer to an `fsm_buzzer_t` struct.
 */
void fsm_buzzer_destroy(fsm_buzzer_t *p_fsm);

/**
 * @brief Set the buzzer system to show the distance in cm.
 *
 * This function is used to set the buzzer system to show the distance in cm.
 *
 * @param p_fsm 	Pointer to an `fsm_buzzer_t` struct.
 * @param distance_cm Distance in cm to show in the buzzer system.
 */
void fsm_buzzer_set_distance(fsm_buzzer_t *p_fsm, uint32_t distance_cm);

/**
 * @brief Fire the buzzer FSM.
 *
 * This function is used to fire the buzzer FSM. It is used to check the transitions and execute the actions of the buzzer FSM.
 *
 * @param p_fsm Pointer to an `fsm_buzzer_t` struct.
 */
void fsm_buzzer_fire(fsm_buzzer_t *p_fsm);

/**
 * @brief Get the status of the buzzer FSM.
 *
 * This function returns the status of the buzzer system. This function might be used for testing and debugging purposes.
 *
 * @param p_fsm Pointer to an `fsm_buzzer_t` struct.
 * @return true If the buzzer system has been indicated to be active.
 * @return false If the buzzer system has been indicated to be paused.
 */
bool fsm_buzzer_get_status(fsm_buzzer_t *p_fsm);

/**
 * @brief Set the status of the buzzer FSM.
 *
 * This function is used to set the status of the buzzer system. Indicating if the buzzer system is active or paused.
 *
 * @param p_fsm Pointer to an `fsm_buzzer_t` struct.
 * @param pause 	Status of the buzzer system. `true` if the buzzer system is paused, `false` if the buzzer system is active.
 */
void fsm_buzzer_set_status(fsm_buzzer_t *p_fsm, bool pause);

/**
 * @brief Check if the buzzer system is active.
 *
 * This function checks if the buzzer system is active.
 *
 * @param p_fsm 	Pointer to an `fsm_buzzer_t` struct.
 * @return true If the buzzer system is active.
 * @return false If the buzzer system is inactive.
 */
bool fsm_buzzer_check_activity(fsm_buzzer_t *p_fsm);

/**
 * @brief Get the inner FSM of the buzzer.
 *
 * This function returns the inner FSM of the buzzer.
 *
 * @param p_fsm 	Pointer to an `fsm_buzzer_t` struct.
 * @return fsm_t* Pointer to the inner FSM.
 */
fsm_t *fsm_buzzer_get_inner_fsm(fsm_buzzer_t *p_fsm);

/**
 * @brief Get the state of the buzzer FSM.
 *
 * This function returns the current state of the buzzer FSM.
 *
 * @param p_fsm Pointer to an `fsm_buzzer_t` struct.
 * @return uint32_t Current state of the buzzer FSM.
 */
uint32_t fsm_buzzer_get_state(fsm_buzzer_t *p_fsm);

/**
 * @brief Set the state of the buzzer FSM.
 *
 * This function sets the current state of the buzzer FSM.
 *
 * @param p_fsm Pointer to an `fsm_buzzer_t` struct.
 * @param state New state of the buzzer FSM.
 */
void fsm_buzzer_set_state(fsm_buzzer_t *p_fsm, int8_t state);

#endif /* FSM_BUZZER_SYSTEM_H_ */