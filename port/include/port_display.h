/**
 * @file port_display.h
 * @brief Header for the portable functions to interact with the HW of the display system. The functions must be implemented in the platform-specific code.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-04-02
 */
#ifndef PORT_DISPLAY_SYSTEM_H_
#define PORT_DISPLAY_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure to define an RGB color.
 * 
 */
typedef struct{
    
    /**
     * @brief Red color value
     * 
     */
    uint8_t r;

    /**
     * @brief Green color value
     * 
     */
    uint8_t g;

    /**
     * @brief Blue color value
     * 
     */
    uint8_t b;

} rgb_color_t;

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
/**
 * @brief Display system identifier for the front parking sensor
 * 
 */
#define 	PORT_FRONT_PARKING_DISPLAY_ID   1

/**
 * @brief Display system identifier for the rear parking sensor
 * 
 */
#define 	PORT_REAR_PARKING_DISPLAY_ID   0

/**
 * @brief Maximum value for the RGB LED
 * 
 */
#define 	PORT_DISPLAY_RGB_MAX_VALUE 255
 
/**
 * @brief Red color
 * 
 */
#define 	COLOR_RED (rgb_color_t){PORT_DISPLAY_RGB_MAX_VALUE, 0, 0}
 
/**
 * @brief Green color
 * 
 */
#define 	COLOR_GREEN (rgb_color_t){0, PORT_DISPLAY_RGB_MAX_VALUE, 0}
 
/**
 * @brief Blue color
 * 
 */
#define 	COLOR_BLUE (rgb_color_t){0, 0, PORT_DISPLAY_RGB_MAX_VALUE}
 
/**
 * @brief Yellow color
 * 
 */
#define 	COLOR_YELLOW (rgb_color_t){PORT_DISPLAY_RGB_MAX_VALUE*37/100, PORT_DISPLAY_RGB_MAX_VALUE*37/100, 0}
 
/**
 * @brief Turquoise color
 * 
 */
#define 	COLOR_TURQUOISE (rgb_color_t){PORT_DISPLAY_RGB_MAX_VALUE/10, PORT_DISPLAY_RGB_MAX_VALUE*35/100, PORT_DISPLAY_RGB_MAX_VALUE*32/100}
 
/**
 * @brief Off color
 * 
 */
#define 	COLOR_OFF   (rgb_color_t){0, 0, 0}

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Configure the HW specifications of a given display.
 * 
 * @param display_id Display ID. This index is used to select the element of the `displays_arr[]` array
 */
void port_display_init	(uint32_t display_id);	

/**
 * @brief Set the Capture/Compare register values for each channel of the RGB LED given a color.
 * 
 * This function disables the timer associated to the RGB LEDs, sets the Capture/Compare register values for each channel of the RGB LED, and enables the timer.
 * 
 * 
 * @attention This function is valid for any given RGB LED, however, each RGB LED has its own timer.
 * 
 * @param display_id Display system identifier number.
 * @param color 	RGB color to set.
 */
void port_display_set_rgb(uint32_t 	display_id, rgb_color_t color );


#endif /* PORT_DISPLAY_SYSTEM_H_ */