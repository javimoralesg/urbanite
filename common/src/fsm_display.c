/**
 * @file fsm_display.c
 * @brief Display system FSM main file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-04-09
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <stdio.h>

/* HW dependent includes */
#include "port_display.h"
#include "port_system.h"

/* Project includes */
#include "fsm.h"
#include "fsm_display.h"

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure of the Display FSM.
 *
 */
struct fsm_display_t
{
    /**
     * @brief Display system FSM.
     *
     */
    fsm_t f;

    /**
     * @brief Distance in cm to the object.
     *
     */
    int32_t distance_cm;

    /**
     * @brief Flag to indicate if a new color has to be set.
     *
     */
    bool new_color;

    /**
     * @brief Flag to indicate if the display is active.
     *
     */
    bool status;

    /**
     * @brief Flag to indicate if the display being active is idle, or not.
     *
     */
    bool idle;

    /**
     * @brief Unique display system identifier number.
     *
     */
    uint32_t display_id;
};

/* Private functions -----------------------------------------------------------*/
/**
 * @brief Linearly interpolates between two RGB colors based on a distance value.
 * 
 * Computes a smooth transition between two colors (`color_inf`and `color_sup`) proportionally to the distance measured by the ultasound.
 * 
 * @param color_inf RGB color corresponding to the lower bound of the range.
 * @param color_sup RGB color corresponding to the upper bound of the range.
 * @param distance_inf Distance range of the lower bound.
 * @param distance_sup Distance range of the upper bound.
 * @param distance_cm Distance measured by the ultrasound sensor in centimeters.
 * @return rgb_color_t 
 */
rgb_color_t _linear_interp(rgb_color_t color_inf, rgb_color_t color_sup, int32_t distance_inf, int32_t distance_sup, int32_t distance_cm) 
{
    int32_t red = color_inf.r * (distance_sup - distance_cm)/(distance_sup - distance_inf) + color_sup.r * (distance_cm - distance_inf)/(distance_sup - distance_inf);
    int32_t green = color_inf.g * (distance_sup - distance_cm)/(distance_sup - distance_inf) + color_sup.g * (distance_cm - distance_inf)/(distance_sup - distance_inf);
    int32_t blue = color_inf.b * (distance_sup - distance_cm)/(distance_sup - distance_inf) + color_sup.b * (distance_cm - distance_inf)/(distance_sup - distance_inf);
    return (rgb_color_t){red, green, blue};
}

/**
 * @brief Set color levels of the RGB LEDs according to the distance.
 *
 * This function sets the levels of an RGB LED according to the distance measured by the ultrasound sensor. This RGB LED structure is later passed to the `port_display_set_rgb()` function to set the color of the RGB LED.
 *
 * @param p_color Pointer to an `rgb_color_t` struct that will store the levels of the RGB LED.
 * @param distance_cm Distance measured by the ultrasound sensor in centimeters.
 */
void _compute_display_levels(rgb_color_t *p_color, int32_t distance_cm)
{
    if ((distance_cm >= HIGH_DANGER_MIN_CM) && (distance_cm <= DANGER_MIN_CM))
    {
        *p_color = COLOR_RED;
    }
    else if ((distance_cm > DANGER_MIN_CM) && (distance_cm <= WARNING_MIN_CM))
    {
        *p_color = _linear_interp(COLOR_RED, COLOR_YELLOW, DANGER_MIN_CM, WARNING_MIN_CM, distance_cm);
    }
    else if ((distance_cm > WARNING_MIN_CM) && (distance_cm <= NO_PROBLEM_MIN_CM))
    {
        *p_color = _linear_interp(COLOR_YELLOW, COLOR_GREEN, WARNING_MIN_CM, NO_PROBLEM_MIN_CM, distance_cm);
    }
    else if ((distance_cm > NO_PROBLEM_MIN_CM) && (distance_cm <= INFO_MIN_CM))
    {
        *p_color = _linear_interp(COLOR_GREEN, COLOR_TURQUOISE, NO_PROBLEM_MIN_CM, INFO_MIN_CM, distance_cm);
    }
    else if ((distance_cm > INFO_MIN_CM) && (distance_cm <= OK_MIN_CM))
    {
        *p_color = _linear_interp(COLOR_TURQUOISE, COLOR_BLUE, INFO_MIN_CM, OK_MIN_CM, distance_cm);
    }
    else if ((distance_cm > OK_MIN_CM) && (distance_cm <= OK_MAX_CM))
    {
        *p_color = _linear_interp(COLOR_BLUE, COLOR_OFF, OK_MIN_CM, OK_MAX_CM, distance_cm);
    }
    else
    {
        *p_color = COLOR_OFF;
    }
}

/* State machine input or transition functions */
/**
 * @brief Check if a new color has to be set.
 *
 * @param p_this Pointer to an `fsm_t` struct than contains an `fsm_display_t`.
 * @return true If a new color has to be set.
 * @return false If a new color does not have to be set.
 */
static bool check_set_new_color(fsm_t *p_this)
{
    fsm_display_t *p_fsm = (fsm_display_t *)(p_this);
    return p_fsm->new_color;
}

/**
 * @brief Check if the display is set to be active (ON), independently if it is idle or not.
 *
 * @param p_this Pointer to an `fsm_t` struct than contains an `fsm_display_t`.
 * @return true If the display system has been indicated to be active independently if it is idle or not.
 * @return false If the display system has been indicated to be inactive.
 */
