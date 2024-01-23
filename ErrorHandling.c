/*
 * ErrorHandling.c
 *
 *  Created on: Nov 7, 2023
 *      Author: aaron
 */


/**
 * @brief Asserts a condition in the application.
 *
 * This function is used for asserting a boolean condition. If the condition is false,
 * the function enters an infinite loop, effectively halting the program. This is typically
 * used for error handling or to ensure certain conditions are met during program execution.
 *
 * @param boi The condition to be checked. If this parameter is false, the function halts the program.
 * @note Use this function with caution as passing a false condition will cause the system to enter
 *       an infinite loop, which can only be exited by resetting the hardware or interrupting the power.
 *
 * Example usage:
 * @code
 *     _Bool condition = checkSomeCondition();
 *     APPLICATION_ASSERT(condition); // Halts the program if the condition is false
 * @endcode
 */

void APPLICATION_ASSERT(_Bool boi){
	while(!boi);
}
