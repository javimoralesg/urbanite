/**
 * @file stm32f4_ultrasound.c
 * @brief Portable functions to interact with the ultrasound FSM library. All portable functions must be implemented in this file.
 * @author Javier Morales
 * @author Cristian Lapides
 * @date 2025-03-26
 */

/* Standard C includes */
#include <stdio.h>
#include <math.h>

/* HW dependent includes */
#include "port_ultrasound.h"
#include "port_system.h"

/* Microcontroller dependent includes */
#include "stm32f4_system.h"
#include "stm32f4_ultrasound.h"

/* Typedefs --------------------------------------------------------------------*/
/**
 * @brief Structure to define the HW dependencies of an ultrasound sensor.
 * 
 */
typedef struct
{
    /**
     * @brief GPIO where the trigger signal is connected.
     * 
     */
    GPIO_TypeDef *p_trigger_port;

    /**
     * @brief GPIO where the echo signal is connected.
     * 
     */
    GPIO_TypeDef *p_echo_port;

    /**
     * @brief Pin/line where the trigger signal is connected.
     * 
     */
    uint8_t trigger_pin;

    /**
     * @brief Pin/line where the echo signal is connected.
     * 
     */
    uint8_t echo_pin;

    /**
     * @brief Alternate function for the echo signal.
     * 
     */
    uint8_t echo_alt_fun;

    /**
     * @brief Flag to indicate that a new measurement can be started.
     * 
     */
    bool trigger_ready;

    /**
     * @brief Flag to indicate that the trigger signal has been sent.
     * 
     */
    bool trigger_end;

    /**
     * @brief Flag to indicate that the echo signal has been received.
     * 
     */
    bool echo_received;

    /**
     * @brief Tick time when the echo signal was received.
     * 
     */
    uint32_t echo_init_tick;

    /**
     * @brief Tick time when the echo signal was received.
     * 
     */
    uint32_t echo_end_tick;

    /**
     * @brief Number of overflows of the timer during the echo signal.
     * 
     */
    uint32_t echo_overflows;
}  stm32f4_ultrasound_hw_t;

/* Global variables */
/**
 * @brief Array of elements that represents the HW characteristics of the ultrasounds connected to the STM32F4 platform.
 * 
 * This must be hidden from the user, so it is declared as static. To access the elements of this array, use the function `_stm32f4_ultrasound_get()`.
 * 
 */
static stm32f4_ultrasound_hw_t ultrasound_arr[] = {
    [PORT_REAR_PARKING_SENSOR_ID] = {
        .p_trigger_port = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO,
        .p_echo_port = STM32F4_REAR_PARKING_SENSOR_ECHO_GPIO,
        .trigger_pin = STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN,
        .echo_pin = STM32F4_REAR_PARKING_SENSOR_ECHO_PIN
    },
    [PORT_FRONT_PARKING_SENSOR_ID] = {
        .p_trigger_port = STM32F4_FRONT_PARKING_SENSOR_TRIGGER_GPIO,
        .p_echo_port = STM32F4_FRONT_PARKING_SENSOR_ECHO_GPIO,
        .trigger_pin = STM32F4_FRONT_PARKING_SENSOR_TRIGGER_PIN,
        .echo_pin = STM32F4_FRONT_PARKING_SENSOR_ECHO_PIN
    },
};

/* Private functions ----------------------------------------------------------*/
/**
 * @brief Get the ultrasound status struct with the given ID.
 *
 * @param ultrasound_id Ultrasound ID.
 *
 * @return Pointer to the ultrasound state struct.
 * @return NULL If the ultrasound ID is not valid.
 */