static bool check_active(fsm_t *p_this)
{
    fsm_display_t *p_fsm = (fsm_display_t *)(p_this);
    return p_fsm->status;
}

/**
 * @brief Check if the display is set to be inactive (OFF).
 *
 * @param p_this Pointer to an `fsm_t` struct than contains an `fsm_display_t`.
 * @return true If the display system has been indicated to be inactive.
 * @return false If the display system has been indicated to be active.
 */
static bool check_off(fsm_t *p_this)
{
    fsm_display_t *p_fsm = (fsm_display_t *)(p_this);
    return !p_fsm->status;
}

/* State machine output or action functions */
/**
 * @brief Turn the display system ON for the first time.
 *
 * @param p_this Pointer to an `fsm_t` struct than contains an `fsm_display_t`.
 */
static void do_set_on(fsm_t *p_this)
{
    fsm_display_t *p_fsm = (fsm_display_t *)(p_this);
    port_display_set_rgb(p_fsm->display_id, COLOR_OFF);
}

/**
 * @brief Turn the display system ON for the first time.
 *
 * @param p_this Pointer to an `fsm_t` struct than contains an `fsm_display_t`.
 */
static void do_set_color(fsm_t *p_this)
{
    fsm_display_t *p_fsm = (fsm_display_t *)(p_this);
    rgb_color_t p_color = COLOR_OFF;
    _compute_display_levels(&p_color, p_fsm->distance_cm);
    port_display_set_rgb(p_fsm->display_id, p_color);
    p_fsm->new_color = false;
    p_fsm->idle = true;
}

/**
 * @brief Turn the display system OFF.
 *
 * @param p_this Pointer to an `fsm_t` struct than contains an `fsm_display_t`.
 */
static void do_set_off(fsm_t *p_this)
{
    fsm_display_t *p_fsm = (fsm_display_t *)(p_this);
    port_display_set_rgb(p_fsm->display_id, COLOR_OFF);
    p_fsm->idle = false;
}

/* State machine */
/**
 * @brief Array representing the transitions table of the FSM display.
 *
 * @image html fsm_v3.png
 *
 */
static fsm_trans_t fsm_trans_display[] = {
    {WAIT_DISPLAY, check_active, SET_DISPLAY, do_set_on},
    {SET_DISPLAY, check_set_new_color, SET_DISPLAY, do_set_color},
    {SET_DISPLAY, check_off, WAIT_DISPLAY, do_set_off},
    {-1, NULL, -1, NULL}};

/* Other auxiliary functions */
/**
 * @brief Initialize a display system FSM.
 *
 * This function initializes the default values of the FSM struct and calls to the `port` to initialize the associated HW given the ID.
 *
 * The FSM stores the display level of the display system. The user should set it using the function `fsm_display_set_distance()`.
 *
 * @attention This display system is agnostic to the ultrasound sensor or any other sensor. It only shows the status of the display system set by the user. It does not matter if the display is for a parking sensor, a door sensor, or any other sensor. The display system only shows a status according to a distance set by the user.
 *
 * The FSM contains information of the RGB LED ID. This ID is a unique identifier that is managed by the user in the `port`. That is where the user provides identifiers and HW information for all the RGB LEDs on his system. The FSM does not have to know anything of the underlying HW.
 *
 * @param p_fsm_display Pointer to the display FSM.
 * @param display_id Unique display identifier number.
 */
static void fsm_display_init(fsm_display_t *p_fsm_display, uint32_t display_id)
{
    fsm_init(&p_fsm_display->f, fsm_trans_display);
    p_fsm_display->display_id = display_id;
    p_fsm_display->distance_cm = -1;
    p_fsm_display->new_color = false;
    p_fsm_display->status = false;
    p_fsm_display->idle = false;
    port_display_init(display_id);
}

/* Public functions -----------------------------------------------------------*/
fsm_display_t *fsm_display_new(uint32_t display_id)
{
    fsm_display_t *p_fsm_display = malloc(sizeof(fsm_display_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_display_init(p_fsm_display, display_id);                  /* Initialize the FSM */
    return p_fsm_display;
}

void fsm_display_destroy(fsm_display_t *p_fsm)
{
    free(&p_fsm->f);
}

void fsm_display_fire(fsm_display_t *p_fsm)
{
    fsm_fire(&p_fsm->f); // Is it also possible to it in this way: fsm_fire((fsm_t *)p_fsm);
}

void fsm_display_set_distance(fsm_display_t *p_fsm, uint32_t distance_cm)
{
    p_fsm->distance_cm = distance_cm;
    p_fsm->new_color = true;
}

bool fsm_display_get_status(fsm_display_t *p_fsm)
{
    return p_fsm->status;
}

void fsm_display_set_status(fsm_display_t *p_fsm, bool status)
{
    p_fsm->status = status;
}

bool fsm_display_check_activity(fsm_display_t *p_fsm)
{
    return (p_fsm->status && !p_fsm->idle);
}

fsm_t *fsm_display_get_inner_fsm(fsm_display_t *p_fsm)
{
    return &p_fsm->f;
}

uint32_t fsm_display_get_state(fsm_display_t *p_fsm)
{
    return p_fsm->f.current_state;
}

void fsm_display_set_state(fsm_display_t *p_fsm, int8_t state)
{
    p_fsm->f.current_state = state;
}