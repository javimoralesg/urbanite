/**
 * @file stm32f4_display.c
 * @brief Portable functions to interact with the display system FSM library. All portable functions must be implemented in this file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-04-02
 */

/* Standard C includes */
#include <stdio.h>
#include <math.h>

/* HW dependent includes */
#include "port_display.h"
#include "port_system.h"

/* Microcontroller dependent includes */
#include "stm32f4_system.h"
#include "stm32f4_display.h"

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure to define the HW dependencies of an RGB LED.
 *
 */
typedef struct
{

    /**
     * @brief GPIO where the RED LED is connected.
     *
     */
    GPIO_TypeDef *p_port_red;

    /**
     * @brief Pin/line where the RED LED is connected.
     *
     */
    uint8_t pin_red;

    /**
     * @brief GPIO where the GREEN LED is connected.
     *
     */
    GPIO_TypeDef *p_port_green;

    /**
     * @brief Pin/line where the GREEN LED is connected.
     *
     */
    uint8_t pin_green;

    /**
     * @brief GPIO where the BLUE LED is connected.
     *
     */
    GPIO_TypeDef *p_port_blue;

    /**
     * @brief Pin/line where the BLUE LED is connected.
     *
     */
    uint8_t pin_blue;
} stm32f4_display_hw_t;

/* Global variables */
/**
 * @brief Array of elements that represents the HW characteristics of the RGB LED of the display systems connected to the STM32F4 platform.
 *
 * To access the elements of this array, use the function [_stm32f4_display_get()](#ae380671bf2f6c9986d7890d91a064fea).
 *
 */
static stm32f4_display_hw_t displays_arr[] = {
    [PORT_REAR_PARKING_DISPLAY_ID] = {
        .p_port_red = STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO,
        .pin_red = STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN,
        .p_port_green = STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO,
        .pin_green = STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN,
        .p_port_blue = STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO,
        .pin_blue = STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN},
    [PORT_FRONT_PARKING_DISPLAY_ID] = {
        .p_port_red = STM32F4_FRONT_PARKING_DISPLAY_RGB_R_GPIO,
        .pin_red = STM32F4_FRONT_PARKING_DISPLAY_RGB_R_PIN,
        .p_port_green = STM32F4_FRONT_PARKING_DISPLAY_RGB_G_GPIO,
        .pin_green = STM32F4_FRONT_PARKING_DISPLAY_RGB_G_PIN,
        .p_port_blue = STM32F4_FRONT_PARKING_DISPLAY_RGB_B_GPIO,
        .pin_blue = STM32F4_FRONT_PARKING_DISPLAY_RGB_B_PIN},
};

/* Private functions -----------------------------------------------------------*/
/**
 * @brief Get the display struct with the given ID.
 *
 * @param display_id	Button ID.

 * @return Pointer to the display state struct.
 * @return NULL If the display ID is not valid.
 */