stm32f4_ultrasound_hw_t *_stm32f4_ultrasound_get (uint32_t ultrasound_id)
{
    // Return the pointer to the ultrasound with the given ID. If the ID is not valid, return NULL.
    if (ultrasound_id < sizeof(ultrasound_arr) / sizeof(ultrasound_arr[0]))
    {
        return &ultrasound_arr[ultrasound_id];
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief Configure the timer that controls the duration of the trigger signal.
 * 
 * This function configures the timers **TIM13** (REAR) and **TIM14** (FRONT) to generate internal interrupts to control the raise and fall of the trigger signal. The duration of the trigger signal is defined in the `PORT_PARKING_SENSOR_TRIGGER_UP_US` macro. This function is called by the `port_ultrasound_init()` public function to configure the timer that controls the duration of the trigger signal.
 * 
 * **To calculare the `ARR` and `PSC` an efficient algorithm is used:**
 * 
 * This option is the most efficient way to calculate the `ARR` and `PSC` values. It is based on the fact that the `ARR` value is near or equal to its maximum value (65535.0). And only one update of the `PSC` is needed. This eliminates the need for a loop, which could be slow if `ARR` is much larger than 0xFFFF.
 * 
 * @param ultrasound_id Ultrasound ID. This ID is used to configure the timer that controls the echo signal of the ultrasound sensor.
 * 
 * @note **The timer is not enabled yet**. This will be done when the trigger signal must be sent.
 */
static void _timer_trigger_setup (uint32_t ultrasound_id)
{
    // Configuramos TIM13
    /*Primero, habilitamos el timer del trigger*/
    RCC->APB1ENR |= RCC_APB1ENR_TIM13EN;
    /*Segundo, inhabilitamos el controlador*/
    TIM13->CR1 &= ~TIM_CR1_CEN;
    /*Tercero, habilitamos el autoreload preload*/
    TIM13->CR1 |= TIM_CR1_ARPE;
    /*Cuarto, aseguramos el inicio del contador a cero*/
    TIM13->CNT = 0;
    /*Quinto, calculamos ARR y PSC para que el periodo del timer sea 10 microsegundos*/
    double reloj = (double) SystemCoreClock;
    double periodo = (double) PORT_PARKING_SENSOR_TRIGGER_UP_US;
    periodo /= 1000000.0;
    double arr = 65535.0;
    double psc = round((periodo * reloj / (arr + 1.0)) - 1.0);
    arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
    if (arr > 65535.0) {
    psc += 1.0;
    arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
    }
    /*Sexto, cargamos ARR y PSC en sus correspondientes registros*/
    TIM13->ARR = (uint32_t)arr;
    TIM13->PSC = (uint32_t)psc;
    /*Septimo, generamos un evento de actualizacion*/
    TIM13->EGR |= TIM_EGR_UG;
    /*Octavo, limpiamos las interrupciones*/
    TIM13->SR &= ~TIM_SR_UIF;
    /*Noveno, habilitamos las interrupciones del timer*/
    TIM13->DIER |= TIM_DIER_UIE;
    /*Decimo, establecemos las prioridades de las interrupciones*/
    NVIC_SetPriority(TIM8_UP_TIM13_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 0));

    // Configuramos TIM14
    /*Primero, habilitamos el timer del trigger*/
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
    /*Segundo, inhabilitamos el controlador*/
    TIM14->CR1 &= ~TIM_CR1_CEN;
    /*Tercero, habilitamos el autoreload preload*/
    TIM14->CR1 |= TIM_CR1_ARPE;
    /*Cuarto, aseguramos el inicio del contador a cero*/
    TIM14->CNT = 0;
    /*Quinto, calculamos ARR y PSC para que el periodo del timer sea 10 microsegundos*/
    /*double*/ reloj = (double) SystemCoreClock;
    /*double*/ periodo = (double) PORT_PARKING_SENSOR_TRIGGER_UP_US;
    periodo /= 1000000.0;
    /*double*/ arr = 65535.0;
    /*double*/ psc = round((periodo * reloj / (arr + 1.0)) - 1.0);
    arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
    if (arr > 65535.0) {
    psc += 1.0;
    arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
    }
    /*Sexto, cargamos ARR y PSC en sus correspondientes registros*/
    TIM14->ARR = (uint32_t)arr;
    TIM14->PSC = (uint32_t)psc;
    /*Septimo, generamos un evento de actualizacion*/
    TIM14->EGR |= TIM_EGR_UG;
    /*Octavo, limpiamos las interrupciones*/
    TIM14->SR &= ~TIM_SR_UIF;
    /*Noveno, habilitamos las interrupciones del timer*/
    TIM14->DIER |= TIM_DIER_UIE;
    /*Decimo, establecemos las prioridades de las interrupciones*/
    NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 0)); 
}

/**
 * @brief Configure the timer that controls the duration of the echo signal.
 * 
 * This function configures the timer **TIM2** as input capture to measure the duration of the echo signal. `port_ultrasound_init()` public function to configure the timer.
 * 
 * @attention The timer that controls the echo signal is specific for each ultrasound sensor and must be configured separately (within a conditional statement). Use the ultrasound ID to select the correct timer.
 * 
 * @note **The timer is not enabled yet**. This will be done when the trigger signal must be sent. **The timer interrupt is not enabled yet**. This will be done when the trigger signal must be sent.
 * 
 * @param ultrasound_id Ultrasound ID. This ID is used to configure the timer that controls the echo signal of the ultrasound sensor.
 */
