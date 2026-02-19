/**
 * @file fsm_urbanite.c
 * @brief Urbanite FSM main file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-04-23
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <stdio.h>

/* HW dependent includes */
#include "port_system.h"

/* Project includes */
#include "fsm.h"
#include "fsm_urbanite.h"

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure to define the Urbanite FSM.
 *
 */
struct fsm_urbanite_t
{
    /**
     * @brief Urbanite FSM.
     *
     */
    fsm_t f;

    /**
     * @brief Pointer to the button FSM.
     *
     */
    fsm_button_t *p_fsm_button;

    /**
     * @brief Time in ms to consider ON/OFF.
     *
     */
    uint32_t on_off_press_time_ms;

    /**
     * @brief Time in milliseconds to change the current gear (front or rear).
     *
     */
    uint32_t change_press_time_ms;

    /**
     * @brief Time in ms to pause the display.
     *
     */
    uint32_t pause_display_time_ms;

    /**
     * @brief Flag to indicate if the system is paused.
     *
     */
    bool is_paused;

    /**
     * @brief Flag to indicate if the current ultrasound is FRONT (false) or REAR (true).
     *
     */
    bool is_rear;

     /**
     * @brief Pointer to the front ultrasound FSM.
     *
     */
    fsm_ultrasound_t *p_fsm_ultrasound_front;

    /**
     * @brief Pointer to the front display FSM.
     *
     */
    fsm_display_t *p_fsm_display_front;

    /**
     * @brief Pointer to the rear ultrasound FSM.
     *
     */
    fsm_ultrasound_t *p_fsm_ultrasound_rear;

    /**
     * @brief Pointer to the rear display FSM.
     *
     */
    fsm_display_t *p_fsm_display_rear;

    /**
     * @brief Pointer to the buzzer FSM.
     *
     */
    fsm_buzzer_t *p_fsm_buzzer;
};

/* Private functions -----------------------------------------------------------*/
/**
 * @brief Check if the button has been pressed for the required time to turn ON the Urbanite system.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 * @return true
 * @return false
 */
static bool check_on(fsm_t *p_this)
{
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)(p_this);
    uint32_t duration = fsm_button_get_duration(p_fsm->p_fsm_button);
    return (duration > 0) && (duration >= p_fsm->on_off_press_time_ms);
}

/**
 * @brief Check if the button has been pressed for the required time to turn OFF the Urbanite system.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 * @return true
 * @return false
 */
static bool check_off(fsm_t *p_this)
{
    return check_on(p_this);
}

/**
 * @brief Check if a new measurement is ready.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 * @return true
 * @return false
 */
static bool check_new_measure(fsm_t *p_this)
{
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)(p_this);
    if(p_fsm->is_rear) {
        return fsm_ultrasound_get_new_measurement_ready(p_fsm->p_fsm_ultrasound_rear);
    } else {
        return fsm_ultrasound_get_new_measurement_ready(p_fsm->p_fsm_ultrasound_front);
    }
}

/**
 * @brief Check if it has been required to pause the display.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 * @return true
 * @return false
 */
static bool check_pause(fsm_t *p_this)
{
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)(p_this);
    uint32_t duration = fsm_button_get_duration(p_fsm->p_fsm_button);
    return (duration > 0) && (duration < p_fsm->change_press_time_ms) && (duration >= p_fsm->pause_display_time_ms);
}

/**
 * @brief Check if any of the elements of the system is active.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 * @return true
 * @return false
 */
static bool check_activity(fsm_t *p_this)
{
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)(p_this);
    bool button_status = fsm_button_check_activity(p_fsm->p_fsm_button);
    bool ultrasound_front_status = fsm_ultrasound_check_activity(p_fsm->p_fsm_ultrasound_front);
    bool display_front_status = fsm_display_check_activity(p_fsm->p_fsm_display_front);
    bool ultrasound_rear_status = fsm_ultrasound_check_activity(p_fsm->p_fsm_ultrasound_rear);
    bool display_rear_status = fsm_display_check_activity(p_fsm->p_fsm_display_rear);
    bool buzzer_status = fsm_buzzer_check_activity(p_fsm->p_fsm_buzzer);
    return (button_status || ultrasound_front_status || display_front_status || ultrasound_rear_status || display_rear_status || buzzer_status);
}

