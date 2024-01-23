/*
 * ApplicationCode.h
 *
 *  Created on: Nov 14, 2023
 *      Author: xcowa
 */

#ifndef INC_APPLICATIONCODE_H_
#define INC_APPLICATIONCODE_H_

#include "LCD_Driver.h"
#include "Button_Driver.h"
#include "Scheduler.h"
#include "RNG_Driver.h"
#include "ErrorHandling.h"
#include "Timer_Driver.h"



void ApplicationInit(void);
void RunDemoForLCD(void);

void RunLCD(void);

uint32_t GetRNG(void);

uint32_t GetTick(void);

uint32_t GetTime(uint32_t Tick1 , uint32_t Tick2);

void DisplayTime(uint32_t Time);

void LCD_clear(void);

void Display_AVG_Time(uint32_t Time);

void Display_LVL(uint8_t Level);

void Display_Lvl3_Stg1();

void Display_Lvl3_Stg2();

void Display_Lvl3_Stg3();

void Start_Screen();

void End_Screen(uint32_t Tim1, uint32_t Tim2, uint32_t Tim3);


void Button_Interrupt_Init();

#endif /* INC_APPLICATIONCODE_H_ */