static void _timer_echo_setup(uint32_t ultrasound_id)
{
    /*Primero, habilitamos el timer del echo y deshabilitamos el contador*/
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->CR1 &= ~TIM_CR1_CEN;
    /*Segundo, calculamos ARR y PSC para que el periodo del timer sea 1 microsegundo*/
    double reloj = (double) SystemCoreClock;
    double periodo = 0.000001;
    double arr = 65535.0;  // Se dice que arr debe configurarse a su máximo
    double psc = (reloj*periodo) - 1;
    TIM2->ARR = (uint32_t)arr;
    TIM2->PSC = (uint32_t)psc;
    /*Tercero, habilitamos el autoreload preload y generamos un evento de actualizacion*/
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR |= TIM_EGR_UG;

    // Configuramos CH2
    /*Cuarto, marcamos la direccion como input en el registros de Captura/Compare*/
    TIM2->CCMR1 |= (0x1 << TIM_CCMR1_CC2S_Pos);
    /*Quinto, deshabilitamos el filtro digital*/
    TIM2->CCMR1 &= ~TIM_CCMR1_IC2F;
    /*Sexto, habilitamos la captura de entrada en ambos flancos (subida y bajada), mediante los bits CC2NP y CC2P del registro CCER*/
    TIM2->CCER |= TIM_CCER_CC2NP;
    TIM2->CCER |= TIM_CCER_CC2P;
    /*Septimo, programamos el preescalado de entrada para capturar cada transicion valida, poner a 0*/
    TIM2->CCMR1 &= ~(TIM_CCMR1_IC2PSC);
    /*Octavo, habilitamos la captura de entrada*/
    TIM2->CCER |= TIM_CCER_CC2E;
    /*Noveno, habilitamos las interrupciones de la captura de entrada*/
    TIM2->DIER |= TIM_DIER_CC2IE;
    
    // Configuramos CH1
    /*Cuarto, marcamos la direccion como input en el registros de Captura/Compare*/
    TIM2->CCMR1 |= (0x1 << TIM_CCMR1_CC1S_Pos);
    /*Quinto, deshabilitamos el filtro digital*/
    TIM2->CCMR1 &= ~TIM_CCMR1_IC1F;
    /*Sexto, habilitamos la captura de entrada en ambos flancos (subida y bajada), mediante los bits CC2NP y CC2P del registro CCER*/
    TIM2->CCER |= TIM_CCER_CC1NP;
    TIM2->CCER |= TIM_CCER_CC1P;
    /*Septimo, programamos el preescalado de entrada para capturar cada transicion valida, poner a 0*/
    TIM2->CCMR1 &= ~(TIM_CCMR1_IC1PSC);
    /*Octavo, habilitamos la captura de entrada*/
    TIM2->CCER |= TIM_CCER_CC1E;
    /*Noveno, habilitamos las interrupciones de la captura de entrada*/
    TIM2->DIER |= TIM_DIER_CC1IE;

    // Configuracion comun
    /*Decimo, actualizamos las interrupciones del timer*/
    TIM2->DIER |= TIM_DIER_UIE;
    /*Undecimo, establecemos las prioridades de las interrupciones*/
    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));
}

/**
 * @brief Configure the timer that controls the duration of the new measurement.
 * 
 * This function configures the timer **TIM10** to generate an internal interrupt to control the duration of a measurement. The duration of a measurement is defined in the `PORT_PARKING_SENSOR_TIMEOUT_MS` macro. This function is called by the `port_ultrasound_init()` public function to configure the timer that controls the duration of the new measurement.
 * 
 * **To calculare the `ARR` and `PSC` an efficient algorithm is used:**
 * 
 * This option is the most efficient way to calculate the `ARR` and `PSC` values. It is based on the fact that the `ARR` value is near or equal to its maximum value (65535.0). And only one update of the `PSC` is needed. This eliminates the need for a loop, which could be slow if `ARR` is much larger than 0xFFFF.
 * 
 * @note **The timer is not enabled yet**. This will be done when the trigger signal must be sent. **The timer interrupt is not enabled yet**. This will be done when the trigger signal must be sent.
 */
