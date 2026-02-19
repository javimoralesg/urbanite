/**
 * @file fsm_button.c
 * @brief Button FSM main file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-03-26
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>

/* HW dependent includes */
#include "port_button.h"
#include "port_system.h"

/* Project includes */
#include "fsm.h"
#include "fsm_button.h"

/* Structs --------------------------------------------------------------------*/
/**
 * @brief Structure of the Button FSM.
 * 
 */
struct fsm_button_t
{
    /**
     * @brief Button FSM.
     * 
     */
    fsm_t f;

    /**
     * @brief Button debounce time in ms.
     * 
     */
    uint32_t debounce_time_ms;

    /**
     * @brief Next timeout for the anti-debounce in ms.
     * 
     */
    uint32_t next_timeout;

    /**
     * @brief Number of ticks when the button was pressed.
     * 
     */
    uint32_t tick_pressed;

    /**
     * @brief How much time the button has been pressed.
     * 
     */
    uint32_t duration;

    /**
     * @brief Button ID. Must be unique.
     * 
     */
    uint32_t button_id;
};

/* State machine input or transition functions */

/* State machine output or action functions */
/**
 * @brief Check if the button has been pressed.
 * 
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_button_t`.

 * @return true 
 * @return false 
 */
static bool check_button_pressed(fsm_t *p_this)
{
    fsm_button_t *p_fsm = (fsm_button_t *)(p_this);
    return port_button_get_pressed(p_fsm->button_id);
}

/**
 * @brief Check if the button has been released.
 * 
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_button_t`.
 * @return true 
 * @return false 
 */
static bool check_button_released(fsm_t *p_this)
{
    fsm_button_t *p_fsm = (fsm_button_t *)(p_this);
    return !port_button_get_pressed(p_fsm->button_id);
}

/**
 * @brief Check if the debounce-time has passed.
 * 
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_button_t`.
 * @return true 
 * @return false 
 */
static bool check_timeout(fsm_t *p_this)
{
    fsm_button_t *p_fsm = (fsm_button_t *)(p_this);
    return port_system_get_millis() > p_fsm->next_timeout;
}

/**
 * @brief Store the system tick when the button was pressed.
 * 
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_button_t`.
 */
static void do_store_tick_pressed(fsm_t *p_this)
{
    fsm_button_t *p_fsm = (fsm_button_t *)(p_this);
    uint32_t tiempo = port_system_get_millis();
    p_fsm->tick_pressed = tiempo;
    p_fsm->next_timeout = tiempo + p_fsm->debounce_time_ms;
}

/**
 * @brief Store the duration of the button press.
 * 
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_button_t`.
 */
static void do_set_duration(fsm_t *p_this)
{
    fsm_button_t *p_fsm = (fsm_button_t *)(p_this);
    uint32_t tiempo = port_system_get_millis();
    p_fsm->duration = tiempo - p_fsm->tick_pressed;
    p_fsm->next_timeout = tiempo + p_fsm->debounce_time_ms;
}

/**
 * @brief Array representing the transitions table of the FSM button.
 * 
 * @image html fsm_v1.png
 * 
 */
static fsm_trans_t fsm_trans_button[] = {
    {BUTTON_RELEASED, check_button_pressed, BUTTON_PRESSED_WAIT, do_store_tick_pressed},
    {BUTTON_PRESSED_WAIT, check_timeout, BUTTON_PRESSED, NULL},
    {BUTTON_PRESSED, check_button_released, BUTTON_RELEASED_WAIT, do_set_duration},
    {BUTTON_RELEASED_WAIT, check_timeout, BUTTON_RELEASED, NULL},
    {-1, NULL, -1, NULL}

};

/* Other auxiliary functions */

/**
 * @brief Initialize a button FSM.
 * 
 * This function initializes the default values of the FSM struct and calls to the `port` to initialize the associated HW given the ID.
 * 
 * This FSM implements an anti-debounce mechanism. Debounces (or very fast button presses) lasting less than the `debounce_time_ms` are filtered out.
 * 
 * The FSM stores the duration of the last button press. The user should ask for it using the function `fsm_button_get_duration()`.
 * 
 * At start and reset, the duration value must be 0 ms. A value of 0 ms means that there has not been a new button press.
 * 
 * @attention The user is required to reset the duration value once it has been read. Otherwise, this value may be misinterpreted by the user, if successive calls are made without having pressed the button. In such a case we would be reading past information. In order to reset the value, the function `fsm_button_reset_duration()` must be called.
 * 
 * In other words, the status flag of this FSM is the variable duration. A duration of 0 means that no new button has been pressed, a value other than 0 means that it has been pressed and the value is its duration, so it is the user's responsibility to clear this status flag.
 * 
 * The FSM contains information of the button ID. This ID is a unique identifier that is managed by the user in the `port`. That is where the user provides identifiers and HW information for all the buttons on his system. The FSM does not have to know anything of the underlying HW.
 * 
 * @param p_fsm_button Pointer to the button FSM.
 * @param debounce_time Anti-debounce time in milliseconds.
 * @param button_id Unique button identifier number.
 */
void fsm_button_init(fsm_button_t *p_fsm_button, uint32_t debounce_time, uint32_t button_id)
{
    /* TODO alumnos: */
    fsm_init(&p_fsm_button->f, fsm_trans_button);

    p_fsm_button->debounce_time_ms = debounce_time;
    p_fsm_button->button_id = button_id;
    p_fsm_button->tick_pressed = 0;
    p_fsm_button->duration = 0;
    
    port_button_init(button_id);
}

/* Public functions -----------------------------------------------------------*/

fsm_button_t *fsm_button_new(uint32_t debounce_time, uint32_t button_id)
{
    fsm_button_t *p_fsm_button = malloc(sizeof(fsm_button_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_button_init(p_fsm_button, debounce_time, button_id);   /* Initialize the FSM */
    return p_fsm_button;                                       /* Composite pattern: return the fsm_t pointer as a fsm_button_t pointer */
}

/* FSM-interface functions. These functions are used to interact with the FSM */

void fsm_button_fire(fsm_button_t *p_fsm)
{
    fsm_fire(&p_fsm->f); // Is it also possible to it in this way: fsm_fire((fsm_t *)p_fsm);
}

void fsm_button_destroy(fsm_button_t *p_fsm)
{
    free(&p_fsm->f);
}

fsm_t *fsm_button_get_inner_fsm(fsm_button_t *p_fsm)
{
    return &p_fsm->f;
}

uint32_t fsm_button_get_state(fsm_button_t *p_fsm)
{
    return p_fsm->f.current_state;
}

uint32_t fsm_button_get_duration(fsm_button_t *p_fsm)
{
    return p_fsm->duration;
}

void fsm_button_reset_duration(fsm_button_t *p_fsm)
{
    p_fsm->duration = 0;
}


uint32_t fsm_button_get_debounce_time_ms(fsm_button_t *p_fsm)
{
    return p_fsm->debounce_time_ms;
}

bool fsm_button_check_activity (fsm_button_t *p_fsm)
{
    return (p_fsm->f.current_state != 0);
}

