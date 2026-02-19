/**
 * @file stm32f4_button.c
 * @brief Portable functions to interact with the button FSM library. All portable functions must be implemented in this file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-03-26
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdio.h>

/* HW dependent includes */
#include "port_button.h" // Used to get general information about the buttons (ID, etc.)
#include "port_system.h" // Used to get the system tick

/* Microcontroller dependent includes */
#include "stm32f4_system.h"
#include "stm32f4_button.h"


/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure to define the HW dependencies of a button status.
 * 
 */
typedef struct
{
    /**
     * @brief GPIO where the button is connected.
     * 
     */
    GPIO_TypeDef *p_port;

    /**
     * @brief Pin/line where the button is connected.
     * 
     */
    uint8_t pin;

    /**
     * @brief Pull-up/Pull-down mode.
     * 
     */
    uint8_t pupd_mode;

    /**
     * @brief Flag to indicate that the button has been pressed.
     * 
     */
    bool flag_pressed;
} stm32f4_button_hw_t;

/* Global variables ------------------------------------------------------------*/
/**
 * @brief Array of elements that represents the HW characteristics of the buttons connected to the STM32F4 platform.
 * 
 * This must be hidden from the user, so it is declared as static. To access the elements of this array, use the function `_stm32f4_button_get()`.
 * 
 */
static stm32f4_button_hw_t buttons_arr[] = {
    [PORT_PARKING_BUTTON_ID] = {.p_port = STM32F4_PARKING_BUTTON_GPIO, .pin = STM32F4_PARKING_BUTTON_PIN, .pupd_mode = STM32F4_GPIO_PUPDR_NOPULL},
};

/* Private functions ----------------------------------------------------------*/
/**
 * @brief Get the button status struct with the given ID.
 *
 * @param button_id Button ID.
 *
 * @return Pointer to the button state struct.
 * @return NULL If the button ID is not valid.
 */
stm32f4_button_hw_t *_stm32f4_button_get(uint32_t button_id)
{
    // Return the pointer to the button with the given ID. If the ID is not valid, return NULL.
    if (button_id < sizeof(buttons_arr) / sizeof(buttons_arr[0]))
    {
        return &buttons_arr[button_id];
    }
    else
    {
        return NULL;
    }
}

/* Public functions -----------------------------------------------------------*/
void port_button_init(uint32_t button_id)
{
    // Retrieve the button struct using the private function and the button ID
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    /* TO-DO alumnos */

    stm32f4_system_gpio_config(p_button->p_port, p_button->pin, 0, p_button->pupd_mode);

    stm32f4_system_gpio_config_exti(p_button->p_port, p_button->pin, 11);

    stm32f4_system_gpio_exti_enable(p_button->pin, 1, 0);
    
}

void stm32f4_button_set_new_gpio(uint32_t button_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    p_button->p_port = p_port;
    p_button->pin = pin;
}

bool port_button_get_pressed (uint32_t button_id){
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    return p_button->flag_pressed;
}

bool port_button_get_value (uint32_t button_id){
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    return stm32f4_system_gpio_read(p_button->p_port, p_button->pin);
}

void port_button_set_pressed (uint32_t button_id, bool pressed){
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    p_button->flag_pressed = pressed;
}

bool port_button_get_pending_interrupt (uint32_t button_id){
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    uint8_t pin = p_button->pin;
    uint32_t mascara = BIT_POS_TO_MASK(pin);
    return (bool)(EXTI->PR & mascara);
}

void port_button_clear_pending_interrupt (uint32_t button_id){
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    uint8_t pin = p_button->pin;
    uint32_t mascara = BIT_POS_TO_MASK(pin);
    EXTI->PR |= mascara;
}

void port_button_disable_interrupts (uint32_t button_id){
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    uint8_t pin = p_button->pin;
    stm32f4_system_gpio_exti_disable(pin);
}