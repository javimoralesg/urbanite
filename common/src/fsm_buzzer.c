/**
 * @file fsm_buzzer.c
 * @brief Buzzer system FSM main file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-05-21
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <stdio.h>

/* HW dependent includes */
#include "port_buzzer.h"
#include "port_system.h"

/* Project includes */
#include "fsm.h"
#include "fsm_buzzer.h"

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure of the Buzzer FSM.
 *
 */
struct fsm_buzzer_t
{
    /**
     * @brief Buzzer system FSM.
     *
     */
    fsm_t f;

    /**
     * @brief Distance in cm to the object.
     *
     */
    int32_t distance_cm;

    /**
     * @brief Flag to indicate if a new sound has to be set.
     *
     */
    bool new_sound;

    /**
     * @brief Flag to indicate if the buzzer is active.
     *
     */
    bool status;

    /**
     * @brief Flag to indicate if the buzzer being active is idle, or not.
     *
     */
    bool idle;

    /**
     * @brief Unique buzzer system identifier number.
     *
     */
    uint32_t buzzer_id;
};

/* Private functions -----------------------------------------------------------*/
/**
 * @brief Set sound levels of the buzzer according to the distance.
 *
 * This function sets the levels of an buzzer according to the distance measured by the ultrasound sensor. This buzzer structure is later passed to the `port_buzzer_set_sound()` function to set the sound of the buzzer.
 *
 * @param distance_cm       Distance measured by the ultrasound sensor in centimeters.
 * @return sound            Level of frequency of the sound.
 */
uint8_t _compute_buzzer_levels(int32_t distance_cm)
{
    uint8_t sound = 0;
    if ((distance_cm >= HIGH_DANGER_MIN_CM) && (distance_cm <= OK_MAX_CM))
    {
        sound = PORT_BUZZER_MAX_VALUE * (OK_MAX_CM - distance_cm) / (OK_MAX_CM);
    }
    else
    {
        sound = PORT_BUZZER_MIN_VALUE;
    }
    return sound;
}

/* State machine input or transition functions */
/**
 * @brief Check if a new sound has to be set.
 *
 * @param p_this Pointer to an `fsm_t` struct than contains an `fsm_buzzer_t`.
 * @return true If a new sound has to be set.
 * @return false If a new sound does not have to be set.
 */
static bool check_set_new_sound(fsm_t *p_this)
{
    fsm_buzzer_t *p_fsm = (fsm_buzzer_t *)(p_this);
    return p_fsm->new_sound;
}

/**
 * @brief Check if the buzzer is set to be active (ON), independently if it is idle or not.
 *
 * @param p_this Pointer to an `fsm_t` struct than contains an `fsm_buzzer_t`.
 * @return true If the buzzer system has been indicated to be active independently if it is idle or not.
 * @return false If the buzzer system has been indicated to be inactive.
 */
static bool check_active(fsm_t *p_this)
{
    fsm_buzzer_t *p_fsm = (fsm_buzzer_t *)(p_this);
    return p_fsm->status;
}

/**
 * @brief Check if the buzzer is set to be inactive (OFF).
 *
 * @param p_this Pointer to an `fsm_t` struct than contains an `fsm_buzzer_t`.
 * @return true If the buzzer system has been indicated to be inactive.
 * @return false If the buzzer system has been indicated to be active.
 */
static bool check_off(fsm_t *p_this)
{
    fsm_buzzer_t *p_fsm = (fsm_buzzer_t *)(p_this);
    return !p_fsm->status;
}

/* State machine output or action functions */
/**
 * @brief Turn the buzzer system ON for the first time.
 *
 * @param p_this Pointer to an `fsm_t` struct than contains an `fsm_buzzer_t`.
 */
static void do_set_on(fsm_t *p_this)
{
    fsm_buzzer_t *p_fsm = (fsm_buzzer_t *)(p_this);
    port_buzzer_set_sound(p_fsm->buzzer_id, PORT_BUZZER_MIN_VALUE);
}

/**
 * @brief Turn the buzzer system ON for the first time.
 *
 * @param p_this Pointer to an `fsm_t` struct than contains an `fsm_buzzer_t`.
 */
static void do_set_sound(fsm_t *p_this)
{
    fsm_buzzer_t *p_fsm = (fsm_buzzer_t *)(p_this);
    uint8_t sound = _compute_buzzer_levels(p_fsm->distance_cm);
    port_buzzer_set_sound(p_fsm->buzzer_id, sound);
    p_fsm->new_sound = false;
    p_fsm->idle = true;
}

