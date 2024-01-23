/*
 * ApplicationCode.c
 *
 *  Created on: Nov 14, 2023
 *      Author: xcowa
 */

#include "ApplicationCode.h"

#define INTERRUPT_CONTROL_EXT0_IQR_NUM 6

/*
 * Initialize all the periferals and clear the screen
 *
 *
 *return void
 */
void ApplicationInit(void) {

	Button_Interrupt_Init(); // initialize button
	LTCD__Init(); // initialize LCD
	Init_RNG(); // initialzie RNG
	LTCD_Layer_Init(0);
	LCD_Clear(0, LCD_COLOR_WHITE); // clear LCD

}

/*
 * Initialize button in interrupt mode
 *
 *
 *return void
 */

void Button_Interrupt_Init() {
	Init_Interrupt_Mode();

}

/*
 * Runs the demo for the LCD screen
 *
 *
 *return void
 */
void RunDemoForLCD(void) {
	QuickDemo();
}

/*
 * Demo function used to test LCD Screen
 *
 *
 *return void
 */
void RunLCD(void) {

	LCD_Display(LCD_LEVEL_1);

}

/*
 * This gets a random number from the RNG and returns it

 *return uint32_t num
 */
uint32_t GetRNG(void) {

	uint32_t num = getRNG();
	return num;
}

/*
 * this gets the current tick of the clock
 * return is in milliseconds from clock start
 *
 *return uint32_t tick
 */
uint32_t GetTick(void) {

	uint32_t tick = getTick();
	return tick;
}

/*
 *this gets the difference between two tick of the clock
 *this return is already in milliseconds
 *
 * Parameters
 *
 * uint32_t Tick1
 * uint32_t Tick2
 *
 *return uint32_t time
 */
uint32_t GetTime(uint32_t Tick1, uint32_t Tick2) {

	uint32_t time = getTime(Tick1, Tick2);
	return time;
}

/*
 *This uses the LCD screen function to display the time
 *
 * Parameters
 *
 * uint32_t Time
 *
 *return void
 */
void DisplayTime(uint32_t Time) {

	LCD_Clear(0, LCD_COLOR_WHITE);
	LCD_DisplayTime(Time);
}

/*
 *This uses the LCD screen function to display the correct level animation
 *
 * Parameters
 *
 * uint8_t Level
 *
 *return void
 */
void Display_LVL(uint8_t Level) {
	LCD_Display(Level);
}

/*
 *This uses the LCD screen function to display the AVG time
 *
 * Parameters
 *
 * uint32_t Time
 *
 *return void
 */
void Display_AVG_Time(uint32_t Time) {
	LCD_Clear(0, LCD_COLOR_WHITE);
	LCD_Display_AVG_Time(Time);
}

/*
 *Displays first part of level 3 animation
 *
 *
 *return void
 */
void Display_Lvl3_Stg1() {
	LCD_Display_Lvl3_Stg1();
}

/*
 *Displays second part of level 3 animation
 *
 *
 *return void
 */
void Display_Lvl3_Stg2() {
	LCD_Display_Lvl3_Stg2();
}

/*
 *Displays third part of level 3 animation
 *
 *
 *return void
 */
void Display_Lvl3_Stg3() {
	LCD_Display_Lvl3_Stg3();
}

/*
 *Displays start screen
 *
 *
 *return void
 */
void Start_Screen() {
	LCD_Start_Screen();
}

/*
 *Displays end screen
 *
 * Parameters
 *
 * uint32_t Tim1
 * uint32_t Tim2
 * uint32_t Tim3
 *
 *return void
 */
void End_Screen(uint32_t Tim1, uint32_t Tim2, uint32_t Tim3) {

	LCD_End_Screen(Tim1, Tim2, Tim3);
}

/*
 * Clears the LCD Screen
 *
 *return void
 */
void LCD_clear(void) {
	LCD_Clear(0, LCD_COLOR_WHITE);
}

/*
 * Button interrupt handler
 *
 * Adds a button pressed event to the scheduler.
 *
 *return void
 */

void EXTI0_IRQHandler() {
	HAL_NVIC_DisableIRQ(INTERRUPT_CONTROL_EXT0_IQR_NUM);

	addSchedulerEvent(SCHEDULER_BUTTON_PRESSED);

	//throw a universal flag that the button has been pressed ?????
//	Button_Flag = 1;

	__HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_0);

	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}