/**
 * @brief Check if all the elements of the system are inactive.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 * @return true
 * @return false
 */
static bool check_no_activity(fsm_t *p_this)
{
    /* return !check_activity(p_this); */
    return false;
}

/**
 * @brief Check if any a new measurement is ready while the system is in low power mode.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 * @return true
 * @return false
 */
static bool check_activity_in_measure(fsm_t *p_this)
{
    return check_new_measure(p_this);
}

/**
 * @brief Check if the button has been pressed for the required time to change the current gear to FRONT.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 * @return true
 * @return false
 */
static bool check_front(fsm_t *p_this)
{
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)(p_this);
    uint32_t duration = fsm_button_get_duration(p_fsm->p_fsm_button);
    return (duration > 0) && (duration >= p_fsm->change_press_time_ms) && (duration < p_fsm->on_off_press_time_ms);
}

/**
 * @brief Check if the button has been pressed for the required time to change the current gear to REAR.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 * @return true
 * @return false
 */
static bool check_rear(fsm_t *p_this)
{
    return check_front(p_this);
}

/**
 * @brief Turn the Urbanite system ON.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 */
static void do_start_up_measure(fsm_t *p_this)
{
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)(p_this);
    fsm_button_reset_duration(p_fsm->p_fsm_button);

    fsm_ultrasound_start(p_fsm->p_fsm_ultrasound_front);
    fsm_display_set_status(p_fsm->p_fsm_display_front, false);
    fsm_buzzer_set_status(p_fsm->p_fsm_buzzer, false);

    printf("[URBANITE][%ld] Urbanite system ON\n", port_system_get_millis());
}

/**
 * @brief Display the distance measured by the ultrasound sensor.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 */
static void do_distance(fsm_t *p_this)
{
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)(p_this);
        
    if (!(p_fsm->is_rear)) {
        fsm_display_set_status(p_fsm->p_fsm_display_rear, false);
        
        uint32_t distance = fsm_ultrasound_get_distance(p_fsm->p_fsm_ultrasound_front);

        if (p_fsm->is_paused)
        {
            if (distance < WARNING_MIN_CM / 2)
            {
                fsm_display_set_distance(p_fsm->p_fsm_display_front, distance);
                fsm_buzzer_set_distance(p_fsm->p_fsm_buzzer, distance);

                fsm_display_set_status(p_fsm->p_fsm_display_front, true);
                fsm_buzzer_set_status(p_fsm->p_fsm_buzzer, true);
            }
            else
            {
                fsm_display_set_status(p_fsm->p_fsm_display_front, false);
                fsm_buzzer_set_status(p_fsm->p_fsm_buzzer, false);
            }
        }
        else
        {   
            fsm_display_set_distance(p_fsm->p_fsm_display_front, distance);
            fsm_buzzer_set_distance(p_fsm->p_fsm_buzzer, distance);

            fsm_display_set_status(p_fsm->p_fsm_display_front, true);
            fsm_buzzer_set_status(p_fsm->p_fsm_buzzer, true);
        }

        printf("[URBANITE][%ld] Distance FRONT: %ld cm\n", port_system_get_millis(), distance);

    } else {
        fsm_display_set_status(p_fsm->p_fsm_display_front, false);
        
        uint32_t distance = fsm_ultrasound_get_distance(p_fsm->p_fsm_ultrasound_rear);

        if (p_fsm->is_paused)
        {
            if (distance < WARNING_MIN_CM / 2)
            {
                fsm_display_set_distance(p_fsm->p_fsm_display_rear, distance);
                fsm_buzzer_set_distance(p_fsm->p_fsm_buzzer, distance);

                fsm_display_set_status(p_fsm->p_fsm_display_rear, true);
                fsm_buzzer_set_status(p_fsm->p_fsm_buzzer, true);
            }
            else
            {
                fsm_display_set_status(p_fsm->p_fsm_display_rear, false);
                fsm_buzzer_set_status(p_fsm->p_fsm_buzzer, false);
            }
        }
        else
        {   
            fsm_display_set_distance(p_fsm->p_fsm_display_rear, distance);
            fsm_buzzer_set_distance(p_fsm->p_fsm_buzzer, distance);

            fsm_display_set_status(p_fsm->p_fsm_display_rear, true);
            fsm_buzzer_set_status(p_fsm->p_fsm_buzzer, true);
        }

        printf("[URBANITE][%ld] Distance REAR: %ld cm\n", port_system_get_millis(), distance);
    }
}

