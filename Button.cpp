// Button.cpp
// This software configures the off-board piano keys
// Lab 6 requires a minimum of 4 keys, but you could have more
// Runs on LM4F120 or TM4C123
// Program written by: put your names here
// Date Created: 3/6/17 
// Last Modified: 1/11/22  
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Button.h"

// **************Key_Init*********************
// Initialize button inputs on PE3-0
// Input: none 
// Output: none
void Button_Init(void){ volatile uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x10;
	while ((SYSCTL_PRGPIO_R&0x10) != 0x10) { };
	
	GPIO_PORTE_AMSEL_R &= ~0x0F; // PE3-0 input 0000 1111
	//GPIO_PORTE_PCTL_R &= ~0x0000FFFF;
	GPIO_PORTE_DIR_R &= ~0x0F;
	GPIO_PORTE_AFSEL_R &= ~0x0F;
	GPIO_PORTE_DEN_R |= 0x0F;
}
// **************Key_In*********************
// Input from piano key inputs on PA5-2 or PE3-0
// Input: none 
// Output: 0 to 15 depending on keys
//   0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2, 0x08 is just Key3
uint32_t Button_In_Player1(void){
	return GPIO_PORTE_DATA_R & 0x0000000A; // 1010
}
uint32_t Button_In_Player2(void){
	return GPIO_PORTE_DATA_R & 0x00000005; // 0101
}
