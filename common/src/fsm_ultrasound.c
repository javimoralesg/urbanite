/**
 * @file fsm_ultrasound.c
 * @brief Ultrasound sensor FSM main file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-03-26
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* HW dependent includes */
#include "port_ultrasound.h"
#include "port_system.h"

/* Project includes */
#include "fsm.h"
#include "fsm_ultrasound.h"

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure to define the Ultrasound FSM.
 *
 */
struct fsm_ultrasound_t
{
    /**
     * @brief Ultrasound FSM.
     *
     */
    fsm_t f;

    /**
     * @brief How much time the ultrasound has been pressed.
     *
     */
    uint32_t distance_cm;

    /**
     * @brief Indicate if the ultrasound sensor is active or not.
     *
     */
    bool status;

    /**
     * @brief Flag to indicate if a new measurement has been completed.
     *
     */
    bool new_measurement;

    /**
     * @brief Ultrasound ID. Must be unique.
     *
     */
    uint32_t ultrasound_id;

    /**
     * @brief Array to store the last distance measurements.
     *
     */
    uint32_t distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS];

    /**
     * @brief Index to store the last distance measurement.
     *
     */
    uint32_t distance_idx;

};

/* Private functions -----------------------------------------------------------*/
// Comparison function for qsort
/**
 * @brief Compare function to sort arrays.
 *
 * This function is used to compare two elements. It will be used for the `qsort()` function to sort the array of distances.
 *
 * @param a Pointer to the first element to compare.
 * @param b Pointer to the second element to compare.
 * @return int Result of the comparison.
 */
int _compare(const void *a, const void *b)
{
    return (*(uint32_t *)a - *(uint32_t *)b);
}

/* State machine input or transition functions */
/**
 * @brief Check if the ultrasound sensor is active and ready to start a new measurement.
 *
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_ultrasound_t`.

 * @return true
 * @return false
 */
static bool check_on(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)(p_this);
    return port_ultrasound_get_trigger_ready(p_fsm->ultrasound_id) && (p_fsm->status);
}

/**
 * @brief Check if the ultrasound sensor has been set to be inactive (OFF).
 *
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_ultrasound_t`.
 * @return true
 * @return false
 */
static bool check_off(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)(p_this);
    return !(p_fsm->status);
}

/**
 * @brief Check if the ultrasound sensor has finished the trigger signal.
 *
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_ultrasound_t`.
 * @return true
 * @return false
 */
static bool check_trigger_end(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)(p_this);
    return port_ultrasound_get_trigger_end(p_fsm->ultrasound_id);
}

/**
 * @brief Check if the ultrasound sensor has received the init (rising edge in the input capture) of the echo signal.
 *
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_ultrasound_t`.
 * @return true
 * @return false
 */
static bool check_echo_init(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)(p_this);
    if (port_ultrasound_get_echo_init_tick(p_fsm->ultrasound_id) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Check if the ultrasound sensor has received the end (falling edge in the input capture) of the echo signal.
 *
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_ultrasound_t`.
 * @return true
 * @return false
 */
static bool check_echo_received(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)(p_this);
    return port_ultrasound_get_echo_received(p_fsm->ultrasound_id);
}

/**
 * @brief Check if a new measurement is ready.
 *
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_ultrasound_t`.
 * @return true
 * @return false
 */
static bool check_new_measurement(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)(p_this);
    return port_ultrasound_get_trigger_ready(p_fsm->ultrasound_id);
}


/**
 * @brief Start a measurement of the ultrasound transceiver for the first time after the FSM is started.
 *
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_ultrasound_t`.
 */
static void do_start_measurement(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)(p_this);
    port_ultrasound_start_measurement(p_fsm->ultrasound_id);
}

/**
 * @brief Stop the trigger signal of the ultrasound sensor.
 *
 * This function is called when the time to trigger the ultrasound sensor has finished. It stops the trigger signal and the trigger timer, and clears the `trigger_end` flag to indicate that the trigger signal has already been lowered.
 *
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_ultrasound_t`.
 */