/**
 * @brief Pause or resume the display system.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 */
static void do_pause(fsm_t *p_this)
{
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)(p_this);
    fsm_button_reset_duration(p_fsm->p_fsm_button);

    p_fsm->is_paused = !p_fsm->is_paused;

    if(p_fsm->is_rear) {
        fsm_display_set_status(p_fsm->p_fsm_display_rear, p_fsm->is_paused);
    } else {
        fsm_display_set_status(p_fsm->p_fsm_display_front, p_fsm->is_paused);
    }
    fsm_buzzer_set_status(p_fsm->p_fsm_buzzer, p_fsm->is_paused);
    
    if(p_fsm->is_paused) printf("[URBANITE][%ld] Urbanite system display PAUSE\n", port_system_get_millis());
    else printf("[URBANITE][%ld] Urbanite system display RESUME\n", port_system_get_millis());
}

/**
 * @brief Turn the Urbanite system OFF.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 */
static void do_stop_urbanite(fsm_t *p_this)
{
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)(p_this);
    fsm_button_reset_duration(p_fsm->p_fsm_button);

    fsm_ultrasound_stop(p_fsm->p_fsm_ultrasound_front);
    fsm_display_set_status(p_fsm->p_fsm_display_front, false);

    fsm_ultrasound_stop(p_fsm->p_fsm_ultrasound_rear);
    fsm_display_set_status(p_fsm->p_fsm_display_rear, false);

    fsm_buzzer_set_status(p_fsm->p_fsm_buzzer, false);

    if (p_fsm->is_paused)
        p_fsm->is_paused = false;

    printf("[URBANITE][%ld] Urbanite system OFF\n", port_system_get_millis());
}

/**
 * @brief Start the low power mode while the Urbanite is awakened by a debug breakpoint or similar in the `SLEEP_WHILE_OFF` state.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 */
static void do_sleep_while_off(fsm_t *p_this)
{
    port_system_sleep();
}

/**
 * @brief Start the low power mode while the Urbanite is awakened by a debug breakpoint or similar in the `SLEEP_WHILE_ON` state.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 */
static void do_sleep_while_on(fsm_t *p_this)
{
    port_system_sleep();
}

/**
 * @brief Start the low power mode while the Urbanite is measuring the distance and it is waiting for a new measurement.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 */
static void do_sleep_while_measure(fsm_t *p_this)
{
    port_system_sleep();
}

/**
 * @brief Start the low power mode while the Urbanite is OFF.
 *
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 */
static void do_sleep_off(fsm_t *p_this)
{
    port_system_sleep();
}

/**
 * @brief Change the current ultrasound and display to the REAR ones.
 * 
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 */
static void do_change_rear(fsm_t *p_this)
{
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)(p_this);

    fsm_button_reset_duration(p_fsm->p_fsm_button);

    fsm_ultrasound_stop(p_fsm->p_fsm_ultrasound_front);
    fsm_display_set_status(p_fsm->p_fsm_display_front, false);

    p_fsm->is_rear = true;
    fsm_ultrasound_start(p_fsm->p_fsm_ultrasound_rear);
    fsm_display_set_status(p_fsm->p_fsm_display_rear, false);

    printf("[URBANITE][%ld] Urbanite change REAR\n", port_system_get_millis());
}

