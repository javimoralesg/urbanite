/**
 * @file stm32f4_buzzer.c
 * @brief Portable functions to interact with the buzzer system FSM library. All portable functions must be implemented in this file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-05-21
 */

/* Standard C includes */
#include <stdio.h>
#include <math.h>

/* HW dependent includes */
#include "port_buzzer.h"
#include "port_system.h"

/* Microcontroller dependent includes */
#include "stm32f4_system.h"
#include "stm32f4_buzzer.h"

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure to define the HW dependencies of an buzzer.
 *
 */
typedef struct
{

    /**
     * @brief GPIO where the buzzer is connected.
     *
     */
    GPIO_TypeDef *p_port_buzzer;

    /**
     * @brief Pin/line where the buzzer is connected.
     *
     */
    uint8_t pin_buzzer;

} stm32f4_buzzer_hw_t;

/* Global variables */
/**
 * @brief Array of elements that represents the HW characteristics of the buzzer systems connected to the STM32F4 platform.
 *
 * To access the elements of this array, use the function [_stm32f4_buzzer_get()](#ae380671bf2f6c9986d7890d91a064fea).
 *
 */
static stm32f4_buzzer_hw_t buzzers_arr[] = {
    [PORT_PARKING_BUZZER_ID] = {
        .p_port_buzzer = STM32F4_PARKING_BUZZER_GPIO,
        .pin_buzzer = STM32F4_PARKING_BUZZER_PIN
    }
};

/* Private functions -----------------------------------------------------------*/
/**
 * @brief Get the buzzer struct with the given ID.
 *
 * @param buzzer_id	Buzzer ID.

 * @return Pointer to the buzzer state struct.
 * @return NULL If the buzzer ID is not valid.
 */
stm32f4_buzzer_hw_t *_stm32f4_buzzer_get(uint32_t buzzer_id)
{
    // Return the pointer to the buzzer with the given ID. If the ID is not valid, return NULL.
    if (buzzer_id < sizeof(buzzers_arr) / sizeof(buzzers_arr[0]))
    {
        return &buzzers_arr[buzzer_id];
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief Configure the timer that controls the PWM of the buzzer system.
 *
 * This function is called by the `port_buzzer_init()` public function to configure the timer that controls the PWM of the buzzer.
 *
 * @note **The duty cycle of the PWM signal is not set yet.** This is done in the `port_buzzer_set_sound()` function where the sound is set.
 *
 * @param buzzer_id 	Buzzer system identifier number.
 */
void _timer_pwm_buzzer_config(uint32_t buzzer_id)
{    
        /*Primero, habilitamos la fuente de reloj del temporizador.*/
        RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
        /*Segundo, inhabilitamos el contador y habilitamos el autoreload preload.*/
        TIM5->CR1 &= ~TIM_CR1_CEN;
        TIM5->CR1 |= TIM_CR1_ARPE;
        /*Tercero, reseteamos el contador*/
        TIM5->CNT = 0;
        /*Cuarto, calculamos ARR y PSC para una frecuencia de 4 kHz, que equivale a un periodo de 0,25 ms.*/
        double reloj = (double)SystemCoreClock;
        double periodo = 0.00025;
        double arr = 65535.0;
        double psc = round((periodo * reloj / (arr + 1.0)) - 1.0);
        arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
        if (arr > 65535.0)
        {
            psc += 1.0;
            arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
        }
        TIM5->ARR = (uint32_t)arr;
        TIM5->PSC = (uint32_t)psc;
        /*Quinto, inhabilitamos la comparación de salida (output compare).*/
        TIM5->CCER &= ~TIM_CCER_CC1E;
        /*Sexto, limpiamos los bits P y NP del Output Compare Register.*/
        TIM5->CCER &= ~TIM_CCER_CC1P;
        TIM5->CCER &= ~TIM_CCER_CC1NP;
        /*Septimo, activamos el modo PWM 1 (bits '110') y habilitamos el preload.*/
        TIM5->CCMR1 |= TIM_CCMR1_OC1PE;
        
        TIM5->CCMR1 &= ~TIM_CCMR1_OC1M_0;
        TIM5->CCMR1 |= TIM_CCMR1_OC1M_1;
        TIM5->CCMR1 |= TIM_CCMR1_OC1M_2;
        /*Octavo, generamos un evento de actualizacion*/
        TIM5->EGR |= TIM_EGR_UG;

}

/* Public functions -----------------------------------------------------------*/
void port_buzzer_init(uint32_t buzzer_id)
{
    stm32f4_buzzer_hw_t *p_buzzer = _stm32f4_buzzer_get(buzzer_id);
    stm32f4_system_gpio_config(p_buzzer->p_port_buzzer, p_buzzer->pin_buzzer, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config_alternate(p_buzzer->p_port_buzzer, p_buzzer->pin_buzzer, STM32F4_AF2);
    /*Finalmente*/
    _timer_pwm_buzzer_config(buzzer_id);
    port_buzzer_set_sound(buzzer_id, PORT_BUZZER_MIN_VALUE);
}

void port_buzzer_set_sound(uint32_t buzzer_id, uint8_t sound)
{
    if (buzzer_id == PORT_PARKING_BUZZER_ID)
    {
        TIM5->CR1 &= ~TIM_CR1_CEN;
        if (sound == 0)
        {
            /*Inhabilitamos output compare.*/
            TIM5->CCER &= ~TIM_CCER_CC1E;
        }
        else
        {
            /*Configuramos el duty cycle.*/
            uint32_t pwm = (PORT_BUZZER_MAX_VALUE + 1) / 2;
            TIM5->CCR1 = pwm;
            /*Cambiamos el periodo para una frecuencia inversamente proporcional a la distancia desde 200 Hz a 2495 Hz*/
            double reloj = (double)SystemCoreClock;
            double periodo = 1/(200.0 + 9 * ((double) sound));
            double arr = 65535.0;
            double psc = round((periodo * reloj / (arr + 1.0)) - 1.0);
            arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
            if (arr > 65535.0)
            {
                psc += 1.0;
                arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
            }
            TIM5->ARR = (uint32_t)arr;
            TIM5->PSC = (uint32_t)psc;
            /*Habilitamos output compare.*/
            TIM5->CCER |= TIM_CCER_CC1E;
        }
        /*Finalmente, generamos un evento de actualizacion y habilitamos el contador.*/
        TIM5->EGR |= TIM_EGR_UG;
        TIM5->CR1 |= TIM_CR1_CEN;
        return;
    }
    else
    {
        return;
    }
}