static void do_stop_trigger(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)(p_this);
    port_ultrasound_stop_trigger_timer(p_fsm->ultrasound_id);
    port_ultrasound_set_trigger_end(p_fsm->ultrasound_id, false);
}

/**
 * @brief Set the distance measured by the ultrasound sensor.
 *
 * This function is called when the ultrasound sensor has received the echo signal. It calculates the distance in cm and stores it in the array of distances.
 *
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_ultrasound_t`.
 */
static void do_set_distance(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)(p_this);
    uint32_t echo_overflows = port_ultrasound_get_echo_overflows(p_fsm->ultrasound_id);
    uint32_t echo_init_tick = port_ultrasound_get_echo_init_tick(p_fsm->ultrasound_id);
    uint32_t echo_end_tick = port_ultrasound_get_echo_end_tick(p_fsm->ultrasound_id);
    uint32_t time = (echo_end_tick + echo_overflows * 65536 - echo_init_tick);
    uint32_t distance = time * SPEED_OF_SOUND_MS / 20000;
    p_fsm->distance_arr[p_fsm->distance_idx] = distance;
    if (p_fsm->distance_idx >= FSM_ULTRASOUND_NUM_MEASUREMENTS - 1)
    {
        qsort(p_fsm->distance_arr, FSM_ULTRASOUND_NUM_MEASUREMENTS, sizeof(uint32_t), _compare);
        p_fsm->distance_cm = p_fsm->distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS / 2]; // Esta es la mediana porque hay un numero IMPAR de elementos
        p_fsm->new_measurement = true;
    }
    p_fsm->distance_idx += 1;
    if (p_fsm->distance_idx >= FSM_ULTRASOUND_NUM_MEASUREMENTS)
        p_fsm->distance_idx = 0;
    port_ultrasound_stop_echo_timer(p_fsm->ultrasound_id);
    port_ultrasound_reset_echo_ticks(p_fsm->ultrasound_id);
}

/**
 * @brief Stop the ultrasound sensor.
 *
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_ultrasound_t`.
 */
static void do_stop_measurement(fsm_t *p_this)
{
    fsm_ultrasound_t *p_fsm = (fsm_ultrasound_t *)(p_this);
    port_ultrasound_stop_ultrasound(p_fsm->ultrasound_id);
}

/**
 * @brief Start a new measurement of the ultrasound transceiver.
 *
 * This function is called when the ultrasound sensor has finished a measurement and is ready to start a new one.
 *
 * @param p_this Pointer to an `fsm_t` structure that contains an `fsm_ultrasound_t`.
 */
static void do_start_new_measurement(fsm_t *p_this)
{
    do_start_measurement(p_this);
}

/* State machine output or action functions */
/**
 * @brief Array representing the transitions table of the FSM ultrasound.
 *
 * @attention The order of the transitions is important. The FSM will check the transitions in the order they are defined in this array. In state `SET_DISTANCE`, the FSM will first check if a new measurement is ready (`check_new_measurement()`), then if the ultrasound sensor is off (`check_off()`). If the order is changed, the FSM may not work as expected and may finish measurements before time.
 *
 * @image html fsm_v2.png
 *
 */
static fsm_trans_t fsm_trans_ultrasound[] = {
    {WAIT_START, check_on, TRIGGER_START, do_start_measurement},
    {TRIGGER_START, check_trigger_end, WAIT_ECHO_START, do_stop_trigger},

    {WAIT_ECHO_START, check_echo_init, WAIT_ECHO_END, NULL},

    {WAIT_ECHO_END, check_echo_received, SET_DISTANCE, do_set_distance},

    {SET_DISTANCE, check_new_measurement, TRIGGER_START, do_start_new_measurement},
    {SET_DISTANCE, check_off, WAIT_START, do_stop_measurement},
    {-1, NULL, -1, NULL}

};