/**
 * @brief Change the current ultrasound and display to the FRONT ones.
 * 
 * @param p_this Pointer to an `fsm_t` struct that contains an `fsm_urbanite_t`.
 */
static void do_change_front(fsm_t *p_this)
{
    fsm_urbanite_t *p_fsm = (fsm_urbanite_t *)(p_this);

    fsm_button_reset_duration(p_fsm->p_fsm_button);
    
    fsm_ultrasound_stop(p_fsm->p_fsm_ultrasound_rear);
    fsm_display_set_status(p_fsm->p_fsm_display_rear, false);

    p_fsm->is_rear = false;
    fsm_ultrasound_start(p_fsm->p_fsm_ultrasound_front);
    fsm_display_set_status(p_fsm->p_fsm_display_front, false);

    printf("[URBANITE][%ld] Urbanite change FRONT\n", port_system_get_millis());
}

/* State machine output or action functions */
/**
 * @brief Array representing the transitions table of the FSM Urbanite.
 *
 * @image html fsm_urbanite.png
 *
 */
static fsm_trans_t fsm_trans_urbanite[] = {
    {OFF, check_on, MEASURE_FRONT, do_start_up_measure},
    {OFF, check_no_activity, SLEEP_WHILE_OFF, do_sleep_off},

    {MEASURE_FRONT, check_off, OFF, do_stop_urbanite},
    {MEASURE_FRONT, check_pause, MEASURE_FRONT, do_pause},
    {MEASURE_FRONT, check_new_measure, MEASURE_FRONT, do_distance},
    {MEASURE_FRONT, check_rear, MEASURE_REAR, do_change_rear},
    {MEASURE_FRONT, check_no_activity, SLEEP_WHILE_ON_FRONT, do_sleep_while_measure},
    
    {MEASURE_REAR, check_off, OFF, do_stop_urbanite},
    {MEASURE_REAR, check_pause, MEASURE_REAR, do_pause},
    {MEASURE_REAR, check_new_measure, MEASURE_REAR, do_distance},
    {MEASURE_REAR, check_front, MEASURE_FRONT, do_change_front},
    {MEASURE_REAR, check_no_activity, SLEEP_WHILE_ON_REAR, do_sleep_while_measure},

    {SLEEP_WHILE_ON_FRONT, check_activity_in_measure, MEASURE_FRONT, NULL},
    {SLEEP_WHILE_ON_FRONT, check_no_activity, SLEEP_WHILE_ON_FRONT, do_sleep_while_on},
    
    {SLEEP_WHILE_ON_REAR, check_activity_in_measure, MEASURE_REAR, NULL},
    {SLEEP_WHILE_ON_REAR, check_no_activity, SLEEP_WHILE_ON_REAR, do_sleep_while_on},
    
    {SLEEP_WHILE_OFF, check_activity, OFF, NULL},
    {SLEEP_WHILE_OFF, check_no_activity, SLEEP_WHILE_OFF, do_sleep_while_off},
    
    {-1, NULL, -1, NULL}

};

