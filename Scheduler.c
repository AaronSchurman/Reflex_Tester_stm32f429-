/*
 * Schedualer.c
 *
 *  Created on: Sep 5, 2023
 *      Author: aaron
 */

#include "Scheduler.h"

static uint32_t scheduledEvents;

/**
 * @brief Adds an event to the scheduler.
 *
 * This function adds an event to the scheduler by performing a bitwise OR operation
 * on a global `scheduledEvents` variable with the provided `Event`. Each event is
 * represented by a single bit set in a 32-bit unsigned integer, allowing for a maximum
 * of 32 unique events to be scheduled.
 *
 * @param Event A 32-bit unsigned integer representing the event to add. The event should
 *              be specified by setting the corresponding bit to 1 (e.g., `1 << eventNumber`).
 * @note This function assumes that there is a global `scheduledEvents` variable which
 *       stores the current state of all scheduled events. It modifies this global variable
 *       directly. Care should be taken to ensure thread safety if used in a multi-threaded environment.
 *
 * Example usage:
 * @code
 *     addSchedulerEvent(1 << 5); // Adds the event represented by the 5th bit
 * @endcode
 */
void addSchedulerEvent(uint32_t Event) {

	scheduledEvents |= Event;

}

/**
 * @brief Removes an event from the scheduler.
 *
 * This function removes a specified event from the scheduler by performing a bitwise AND operation
 * with the negation of the provided `Event` on a global `scheduledEvents` variable. Each event is
 * represented by a single bit in a 32-bit unsigned integer.
 *
 * @param Event A 32-bit unsigned integer representing the event to remove. The event should be
 *              specified by setting the corresponding bit to 1 (e.g., `1 << eventNumber`).
 * @note This function assumes that there is a global `scheduledEvents` variable which
 *       stores the current state of all scheduled events. It modifies this global variable
 *       directly. Ensure that the event number passed to the function is within the range of
 *       available events (0 to 31).
 *
 * Example usage:
 * @code
 *     removeSchedulerEvent(1 << 5); // Removes the event represented by the 5th bit
 * @endcode
 */
void removeSchedulerEvent(uint32_t Event) {

	scheduledEvents &= ~Event;

}

/**
 * @brief Retrieves the currently scheduled events.
 *
 * This function returns the current state of scheduled events. Each bit in the returned 32-bit
 * unsigned integer represents a different event, where a set bit (1) indicates that the event is scheduled.
 *
 * @return A 32-bit unsigned integer representing the currently scheduled events. Each bit corresponds
 *         to a different event, with a 1 indicating the event is scheduled and a 0 indicating it is not.
 * @note This function relies on a global variable `scheduledEvents` which tracks all scheduled events.
 *       It is important that events are managed consistently to ensure that this function returns accurate information.
 *
 * Example usage:
 * @code
 *     uint32_t currentEvents = getScheduledEvents();
 *     if (currentEvents & (1 << 5)) {
 *         // Check if the event represented by the 5th bit is scheduled
 *     }
 * @endcode
 */
uint32_t getScheduledEvents() {
	return scheduledEvents;
}