/**
 * @brief Turn the buzzer system OFF.
 *
 * @param p_this Pointer to an `fsm_t` struct than contains an `fsm_buzzer_t`.
 */
static void do_set_off(fsm_t *p_this)
{
    fsm_buzzer_t *p_fsm = (fsm_buzzer_t *)(p_this);
    port_buzzer_set_sound(p_fsm->buzzer_id, PORT_BUZZER_MIN_VALUE);
    p_fsm->idle = false;
}

/* State machine */
/**
 * @brief Array representing the transitions table of the FSM buzzer.
 *
 * @image html fsm_buzzer.png
 *
 */
static fsm_trans_t fsm_trans_buzzer[] = {
    {WAIT_BUZZER, check_active, SET_BUZZER, do_set_on},
    {SET_BUZZER, check_set_new_sound, SET_BUZZER, do_set_sound},
    {SET_BUZZER, check_off, WAIT_BUZZER, do_set_off},
    {-1, NULL, -1, NULL}};

/* Other auxiliary functions */
/**
 * @brief Initialize a buzzer system FSM.
 *
 * This function initializes the default values of the FSM struct and calls to the `port` to initialize the associated HW given the ID.
 *
 * The FSM stores the buzzer level of the buzzer system. The user should set it using the function `fsm_buzzer_set_distance()`.
 *
 * @attention This buzzer system is agnostic to the ultrasound sensor or any other sensor. It only shows the status of the buzzer system set by the user. It does not matter if the buzzer is for a parking sensor, a door sensor, or any other sensor. The buzzer system only shows a status according to a distance set by the user.
 *
 * The FSM contains information of the buzzer ID. This ID is a unique identifier that is managed by the user in the `port`. That is where the user provides identifiers and HW information for all the buzzers on his system. The FSM does not have to know anything of the underlying HW.
 *
 * @param p_fsm_buzzer Pointer to the buzzer FSM.
 * @param buzzer_id Unique buzzer identifier number.
 */
static void fsm_buzzer_init(fsm_buzzer_t *p_fsm_buzzer, uint32_t buzzer_id)
{
    fsm_init(&p_fsm_buzzer->f, fsm_trans_buzzer);
    p_fsm_buzzer->buzzer_id = buzzer_id;
    p_fsm_buzzer->distance_cm = -1;
    p_fsm_buzzer->new_sound = false;
    p_fsm_buzzer->status = false;
    p_fsm_buzzer->idle = false;
    port_buzzer_init(buzzer_id);
}

/* Public functions -----------------------------------------------------------*/
fsm_buzzer_t *fsm_buzzer_new(uint32_t buzzer_id)
{
    fsm_buzzer_t *p_fsm_buzzer = malloc(sizeof(fsm_buzzer_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_buzzer_init(p_fsm_buzzer, buzzer_id);                  /* Initialize the FSM */
    return p_fsm_buzzer;
}

void fsm_buzzer_destroy(fsm_buzzer_t *p_fsm)
{
    free(&p_fsm->f);
}

void fsm_buzzer_fire(fsm_buzzer_t *p_fsm)
{
    fsm_fire(&p_fsm->f); // Is it also possible to it in this way: fsm_fire((fsm_t *)p_fsm);
}

void fsm_buzzer_set_distance(fsm_buzzer_t *p_fsm, uint32_t distance_cm)
{
    p_fsm->distance_cm = distance_cm;
    p_fsm->new_sound = true;
}

bool fsm_buzzer_get_status(fsm_buzzer_t *p_fsm)
{
    return p_fsm->status;
}

void fsm_buzzer_set_status(fsm_buzzer_t *p_fsm, bool status)
{
    p_fsm->status = status;
}

bool fsm_buzzer_check_activity(fsm_buzzer_t *p_fsm)
{
    return (p_fsm->status && !p_fsm->idle);
}

fsm_t *fsm_buzzer_get_inner_fsm(fsm_buzzer_t *p_fsm)
{
    return &p_fsm->f;
}

uint32_t fsm_buzzer_get_state(fsm_buzzer_t *p_fsm)
{
    return p_fsm->f.current_state;
}

void fsm_buzzer_set_state(fsm_buzzer_t *p_fsm, int8_t state)
{
    p_fsm->f.current_state = state;
}