/* Other auxiliary functions */
/**
 * @brief Create a new Urbanite FSM.
 *
 * This FSM implements a system that measures the distance to the nearest obstacle and shows it on a display. The display is a RGB LED that changes its color depending on the distance to the obstacle. The system is activated by pressing a button. The system is deactivated by pressing the same button for the same time.
 *
 * The basic implementation of this FSM assumes that the system is mounted on a car and the distance is measured by an ultrasound sensor located at the rear of the car. The display is located on the dashboard of the car. The button is assumed that activates when the driver is parking the car. The system can add more sensors and displays to cover more areas of the car.
 *
 * A short press of the button pauses the display if it disturbs the driver, but the system continues measuring the distance. In case the driver wants to activate the display again, he must press the button again and the display will show the last distance measured. A long press of the button deactivates the ultrasounds and the displays.
 *
 * When the system is OFF it does not measure the distance and the display is OFF. The system is in a low power mode.
 *
 * This function initializes the default values of the FSM struct and calls to the `port` to initialize the HWs associated to the devices.
 *
 * @param p_fsm_urbanite 	        Pointer to the Urbanite FSM.
 * @param p_fsm_button 	            Pointer to the button FSM that activates the system and disables the display if it disturbs the driver.
 * @param on_off_press_time_ms      Button press time in milliseconds to turn on or off the system.
 * @param change_press_time_ms      Time in milliseconds to change the current gear (front or rear).
 * @param pause_display_time_ms     Time in milliseconds to pause the display after a short press of the button.
 * @param p_fsm_ultrasound_front Pointer to the front ultrasound FSM.
 * @param p_fsm_display_front Pointer to the front display FSM.
 * @param p_fsm_ultrasound_rear     Pointer to the ultrasound FSM that measures the distance to the rear obstacle.
 * @param p_fsm_display_rear 	    Pointer to the display FSM that shows the distance to the rear obstacle.
 * @param p_fsm_buzzer          Pointer to the buzzer FSM that sounds depending on the distance to the obstacle.
 */
static void fsm_urbanite_init(fsm_urbanite_t *p_fsm_urbanite, fsm_button_t *p_fsm_button, uint32_t on_off_press_time_ms, uint32_t change_press_time_ms, uint32_t pause_display_time_ms, fsm_ultrasound_t *p_fsm_ultrasound_front, fsm_display_t *p_fsm_display_front, fsm_ultrasound_t *p_fsm_ultrasound_rear, fsm_display_t *p_fsm_display_rear, fsm_buzzer_t *p_fsm_buzzer)
{
    fsm_init(&p_fsm_urbanite->f, fsm_trans_urbanite);

    p_fsm_urbanite->p_fsm_button = p_fsm_button;
    p_fsm_urbanite->on_off_press_time_ms = on_off_press_time_ms;
    p_fsm_urbanite->change_press_time_ms = change_press_time_ms;
    p_fsm_urbanite->pause_display_time_ms = pause_display_time_ms;
    p_fsm_urbanite->p_fsm_ultrasound_front = p_fsm_ultrasound_front;
    p_fsm_urbanite->p_fsm_display_front = p_fsm_display_front;
    p_fsm_urbanite->p_fsm_ultrasound_rear = p_fsm_ultrasound_rear;
    p_fsm_urbanite->p_fsm_display_rear = p_fsm_display_rear;
    p_fsm_urbanite->p_fsm_buzzer = p_fsm_buzzer;
    p_fsm_urbanite->is_paused = false;
    p_fsm_urbanite->is_rear = false;
}

/* Public functions ------------------------------------------------------------*/
fsm_urbanite_t *fsm_urbanite_new(fsm_button_t *p_fsm_button, uint32_t on_off_press_time_ms,  uint32_t change_press_time_ms, uint32_t pause_display_time_ms, fsm_ultrasound_t *p_fsm_ultrasound_front, fsm_display_t *p_fsm_display_front, fsm_ultrasound_t *p_fsm_ultrasound_rear, fsm_display_t *p_fsm_display_rear, fsm_buzzer_t *p_fsm_buzzer)
{
    fsm_urbanite_t *p_fsm_urbanite = malloc(sizeof(fsm_urbanite_t));                                                                         /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_urbanite_init(p_fsm_urbanite, p_fsm_button, on_off_press_time_ms, change_press_time_ms, pause_display_time_ms, p_fsm_ultrasound_front, p_fsm_display_front, p_fsm_ultrasound_rear, p_fsm_display_rear, p_fsm_buzzer); /* Initialize the FSM */
    return p_fsm_urbanite;
}

void fsm_urbanite_fire(fsm_urbanite_t *p_fsm)
{
    fsm_fire(&p_fsm->f); // It is also possible to it in this way: fsm_fire((fsm_t *)p_fsm);
}

void fsm_urbanite_destroy(fsm_urbanite_t *p_fsm)
{
    free(&p_fsm->f);
}
