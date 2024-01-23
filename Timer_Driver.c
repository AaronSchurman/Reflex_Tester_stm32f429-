/*
 * Timer_Driver.c
 *
 *  Created on: Nov 30, 2023
 *      Author: aaron
 */


#include "Timer_Driver.h"



/**
 * @brief Retrieves the current system tick value.
 *
 * This function returns the current value of the system tick timer, which typically counts the number
 * of milliseconds since the system started.
 *
 * @return The current system tick value as a 32-bit unsigned integer.
 * @note This function is a wrapper around the HAL library's `HAL_GetTick` function. The tick value
 *       can be used for timing operations, measuring time intervals, or implementing non-blocking delays.
 *
 * Example usage:
 * @code
 *     uint32_t currentTick = getTick(); // Retrieves the current system tick value
 * @endcode
 */
uint32_t getTick(void){

	uint32_t tick = HAL_GetTick();

	return tick;
}

/**
 * @brief Calculates the time difference between two tick values.
 *
 * This function computes the difference in time between a start tick and a stop tick.
 * It is typically used to determine the duration of an event or a process, measured in system ticks.
 *
 * @param StartTick The start tick value, representing the start time.
 * @param StopTick The stop tick value, representing the end time.
 * @return The difference between the StopTick and StartTick, indicating the elapsed time.
 * @note The function assumes that the tick values are provided in the same unit and are generated
 *       from a consistent source, like the system tick timer. It simply subtracts the start tick
 *       from the stop tick to determine the elapsed time.
 *
 * Example usage:
 * @code
 *     uint32_t startTime = getTick();
 *     // ... some operations ...
 *     uint32_t stopTime = getTick();
 *     uint32_t elapsedTime = getTime(startTime, stopTime); // Calculates the elapsed time of the operations
 * @endcode
 */
uint32_t getTime(uint32_t StartTick, uint32_t StopTick){

	uint32_t difTick = StopTick - StartTick;

	return difTick;

}
