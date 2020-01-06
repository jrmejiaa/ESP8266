#ifndef DWT_STM32_DELAY_H
#define DWT_STM32_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx_hal.h"

/**
 * @brief  Initializes DWT_Cycle_Count for DWT_Delay_us function
 * @return Error DWT counter
 *         1: DWT counter Error
 *         0: DWT counter works
 */
uint32_t DWT_Delay_Init(void);
void DWT_Delay_us(volatile uint32_t microseconds);


/**
 * @brief  This function provides a delay (in microseconds)
 * @param  microseconds: delay in microseconds
 */


#ifdef __cplusplus
}
#endif

#endif