void _timer_new_measurement_setup(uint32_t ultrasound_id) 
{
    // Configuramos TIM10
    /*Primero, habilitamos el timer del measurement*/
    RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
    /*Segundo, inhabilitamos el controlador*/
    TIM10->CR1 &= ~TIM_CR1_CEN;
    /*Tercero, habilitamos el autoreload preload*/
    TIM10->CR1 |= TIM_CR1_ARPE;
    /*Cuarto, aseguramos el inicio del contador a cero*/
    TIM10->CNT = 0;
    /*Quinto, calculamos ARR y PSC para que el periodo del timer sea 10 microsegundos*/
    double reloj = (double) SystemCoreClock;
    double periodo = PORT_PARKING_SENSOR_TIMEOUT_MS;
    periodo /= 1000.0;
    double arr = 65535.0;
    double psc = round((periodo * reloj / (arr + 1.0)) - 1.0);
    arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
    if (arr > 65535.0) {
    psc += 1.0;
    arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
    }
    /*Sexto, cargamos ARR y PSC en sus correspondientes registros*/
    TIM10->ARR = (uint32_t)arr;
    TIM10->PSC = (uint32_t)psc;
    /*Septimo, generamos un evento de actualizacion*/
    TIM10->EGR |= TIM_EGR_UG;
    /*Octavo, limpiamos las interrupciones*/
    TIM10->SR &= ~TIM_SR_UIF;
    /*Noveno, habilitamos las interrupciones del timer*/
    TIM10->DIER |= TIM_DIER_UIE;
    /*Decimo, establecemos las prioridades de las interrupciones*/
    NVIC_SetPriority(TIM1_UP_TIM10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));

    // Configuramos TIM6
    /*Primero, habilitamos el timer del measurement*/
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    /*Segundo, inhabilitamos el controlador*/
    TIM6->CR1 &= ~TIM_CR1_CEN;
    /*Tercero, habilitamos el autoreload preload*/
    TIM6->CR1 |= TIM_CR1_ARPE;
    /*Cuarto, aseguramos el inicio del contador a cero*/
    TIM6->CNT = 0;
    /*Quinto, calculamos ARR y PSC para que el periodo del timer sea 10 microsegundos*/
    reloj = (double) SystemCoreClock;
    periodo = PORT_PARKING_SENSOR_TIMEOUT_MS;
    periodo /= 1000.0;
    arr = 65535.0;
    psc = round((periodo * reloj / (arr + 1.0)) - 1.0);
    arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
    if (arr > 65535.0) {
    psc += 1.0;
    arr = round((periodo * reloj / (psc + 1.0)) - 1.0);
    }
    /*Sexto, cargamos ARR y PSC en sus correspondientes registros*/
    TIM6->ARR = (uint32_t)arr;
    TIM6->PSC = (uint32_t)psc;
    /*Septimo, generamos un evento de actualizacion*/
    TIM6->EGR |= TIM_EGR_UG;
    /*Octavo, limpiamos las interrupciones*/
    TIM6->SR &= ~TIM_SR_UIF;
    /*Noveno, habilitamos las interrupciones del timer*/
    TIM6->DIER |= TIM_DIER_UIE;
    /*Decimo, establecemos las prioridades de las interrupciones*/
    NVIC_SetPriority(TIM6_DAC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
}

/* Public functions -----------------------------------------------------------*/
void port_ultrasound_init(uint32_t ultrasound_id)
{
    /* Get the ultrasound sensor */
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);

    /* Trigger pin configuration */
    p_ultrasound->trigger_ready = true;
    p_ultrasound->trigger_end = false;
    stm32f4_system_gpio_config(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, STM32F4_GPIO_MODE_OUT, STM32F4_GPIO_PUPDR_NOPULL);

    /* Echo pin configuration */
    p_ultrasound->echo_alt_fun = STM32F4_AF1;
    p_ultrasound->echo_received = false;
    p_ultrasound->echo_init_tick = 0;
    p_ultrasound->echo_end_tick = 0;
    p_ultrasound->echo_overflows = 0;
    stm32f4_system_gpio_config(p_ultrasound->p_echo_port, p_ultrasound->echo_pin, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config_alternate(p_ultrasound->p_echo_port, p_ultrasound->echo_pin, p_ultrasound->echo_alt_fun);

    /* Configure timers */
    // Se configuran una sola vez
    if (ultrasound_id == 0) {
        _timer_trigger_setup(ultrasound_id);
        _timer_echo_setup(ultrasound_id);
        _timer_new_measurement_setup(ultrasound_id);
    }
}

// Getters and setters functions
void stm32f4_ultrasound_set_new_trigger_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->p_trigger_port = p_port;
    p_ultrasound->trigger_pin = pin;
}

void stm32f4_ultrasound_set_new_echo_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->p_echo_port = p_port;
    p_ultrasound->echo_pin = pin;
}

bool port_ultrasound_get_trigger_ready (uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->trigger_ready;
}