stm32f4_display_hw_t *_stm32f4_display_get(uint32_t display_id)
{
    // Return the pointer to the display with the given ID. If the ID is not valid, return NULL.
    if (display_id < sizeof(displays_arr) / sizeof(displays_arr[0]))
    {
        return &displays_arr[display_id];
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief Configure the timer that controls the PWM of **each one** of the RGB LEDs of the display system.
 *
 * This function is called by the `port_display_init()` public function to configure the timer that controls the PWM of the RGB LEDs of the display.
 *
 * @attention The timer is common for all the RGB LEDs of one RGB display despite each one has its own channel.
 *
 * @note **The duty cycle of the PWM signal is not set yet.** This is done in the `port_display_set_rgb()` function where the color is set.
 *
 * @param display_id 	Display system identifier number.
 */
void _timer_pwm_config(uint32_t display_id)
{
    if (display_id == PORT_REAR_PARKING_DISPLAY_ID)
    {
        /*Primero, habilitamos la fuente de reloj del temporizador.*/
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
        /*Segundo, inhabilitamos el contador y habilitamos el autoreload preload.*/
        TIM4->CR1 &= ~TIM_CR1_CEN;
        TIM4->CR1 |= TIM_CR1_ARPE;
        /*Tercero, reseteamos el contador*/
        TIM4->CNT = 0;
        /*Cuarto, calculamos ARR y PSC para una frecuencia de 50 Hz, que equivale a un periodo de 20 ms.*/
        double reloj = (double)SystemCoreClock;
        double periodo = 0.02;
        double arr = 65535.0;
        double psc = round((periodo * reloj / (arr + 1.0)) - 1.0);
        arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
        if (arr > 65535.0)
        {
            psc += 1.0;
            arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
        }
        TIM4->ARR = (uint32_t)arr;
        TIM4->PSC = (uint32_t)psc;
        /*Quinto, inhabilitamos la comparación de salida (output compare) para los tres canales*/
        TIM4->CCER &= ~TIM_CCER_CC1E;
        TIM4->CCER &= ~TIM_CCER_CC3E;
        TIM4->CCER &= ~TIM_CCER_CC4E;
        /*Sexto, limpiamos los bits P y NP del Output Compare Register para los tres canales*/
        TIM4->CCER &= ~TIM_CCER_CC1P;
        TIM4->CCER &= ~TIM_CCER_CC1NP;
        TIM4->CCER &= ~TIM_CCER_CC3P;
        TIM4->CCER &= ~TIM_CCER_CC3NP;
        TIM4->CCER &= ~TIM_CCER_CC4P;
        TIM4->CCER &= ~TIM_CCER_CC4NP;
        /*Septimo, activamos el modo PWM 1 (bits '110') y habilitamos el preload para los tres canales*/
        TIM4->CCMR1 |= TIM_CCMR1_OC1PE;
        TIM4->CCMR2 |= TIM_CCMR2_OC3PE;
        TIM4->CCMR2 |= TIM_CCMR2_OC4PE;

        TIM4->CCMR1 &= ~TIM_CCMR1_OC1M_0;
        TIM4->CCMR1 |= TIM_CCMR1_OC1M_1;
        TIM4->CCMR1 |= TIM_CCMR1_OC1M_2;
        TIM4->CCMR2 &= ~TIM_CCMR2_OC3M_0;
        TIM4->CCMR2 |= TIM_CCMR2_OC3M_1;
        TIM4->CCMR2 |= TIM_CCMR2_OC3M_2;
        TIM4->CCMR2 &= ~TIM_CCMR2_OC4M_0;
        TIM4->CCMR2 |= TIM_CCMR2_OC4M_1;
        TIM4->CCMR2 |= TIM_CCMR2_OC4M_2;
        /*Octavo, generamos un evento de actualizacion*/
        TIM4->EGR |= TIM_EGR_UG;
    } if (display_id == PORT_FRONT_PARKING_DISPLAY_ID)
    {
        /*Primero, habilitamos la fuente de reloj del temporizador.*/
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
        /*Segundo, inhabilitamos el contador y habilitamos el autoreload preload.*/
        TIM3->CR1 &= ~TIM_CR1_CEN;
        TIM3->CR1 |= TIM_CR1_ARPE;
        /*Tercero, reseteamos el contador*/
        TIM3->CNT = 0;
        /*Cuarto, calculamos ARR y PSC para una frecuencia de 50 Hz, que equivale a un periodo de 20 ms.*/
        double reloj = (double)SystemCoreClock;
        double periodo = 0.02;
        double arr = 65535.0;
        double psc = round((periodo * reloj / (arr + 1.0)) - 1.0);
        arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
        if (arr > 65535.0)
        {
            psc += 1.0;
            arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
        }
        TIM3->ARR = (uint32_t)arr;
        TIM3->PSC = (uint32_t)psc;
        /*Quinto, inhabilitamos la comparación de salida (output compare) para los tres canales*/
        TIM3->CCER &= ~TIM_CCER_CC1E;
        TIM3->CCER &= ~TIM_CCER_CC3E;
        TIM3->CCER &= ~TIM_CCER_CC4E;
        /*Sexto, limpiamos los bits P y NP del Output Compare Register para los tres canales*/
        TIM3->CCER &= ~TIM_CCER_CC1P;
        TIM3->CCER &= ~TIM_CCER_CC1NP;
        TIM3->CCER &= ~TIM_CCER_CC3P;
        TIM3->CCER &= ~TIM_CCER_CC3NP;
        TIM3->CCER &= ~TIM_CCER_CC4P;
        TIM3->CCER &= ~TIM_CCER_CC4NP;
        /*Septimo, activamos el modo PWM 1 (bits '110') y habilitamos el preload para los tres canales*/
        TIM3->CCMR1 |= TIM_CCMR1_OC1PE;
        TIM3->CCMR2 |= TIM_CCMR2_OC3PE;
        TIM3->CCMR2 |= TIM_CCMR2_OC4PE;

        TIM3->CCMR1 &= ~TIM_CCMR1_OC1M_0;
        TIM3->CCMR1 |= TIM_CCMR1_OC1M_1;
        TIM3->CCMR1 |= TIM_CCMR1_OC1M_2;
        TIM3->CCMR2 &= ~TIM_CCMR2_OC3M_0;
        TIM3->CCMR2 |= TIM_CCMR2_OC3M_1;
        TIM3->CCMR2 |= TIM_CCMR2_OC3M_2;
        TIM3->CCMR2 &= ~TIM_CCMR2_OC4M_0;
        TIM3->CCMR2 |= TIM_CCMR2_OC4M_1;
        TIM3->CCMR2 |= TIM_CCMR2_OC4M_2;
        /*Octavo, generamos un evento de actualizacion*/
        TIM3->EGR |= TIM_EGR_UG;
    }
}

/* Public functions -----------------------------------------------------------*/
void port_display_init(uint32_t display_id)
{
    stm32f4_display_hw_t *p_display = _stm32f4_display_get(display_id);
    /*Para el rojo*/
    stm32f4_system_gpio_config(p_display->p_port_red, p_display->pin_red, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config_alternate(p_display->p_port_red, p_display->pin_red, STM32F4_AF2);
    /*Para el verde*/
    stm32f4_system_gpio_config(p_display->p_port_green, p_display->pin_green, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config_alternate(p_display->p_port_green, p_display->pin_green, STM32F4_AF2);
    /*Para el azul*/
    stm32f4_system_gpio_config(p_display->p_port_blue, p_display->pin_blue, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config_alternate(p_display->p_port_blue, p_display->pin_blue, STM32F4_AF2);
    /*Finalmente*/
    _timer_pwm_config(display_id);
    port_display_set_rgb(display_id, COLOR_OFF);
}

void port_display_set_rgb(uint32_t display_id, rgb_color_t color)
{
    uint8_t r = color.r;
    uint8_t g = color.g;
    uint8_t b = color.b;
    if (display_id == PORT_REAR_PARKING_DISPLAY_ID)
    {
        TIM4->CR1 &= ~TIM_CR1_CEN;
        if (r == 0 && g == 0 && b == 0)
        {
            TIM4->CCER &= ~TIM_CCER_CC1E;
            TIM4->CCER &= ~TIM_CCER_CC3E;
            TIM4->CCER &= ~TIM_CCER_CC4E;
            return;
        }
        else
        {
            if (r == 0)
            {
                /*Inhabilitamos output compare solo para el rojo.*/
                TIM4->CCER &= ~TIM_CCER_CC1E;
            }
            else
            {
                /*Configuramos el duty cycle para el rojo.*/
                uint32_t pwm = (TIM4->ARR + 1) * r / PORT_DISPLAY_RGB_MAX_VALUE;
                TIM4->CCR1 = pwm;
                /*Habilitamos output compare solo para el rojo.*/
                TIM4->CCER |= TIM_CCER_CC1E;
            }

            if (g == 0)
            {
                /*Inhabilitamos output compare solo para el verde.*/
                TIM4->CCER &= ~TIM_CCER_CC3E;
            }
            else
            {
                /*Configuramos el duty cycle para el verde.*/
                uint32_t pwm = (TIM4->ARR + 1) * g / PORT_DISPLAY_RGB_MAX_VALUE;
                TIM4->CCR3 = pwm;
                /*Habilitamos output compare solo para el verde.*/
                TIM4->CCER |= TIM_CCER_CC3E;
            }

            if (b == 0)
            {
                /*Inhabilitamos output compare solo para el azul.*/
                TIM4->CCER &= ~TIM_CCER_CC4E;
            }
            else
            {
                /*Configuramos el duty cycle para el azul.*/
                uint32_t pwm = (TIM4->ARR + 1) * b / PORT_DISPLAY_RGB_MAX_VALUE;
                TIM4->CCR4 = pwm;
                /*Habilitamos output compare solo para el azul.*/
                TIM4->CCER |= TIM_CCER_CC4E;
            }
            /*Finalmente, generamos un evento de actualizacion y habilitamos el contador.*/
            TIM4->EGR |= TIM_EGR_UG;
            TIM4->CR1 |= TIM_CR1_CEN;
            return;
        }
    }
    if (display_id == PORT_FRONT_PARKING_DISPLAY_ID)
    {
        TIM3->CR1 &= ~TIM_CR1_CEN;
        if (r == 0 && g == 0 && b == 0)
        {
            TIM3->CCER &= ~TIM_CCER_CC1E;
            TIM3->CCER &= ~TIM_CCER_CC3E;
            TIM3->CCER &= ~TIM_CCER_CC4E;
            return;
        }
        else
        {
            if (r == 0)
            {
                /*Inhabilitamos output compare solo para el rojo.*/
                TIM3->CCER &= ~TIM_CCER_CC1E;
            }
            else
            {
                /*Configuramos el duty cycle para el rojo.*/
                uint32_t pwm = (TIM3->ARR + 1) * r / PORT_DISPLAY_RGB_MAX_VALUE;
                TIM3->CCR1 = pwm;
                /*Habilitamos output compare solo para el rojo.*/
                TIM3->CCER |= TIM_CCER_CC1E;
            }

            if (g == 0)
            {
                /*Inhabilitamos output compare solo para el verde.*/
                TIM3->CCER &= ~TIM_CCER_CC3E;
            }
            else
            {
                /*Configuramos el duty cycle para el verde.*/
                uint32_t pwm = (TIM3->ARR + 1) * g / PORT_DISPLAY_RGB_MAX_VALUE;
                TIM3->CCR3 = pwm;
                /*Habilitamos output compare solo para el verde.*/
                TIM3->CCER |= TIM_CCER_CC3E;
            }

            if (b == 0)
            {
                /*Inhabilitamos output compare solo para el azul.*/
                TIM3->CCER &= ~TIM_CCER_CC4E;
            }
            else
            {
                /*Configuramos el duty cycle para el azul.*/
                uint32_t pwm = (TIM3->ARR + 1) * b / PORT_DISPLAY_RGB_MAX_VALUE;
                TIM3->CCR4 = pwm;
                /*Habilitamos output compare solo para el azul.*/
                TIM3->CCER |= TIM_CCER_CC4E;
            }
            /*Finalmente, generamos un evento de actualizacion y habilitamos el contador.*/
            TIM3->EGR |= TIM_EGR_UG;
            TIM3->CR1 |= TIM_CR1_CEN;
            return;
        }
    }
    else
    {
        return;
    }
}