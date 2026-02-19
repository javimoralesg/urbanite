/**
 * @file main.c
 * @brief Main file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-04-23
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C libraries */
#include <stdio.h> // printf
#include <stdlib.h>
#include <stdint.h>

/* HW libraries */
#include "port_system.h"
#include "port_button.h"
#include "port_ultrasound.h"
#include "port_display.h"
#include "port_buzzer.h"

/* Project includes */
#include "fsm.h"
#include "fsm_button.h"
#include "fsm_ultrasound.h"
#include "fsm_display.h"
#include "fsm_buzzer.h"
#include "fsm_urbanite.h"

/* Defines ------------------------------------------------------------------*/
/**
 * @brief Time in ms to activate the Urbanite system.
 *
 */
#define URBANITE_ON_OFF_PRESS_TIME_MS 3000

/**
 * @brief Time in ms to change between front and rear maneuver.
 *
 */
#define URBANITE_FRONT_REAR_PRESS_TIME_MS 1000

/**
 * @brief Time in ms to pause the display.
 *
 */
#define URBANITE_PAUSE_DISPLAY_TIME_MS 500


/**
 * @brief  Main function. Entry point of the program.
 * @retval int Status of the execution.
 */
int main(void)
{
    /* Init board */
    port_system_init();
    fsm_button_t *p_fsm_button = fsm_button_new(PORT_PARKING_BUTTON_DEBOUNCE_TIME_MS, PORT_PARKING_BUTTON_ID);
    fsm_ultrasound_t *p_fsm_ultrasound_front = fsm_ultrasound_new(PORT_FRONT_PARKING_SENSOR_ID);
    fsm_display_t *p_fsm_display_front = fsm_display_new(PORT_FRONT_PARKING_DISPLAY_ID);
    fsm_ultrasound_t *p_fsm_ultrasound_rear = fsm_ultrasound_new(PORT_REAR_PARKING_SENSOR_ID);
    fsm_display_t *p_fsm_display_rear = fsm_display_new(PORT_REAR_PARKING_DISPLAY_ID);
    fsm_buzzer_t *p_fsm_buzzer = fsm_buzzer_new(PORT_PARKING_BUZZER_ID);
    fsm_urbanite_t *p_fsm_urbanite = fsm_urbanite_new(p_fsm_button, URBANITE_ON_OFF_PRESS_TIME_MS, URBANITE_FRONT_REAR_PRESS_TIME_MS, URBANITE_PAUSE_DISPLAY_TIME_MS, p_fsm_ultrasound_front, p_fsm_display_front, p_fsm_ultrasound_rear, p_fsm_display_rear, p_fsm_buzzer);

    /* Infinite loop */
    while (1)
    {
        fsm_button_fire(p_fsm_button);
        fsm_ultrasound_fire(p_fsm_ultrasound_front);
        fsm_display_fire(p_fsm_display_front);
        fsm_ultrasound_fire(p_fsm_ultrasound_rear);
        fsm_display_fire(p_fsm_display_rear);
        fsm_buzzer_fire(p_fsm_buzzer);
        fsm_urbanite_fire(p_fsm_urbanite);
    } // End of while(1)

    fsm_button_destroy(p_fsm_button);
    fsm_ultrasound_destroy(p_fsm_ultrasound_front);
    fsm_display_destroy(p_fsm_display_front);
    fsm_ultrasound_destroy(p_fsm_ultrasound_rear);
    fsm_display_destroy(p_fsm_display_rear);
    fsm_buzzer_destroy(p_fsm_buzzer);
    fsm_urbanite_destroy(p_fsm_urbanite);

    return 0;
}