/* Other auxiliary functions */
/**
 * @brief Initialize a ultrasound FSM.
 *
 * This function initializes the default values of the FSM struct and calls to the `port` to initialize the associated HW given the ID.
 *
 * The FSM stores the distance of the last ultrasound trigger. The user should ask for it using the function `fsm_ultrasound_get_distance()`.
 *
 * The FSM contains information of the ultrasound ID. This ID is a unique identifier that is managed by the user in the `port`. That is where the user provides identifiers and HW information for all the ultrasounds on his system. The FSM does not have to know anything of the underlying HW.
 *
 * @note The main program and the `port` are the only ones that manage and stablish the ID. The ultrasound FSM acts as a man-in-the-middle but this library does not handles the ultrasound IDs.
 *
 * @param p_fsm_ultrasound Pointer to the ultrasound FSM.
 * @param ultrasound_id Unique ultrasound identifier number.

 */
void fsm_ultrasound_init(fsm_ultrasound_t *p_fsm_ultrasound, uint32_t ultrasound_id)
{
    // Initialize the FSM
    fsm_init(&p_fsm_ultrasound->f, fsm_trans_ultrasound);

    p_fsm_ultrasound->distance_cm = 0;
    p_fsm_ultrasound->distance_idx = 0;
    memset(p_fsm_ultrasound->distance_arr, 0, sizeof(p_fsm_ultrasound->distance_arr));
    p_fsm_ultrasound->new_measurement = false;
    p_fsm_ultrasound->status = false;
    p_fsm_ultrasound->ultrasound_id = ultrasound_id;

    port_ultrasound_init(ultrasound_id);
}

/* Public functions -----------------------------------------------------------*/

fsm_ultrasound_t *fsm_ultrasound_new(uint32_t ultrasound_id)
{
    fsm_ultrasound_t *p_fsm_ultrasound = malloc(sizeof(fsm_ultrasound_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_ultrasound_init(p_fsm_ultrasound, ultrasound_id);                  /* Initialize the FSM */
    return p_fsm_ultrasound;
}

// Other auxiliary functions

void fsm_ultrasound_set_state(fsm_ultrasound_t *p_fsm, int8_t state)
{
    p_fsm->f.current_state = state;
}

/* FSM-interface functions. These functions are used to interact with the FSM */

void fsm_ultrasound_fire(fsm_ultrasound_t *p_fsm)
{
    fsm_fire(&p_fsm->f); // It is also possible to it in this way: fsm_fire((fsm_t *)p_fsm);
}

void fsm_ultrasound_destroy(fsm_ultrasound_t *p_fsm)
{
    free(&p_fsm->f);
}

fsm_t *fsm_ultrasound_get_inner_fsm(fsm_ultrasound_t *p_fsm)
{
    return &p_fsm->f;
}

uint32_t fsm_ultrasound_get_state(fsm_ultrasound_t *p_fsm)
{
    return p_fsm->f.current_state;
}

uint32_t fsm_ultrasound_get_distance(fsm_ultrasound_t *p_fsm)
{
    p_fsm->new_measurement = false;
    return p_fsm->distance_cm;
}

void fsm_ultrasound_stop(fsm_ultrasound_t *p_fsm)
{
    p_fsm->status = false;
    port_ultrasound_stop_ultrasound(p_fsm->ultrasound_id);
}

void fsm_ultrasound_start(fsm_ultrasound_t *p_fsm)
{
    p_fsm->status = true;
    p_fsm->distance_idx = 0;
    p_fsm->distance_cm = 0;
    port_ultrasound_reset_echo_ticks(p_fsm->ultrasound_id);
    port_ultrasound_set_trigger_ready(p_fsm->ultrasound_id, true);
    port_ultrasound_start_new_measurement_timer(p_fsm->ultrasound_id);
}

bool fsm_ultrasound_get_status(fsm_ultrasound_t *p_fsm)
{
    return p_fsm->status;
}

void fsm_ultrasound_set_status(fsm_ultrasound_t *p_fsm, bool status)
{
    p_fsm->status = status;
}

bool fsm_ultrasound_get_ready(fsm_ultrasound_t *p_fsm)
{
    return port_ultrasound_get_trigger_ready(p_fsm->ultrasound_id);
}

bool fsm_ultrasound_get_new_measurement_ready(fsm_ultrasound_t *p_fsm)
{
    return p_fsm->new_measurement;
}

bool fsm_ultrasound_check_activity(fsm_ultrasound_t *p_fsm)
{
    return false;
}