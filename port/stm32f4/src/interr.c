/**
 * @file interr.c
 * @brief Interrupt service routines for the STM32F4 platform.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-03-26
 */
// Include HW dependencies:
#include "port_system.h"
#include "stm32f4_system.h"
#include "port_button.h"
#include "stm32f4_button.h"
#include "port_ultrasound.h"
#include "stm32f4_ultrasound.h"

// Include headers of different port elements:

//------------------------------------------------------
// INTERRUPT SERVICE ROUTINES
//------------------------------------------------------
/**
 * @brief Interrupt service routine for the System tick timer (SysTick).
 *
 * @note This ISR is called when the SysTick timer generates an interrupt.
 * The program flow jumps to this ISR and increments the tick counter by one millisecond.
 *
 * @warning **The variable `msTicks` must be declared volatile!** Just because it is modified by a call of an ISR, in order to avoid [*race conditions*](https://en.wikipedia.org/wiki/Race_condition). **Added to the definition** after *static*.
 *
 */
void SysTick_Handler(void)
{
    uint32_t ms = port_system_get_millis() + 1;
    port_system_set_millis(ms);
}

/**
 * @brief This function handles Px10-Px15 global interrupts.
 * 
 * First, this function identifies the line/ pin which has raised the interruption. Then, perform the desired action. Before leaving it cleans the interrupt pending register.
 * 
 */
void EXTI15_10_IRQHandler(void)
{
    /* ISR parking button */
    port_system_systick_resume();
    if (port_button_get_pending_interrupt(PORT_PARKING_BUTTON_ID))
    {
        if(port_button_get_value(PORT_PARKING_BUTTON_ID)) {
            port_button_set_pressed(PORT_PARKING_BUTTON_ID, false);
        } else {
            port_button_set_pressed(PORT_PARKING_BUTTON_ID, true);
        }
        port_button_clear_pending_interrupt(PORT_PARKING_BUTTON_ID);
    }
}

/**
 * @brief Interrupt service routine for the TIM2 timer.
 * 
 * This timer controls the duration of the measurements of the ultrasound sensor. When the interrupt occurs it means that the time of the a measurement has expired and a new measurement can be started.
 * 
 * **The timer can interrupt in two cases:**
 * 
 * 1. When the echo signal has not been received and the ARR register overflows. In this case, the echo_overflows counter is incremented.
 * 
 * 2. When the echo signal has been received. In this case, the echo_init_tick and echo_end_tick are updated.
 * 
 */
void TIM2_IRQHandler(void)
{
    /* ISR ultrasound echo timer*/
    port_system_systick_resume();
    if (TIM2->SR & TIM_SR_UIF)
    {
        uint32_t overflows = port_ultrasound_get_echo_overflows(PORT_REAR_PARKING_SENSOR_ID);
        overflows++;
        port_ultrasound_set_echo_overflows(PORT_REAR_PARKING_SENSOR_ID, overflows);
        overflows = port_ultrasound_get_echo_overflows(PORT_FRONT_PARKING_SENSOR_ID);
        overflows++;
        port_ultrasound_set_echo_overflows(PORT_FRONT_PARKING_SENSOR_ID, overflows);
        TIM2->SR &= ~TIM_SR_UIF;
    }
    
    if (TIM2->SR & TIM_SR_CC2IF)
    {
        uint32_t currentTicks = TIM2->CCR2;
        uint32_t init = port_ultrasound_get_echo_init_tick(PORT_REAR_PARKING_SENSOR_ID);
        uint32_t end = port_ultrasound_get_echo_end_tick(PORT_REAR_PARKING_SENSOR_ID);
        if (init == 0 && end == 0)
        {
            port_ultrasound_set_echo_init_tick(PORT_REAR_PARKING_SENSOR_ID, currentTicks);
        } 
        else
        {
            port_ultrasound_set_echo_end_tick(PORT_REAR_PARKING_SENSOR_ID, currentTicks);
            port_ultrasound_set_echo_received(PORT_REAR_PARKING_SENSOR_ID, true);
        }
    }
    if (TIM2->SR & TIM_SR_CC1IF)
    {
        uint32_t currentTicks = TIM2->CCR1;
        uint32_t init = port_ultrasound_get_echo_init_tick(PORT_FRONT_PARKING_SENSOR_ID);
        uint32_t end = port_ultrasound_get_echo_end_tick(PORT_FRONT_PARKING_SENSOR_ID);
        if (init == 0 && end == 0)
        {
            port_ultrasound_set_echo_init_tick(PORT_FRONT_PARKING_SENSOR_ID, currentTicks);
        } 
        else
        {
            port_ultrasound_set_echo_end_tick(PORT_FRONT_PARKING_SENSOR_ID, currentTicks);
            port_ultrasound_set_echo_received(PORT_FRONT_PARKING_SENSOR_ID, true);
        }
    }
}

/**
 * @brief Interrupt service routine for the TIM10 timer.
 * 
 * This timer controls the duration of the measurements of the ultrasound sensor. When the interrupt occurs it means that the time of the a measurement has expired and a new measurement can be started.
 * 
 */
void TIM1_UP_TIM10_IRQHandler(void)
{
    /* ISR ultrasound new measurement timer */
    TIM10->SR &= ~TIM_SR_UIF;
    port_ultrasound_set_trigger_ready(PORT_REAR_PARKING_SENSOR_ID, true); 
}

/**
 * @brief Interrupt service routine for the TIM6 timer.
 * 
 * This timer controls the duration of the measurements of the ultrasound sensor. When the interrupt occurs it means that the time of the a measurement has expired and a new measurement can be started.
 * 
 */
void TIM6_DAC_IRQHandler(void)
{
    /* ISR ultrasound new measurement timer */
    TIM6->SR &= ~TIM_SR_UIF;
    port_ultrasound_set_trigger_ready(PORT_FRONT_PARKING_SENSOR_ID, true); 
}

/**
 * @brief Interrupt service routine for the TIM13 timer.
 * 
 * This timer controls the duration of the trigger signal of the REAR ultrasound sensor. When the interrupt occurs it means that the time of the trigger signal has expired and must be lowered.
 * 
 */
void TIM8_UP_TIM13_IRQHandler(void)
{
/* ISR ultrasound trigger timer */
    TIM13->SR &= ~TIM_SR_UIF;
    port_ultrasound_set_trigger_end(PORT_REAR_PARKING_SENSOR_ID, true);
}

/**
 * @brief Interrupt service routine for the TIM14 timer.
 * 
 * This timer controls the duration of the trigger signal of the FRONT ultrasound sensor. When the interrupt occurs it means that the time of the trigger signal has expired and must be lowered.
 * 
 */
void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
    TIM14->SR &= ~TIM_SR_UIF;
    port_ultrasound_set_trigger_end(PORT_FRONT_PARKING_SENSOR_ID, true);
}

