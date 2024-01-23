/*
 * Scheduler.h
 *
 *  Created on: Sep 5, 2023
 *      Author: aaron
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#define SCHEDULER_BUTTON_PRESSED 1 << 0
#define SCHEDULER_LEVEL_1 1 << 1
#define SCHEDULER_LEVEL_2 1 << 2
#define SCHEDULER_LEVEL_3 1 << 3
#define SCHEDULER_START_SCREEN 1 << 4
#define SCHEDULER_END_SCREEN 1 << 6


uint32_t getScheduledEvents();

void addSchedulerEvent(uint32_t Event);

void removeSchedulerEvent(uint32_t Event);

#endif /* SCHEDULER_H_ */