void port_ultrasound_set_trigger_ready (uint32_t ultrasound_id, bool trigger_ready)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->trigger_ready = trigger_ready;
}

bool port_ultrasound_get_trigger_end (uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->trigger_end;
}

void port_ultrasound_set_trigger_end (uint32_t ultrasound_id, bool trigger_end)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->trigger_end = trigger_end;
}

uint32_t port_ultrasound_get_echo_end_tick(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->echo_end_tick;
}

void port_ultrasound_set_echo_end_tick(uint32_t ultrasound_id, uint32_t echo_end_tick)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_end_tick = echo_end_tick;
}

uint32_t port_ultrasound_get_echo_init_tick(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->echo_init_tick;
}

void port_ultrasound_set_echo_init_tick(uint32_t ultrasound_id, uint32_t echo_init_tick)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_init_tick = echo_init_tick;
}

uint32_t port_ultrasound_get_echo_overflows(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->echo_overflows;
}

void port_ultrasound_set_echo_overflows(uint32_t ultrasound_id, uint32_t echo_overflows)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_overflows = echo_overflows;
}

bool port_ultrasound_get_echo_received(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->echo_received;
}

void port_ultrasound_set_echo_received(uint32_t ultrasound_id, bool echo_received)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_received = echo_received;
}

// Util
void port_ultrasound_stop_trigger_timer (uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    stm32f4_system_gpio_write(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, false);
    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID) TIM13->CR1 &= ~TIM_CR1_CEN;
    if (ultrasound_id == PORT_FRONT_PARKING_SENSOR_ID) TIM14->CR1 &= ~TIM_CR1_CEN;
    
}

void port_ultrasound_stop_echo_timer(uint32_t ultrasound_id) 
{
    TIM2->CR1 &= ~TIM_CR1_CEN;
}

void port_ultrasound_reset_echo_ticks(uint32_t ultrasound_id) 
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_init_tick = 0;
    p_ultrasound->echo_end_tick = 0;
    p_ultrasound->echo_overflows = 0;
    p_ultrasound->echo_received = false;
}

void port_ultrasound_start_measurement(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->trigger_ready = false;
    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID)
    {
        TIM13->CNT = 0; 
        TIM2->CNT = 0;
        TIM10->CNT = 0;
    }
    if (ultrasound_id == PORT_FRONT_PARKING_SENSOR_ID)
    {
        TIM14->CNT = 0;
        TIM2->CNT = 0;
        TIM6->CNT = 0;
    }
    stm32f4_system_gpio_write(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, true);
    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID)
    {
        NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);  
        NVIC_EnableIRQ(TIM2_IRQn);
        NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

        TIM13->CR1 |= TIM_CR1_CEN;  
        TIM2->CR1 |= TIM_CR1_CEN; 
        TIM10->CR1 |= TIM_CR1_CEN;
    }
    if (ultrasound_id == PORT_FRONT_PARKING_SENSOR_ID)
    {
        NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
        NVIC_EnableIRQ(TIM2_IRQn);
        NVIC_EnableIRQ(TIM6_DAC_IRQn);

        TIM14->CR1 |= TIM_CR1_CEN; 
        TIM2->CR1 |= TIM_CR1_CEN;  
        TIM6->CR1 |= TIM_CR1_CEN;
    }    
}

void port_ultrasound_start_new_measurement_timer(uint32_t ultrasound_id)
{
    if(ultrasound_id == PORT_REAR_PARKING_SENSOR_ID)
    {
        NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn); 
        TIM10->CR1 |= TIM_CR1_CEN;
    }
    if(ultrasound_id == PORT_FRONT_PARKING_SENSOR_ID)
    {
        NVIC_EnableIRQ(TIM6_DAC_IRQn); 
        TIM6->CR1 |= TIM_CR1_CEN;
    }
}

void port_ultrasound_stop_new_measurement_timer(uint32_t ultrasound_id)
{
    if(ultrasound_id == PORT_REAR_PARKING_SENSOR_ID)
    {
        TIM10->CR1 &= ~TIM_CR1_CEN;
    }
    if(ultrasound_id == PORT_FRONT_PARKING_SENSOR_ID)
    {
        TIM6->CR1 &= ~TIM_CR1_CEN;
    }
}

void port_ultrasound_stop_ultrasound(uint32_t ultrasound_id)
{
    port_ultrasound_stop_trigger_timer(ultrasound_id);
    port_ultrasound_stop_echo_timer(ultrasound_id);
    port_ultrasound_stop_new_measurement_timer(ultrasound_id);
    port_ultrasound_reset_echo_ticks(ultrasound_id);
}