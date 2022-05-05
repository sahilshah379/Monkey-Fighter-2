// main.cpp
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10 in C++

// Last Modified: 1/17/2020 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2017

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2017

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include <stdlib.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "SlidePot.h"
#include "Images.h"
#include "DAC.h"
#include "Sound.h"
#include "UART.h"
#include "Timer0.h"
#include "Timer1.h"
#include "Button.h"
#include "ImageTools.h"
#include "Joystick.h"

#define SCREEN_HEIGHT 160
#define SCREEN_WIDTH 128
#define MONKEY_WIDTH 22
#define MONKEY_ATTACK_WIDTH 27
#define MONKEY_HEIGHT 19
#define GROUND_LEVEL 160-31
#define PLAYER1_PROTECT_POSITION 37
#define PLAYER2_PROTECT_POSITION 90

SlidePot my(189,8);

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);

typedef enum {left,right,neutral} direction_t;
typedef enum {left_winner,right_winner, progress} gamestatus_t;
typedef enum {english, spanish} language_t;
typedef struct {
  int32_t x;      // x coordinate
  int32_t y;      // y coordinate
	direction_t direction; // left or right
	bool alive;
	bool attack;
	bool jump;
	bool stun;
	bool push;
	bool move;
	bool boundary;
} player_t;
typedef struct {
  int32_t x;      // x coordinate
  int32_t y;      // y coordinate
	bool collided;
  const unsigned short **image; // ptr->image
} coconut_t;

gamestatus_t gamestatus = progress;
direction_t attacker = neutral;
uint8_t stage = 3;
player_t player1, player2;
language_t language;

//coconut_t coconut1 = {100, 100, false, SmallEnemy20pointB};
//coconut_t coconut2 = {100, 100, false, SmallEnemy20pointB};

void Init(void) {
	gamestatus = progress;
	attacker = neutral;
	stage = 3;
	
	player1.x = 40;
	player1.y = GROUND_LEVEL;
	player1.direction = right;
	player1.alive = true;
	player1.attack = false;
	player1.jump = false;
	player1.stun = false;
	player1.push = false;
	player1.move = false;
	player1.boundary = false;

	player2.x = 70;
	player2.y = GROUND_LEVEL;
	player2.direction = left;
	player2.alive = true;
	player2.attack = false;
	player2.jump = false;
	player2.stun = false;
	player2.push = false;
	player2.move = false;
	player2.boundary = false;
}

const uint32_t push_duration = 20;
const uint32_t stun_duration = 25;
uint32_t push_progression_1 = 0;
uint32_t push_progression_2 = 0;
uint32_t stun_progression_1 = 0;
uint32_t stun_progression_2 = 0;
uint32_t push_amount = 7;

void checkCollisions() { // checks collision between player 1 and player 2 and adjusts
	
	int x1 = (player1.direction == left && player1.attack) ? player1.x-(MONKEY_ATTACK_WIDTH-MONKEY_WIDTH) : player1.x;
	int x2 = (player2.direction == left && player2.attack) ? player2.x-(MONKEY_ATTACK_WIDTH-MONKEY_WIDTH) : player2.x;
	
	if (abs(x1-x2) < MONKEY_WIDTH && abs(player1.y-player2.y) < MONKEY_HEIGHT) {
		if (player1.attack && !player2.attack && ((x1 < x2 && player1.direction == right && player2.direction == right) || (x1 > x2 && player1.direction == left && player2.direction == left))) {
			attacker = left; // player2 die
			player2.alive = false;
		} else if (player2.attack && !player1.attack && ((x1 < x2 && player1.direction == left && player2.direction == left) || (x1 > x2 && player1.direction == right && player2.direction == right))) {
			attacker = right; // player1 die
			player1.alive = false;
		} else if (!player2.push && player1.attack && !player2.attack && ((x1 < x2 && player1.direction == right && player2.direction == left) || (x1 > x2 && player1.direction == left && player2.direction == right))) {
			if (x1 < x2 && player1.direction == right && player2.direction == left) player2.x += 7; // player2 pushed back
			else player2.x -= push_amount;
			playSound(SWORD);
			player2.push = true;
		} else if (!player1.push && player2.attack && !player1.attack && ((x1 < x2 && player1.direction == right && player2.direction == left) || (x1 > x2 && player1.direction == left && player2.direction == right))) {
			if (x1 < x2 && player1.direction == right && player2.direction == left) player1.x -= 7;
			else player1.x += push_amount; // player1 pushed back
			playSound(SWORD);
			player1.push = true;
		} else if (!player1.push && !player2.push && player2.attack && player1.attack && ((x1 < x2 && player1.direction == right && player2.direction == left) || (x1 > x2 && player1.direction == left && player2.direction == right))) {
			if (x1 < x2 && player1.direction == right && player2.direction == left) player2.x += 7; // player1 and 2 pushed back
			else player2.x -= push_amount;
			if (x1 < x2 && player1.direction == right && player2.direction == left) player1.x -= 7;
			else player1.x += push_amount;
			playSound(SWORD);
			player1.push = true;
			player2.push = true;
		} else {
			// check direction
			if (player1.y < player2.y+10) { // above so stun
				player2.stun = true;
			} else if (player2.y < player1.y+10) {
				player1.stun = true;
			}
			if (player1.boundary && player2.boundary) {
				if (x1 <= SCREEN_WIDTH/2) { // left boundary
					if (player1.y < player2.y) player1.x = player2.x+MONKEY_WIDTH;
					else player2.x = player1.x+MONKEY_WIDTH;
				} else {
					if (player1.y < player2.y) player1.x = player2.x-MONKEY_WIDTH;
					else player2.x = player1.x-MONKEY_WIDTH;
				}
			} else if (x1 > x2) { // player1 is to the right
				if ((player1.move && !player2.move) || (player2.boundary && player1.move && player2.move)) {
					player1.x = player2.x+MONKEY_WIDTH;
				} else if ((!player1.move && player2.move) || (player1.boundary && player1.move && player2.move)) {
					player2.x = player1.x-MONKEY_WIDTH;
				} else {
					int old_p1 = player1.x;
					player1.x = player2.x+MONKEY_WIDTH;
					player2.x = old_p1-MONKEY_WIDTH;
				}
			} else { // player2 is to the right
				if ((player1.move && !player2.move) || (player2.boundary && player1.move && player2.move)) {
					player1.x = player2.x-MONKEY_WIDTH;
				} else if ((!player1.move && player2.move) || (player1.boundary && player1.move && player2.move)) {
					player2.x = player1.x+MONKEY_WIDTH;
				} else {
					int old_p1 = player1.x;
					player1.x = player2.x-MONKEY_WIDTH;
					player2.x = old_p1+MONKEY_WIDTH;
				}
			}
		}
	}
	if (push_progression_1 >= push_duration) {
		player1.push = false;
		push_progression_1 = 0;
	} else {
		push_progression_1++;
	}
	if (push_progression_2 >= push_duration) {
		player2.push = false;
		push_progression_2 = 0;
	} else {
		push_progression_2++;
	}
	
	if (stun_progression_1 >= stun_duration) {
		player1.stun = false;
		stun_progression_1 = 0;
	} else {
		stun_progression_1++;
	}
	if (stun_progression_2 >= stun_duration) {
		player2.stun = false;
		stun_progression_2 = 0;
	} else {
		stun_progression_2++;
	}
}

bool Semaphore = 1; // 1 if things have changed

const uint32_t jump_duration = 40;
const uint32_t attack_duration = 10;
const uint32_t death_duration = 50;

uint32_t last_jump_1 = 0;
uint32_t last_attack_1 = 0;
uint32_t jump_progression_1 = 0;
uint32_t attack_progression_1 = 0;
uint32_t death_progression_1 = 0;
double k = -MONKEY_HEIGHT*2.5; // pixels jumped
double y1 = GROUND_LEVEL; // initial height
double h = jump_duration/2.0;

void movePlayer1(void) { // 50 Hz from ISR
	// left and right movement
	
	if (!player1.alive) {
		death_progression_1++;
		if (death_progression_1 >= death_duration) {
			death_progression_1 = 0;
			player1.x = 70;
			player1.y = GROUND_LEVEL;
			player1.direction = right;
			player1.alive = true;
			Semaphore = 1;
		} else {
			return;
		}
	}
	
	uint32_t in = Button_In_Player1();
	uint32_t joystickData[2];
	Joystick_In(joystickData);
	if (joystickData[0] <= 600 && !player1.push && !player1.stun) {
		player1.x+=2;
		player1.direction = right;
		player1.move = true;
		Semaphore = 1;
 	} else if (joystickData[0] >= 3495 && !player1.push && !player1.stun) {
		player1.x-=2;
		player1.direction = left;
		player1.move = true;
		Semaphore = 1;
	} else {
		player1.move = false;
	}
	
	// attack
	if (last_attack_1 != 0x02) {
		 if (in==0x02 && !player1.push && !player1.stun) {
			Semaphore = 1;
			last_attack_1 = 0x02;
			player1.attack = true;
			playSound(ATTACK);
		}
	} else {
		Semaphore = 1;
		if (attack_progression_1 >= attack_duration) {
			last_attack_1 = 0;
			attack_progression_1 = 0;
			player1.attack = false;
		} else {
			attack_progression_1++;
		}
	}
	
	// jump
	if (last_jump_1 != 0x08) {
		 if (in==0x08 && !player1.push && !player1.stun) {
			Semaphore = 1;
			last_jump_1 = 0x08;
			player1.jump = true;
			playSound(JUMP);
		}
	} else {
		Semaphore = 1;
		if (jump_progression_1 >= jump_duration) {
			last_jump_1 = 0;
			player1.y = y1;
			jump_progression_1 = 0;
			player1.jump = false;
		} else {
			player1.y = y1+(-1*k/(h*h))*(jump_progression_1-h)*(jump_progression_1-h)+k;
			jump_progression_1++;
		}
	}	
}

uint32_t last_jump_2 = 0;
uint32_t last_attack_2 = 0;
uint32_t jump_progression_2 = 0;
uint32_t attack_progression_2 = 0;
uint32_t death_progression_2 = 0;

void movePlayer2(void) { // 50 Hz from ISR
	// left and right movement
	
	if (!player2.alive) {
		death_progression_2++;
		if (death_progression_2 >= death_duration) {
			death_progression_2 = 0;
			player2.x = 57;
			player2.y = GROUND_LEVEL;
			player2.alive = true;
			player2.direction = left;
			Semaphore = 1;
		} else {
			return;
		}
	}
	
	uint32_t in = Button_In_Player2();
	my.Save(ADC_In());
	uint32_t temp = my.Distance(); // 653 1307
	
	if (temp <= 353 && !player2.push && !player2.stun) {
		player2.x+=2;
		player2.direction = right;
		player2.move = true;
		Semaphore = 1;
 	} else if (temp >= 1607 && !player2.push && !player2.stun) {
		player2.x-=2;
		player2.direction = left;
		player2.move = true;
		Semaphore = 1;
	} else {
		player2.move = false;
	}
	
	// 0100 0000 or 1000 0000 - 0x40 and 0x80
	
	// attack
	if (last_attack_2 != 0x01) {
		 if (in==0x01 && !player2.push && !player2.stun) {
			Semaphore = 1;
			last_attack_2 = 0x01;
			player2.attack = true;
			playSound(ATTACK);
		}
	} else {
		Semaphore = 1;
		if (attack_progression_2 >= attack_duration) {
			last_attack_2 = 0;
			attack_progression_2 = 0;
			player2.attack = false;
		} else {
			attack_progression_2++;
		}
	}
	
	// jump
	if (last_jump_2 != 0x04) {
		 if (in==0x04 && !player2.push && !player2.stun) {
			Semaphore = 1;
			last_jump_2 = 0x04;
			player2.jump = true;
			playSound(JUMP);
		}
	} else {
		Semaphore = 1;
		if (jump_progression_2 >= jump_duration) {
			last_jump_2 = 0;
			player2.y = y1;
			jump_progression_2 = 0;
			player1.jump = false;
		} else {
			player2.y = y1+(-1*k/(h*h))*(jump_progression_2-h)*(jump_progression_2-h)+k;
			jump_progression_2++;
		}
	}	
}


void Move(void) {
	movePlayer1();
	movePlayer2();

	player1.boundary = false;
	player2.boundary = false;
	// screen advance and boundary conditions
	uint32_t right_boundary = 127-MONKEY_WIDTH-1;
	uint32_t left_boundary = 3;
	if (attacker == neutral) {
		if (player1.x >= right_boundary) {
			player1.x = right_boundary;
			player1.boundary = true;
		} else if (player1.x <= left_boundary) {
			player1.x = left_boundary;
			player1.boundary = true;
		}
		if (player2.x >= right_boundary) {
			player2.x = right_boundary;
			player2.boundary = true;
		} else if (player2.x <= left_boundary) {
			player2.x = left_boundary;
			player2.boundary = true;
		}
	} else if (attacker == left) {
		if (player1.x >= right_boundary) { // player1 advances right
			player1.x = left_boundary;
			player2.x = PLAYER2_PROTECT_POSITION;
			stage++;
			if (death_progression_2 < death_duration) {
				death_progression_2 = 0;
				player2.x = 57;
				player2.y = GROUND_LEVEL;
				player2.alive = true;
				Semaphore = 1;
			}
			
		} else if (player1.x <= left_boundary) {
			player1.x = left_boundary;
			player1.boundary = true;
		}
		if (player2.x >= right_boundary) {
			player2.x = right_boundary;
			player2.boundary = true;
		} else if (player2.x <= left_boundary) {
			player2.x = left_boundary;
			player2.boundary = true;
		}
	} else if (attacker == right) {
		if (player1.x >= right_boundary) {
			player1.x = right_boundary;
			player1.boundary = true;
		} else if (player1.x <= left_boundary) {
			player1.x = left_boundary;
			player1.boundary = true;
		}
		if (player2.x >= right_boundary) {
			player2.x = right_boundary;
			player2.boundary = true;
		} else if (player2.x <= left_boundary) { // player2 advances left
			player1.x = PLAYER1_PROTECT_POSITION;
			player2.x = right_boundary;
			stage--;
			if (death_progression_1 < death_duration) {
				death_progression_1 = 0;
				player1.x = 70;
				player1.y = GROUND_LEVEL;
				player1.alive = true;
				Semaphore = 1;
			}
		}
	}
	if (player1.alive && player2.alive) checkCollisions();

	if (stage == 0) player1.alive = false;
	if (stage == 6) player2.alive = false;
}



void Draw() { // 30 Hz from main
	
	if (stage == 0) ST7735_DrawBitmap(0,159,marsbg,127,159);	
	else if (stage == 1) ST7735_DrawBitmap(0,159,desertbg,127,159);	
	else if (stage == 2 || stage == 3 || stage == 4) ST7735_DrawBitmap(0,159,jungle_backdrop,128,160);
	else if (stage == 5) ST7735_DrawBitmap(0,159,purpleforestbg,127,159);	
	else if (stage == 6) ST7735_DrawBitmap(0,159,factorybg,127,159);	
	
	if (stage == 0) drawTransparentImageBanana(banana, 53, GROUND_LEVEL, 16, 15);
	else if (stage == 6) drawTransparentImageBanana(banana, 74, GROUND_LEVEL, 16, 15);
	
	if (attacker == left && stage <= 5) {
		drawTransparentImageArrow(arrow_right, 100, 20, 24, 17);
	} else if (attacker == right && stage >= 1) {
		drawTransparentImageArrow(arrow_left, 7, 20, 24, 17);
	}
	
	if (stage >= 1 && player1.alive) {
		if (player1.direction == left) {
			int offset_x = player1.x-(MONKEY_ATTACK_WIDTH-MONKEY_WIDTH);
			if (player1.jump && player1.attack) {
			drawTransparentImageAttack(monkey_1_jumpattack_left, offset_x, player1.y, MONKEY_ATTACK_WIDTH, MONKEY_HEIGHT);
			} else if (player1.attack) {
			drawTransparentImageAttack(monkey_1_attack_left, offset_x, player1.y, MONKEY_ATTACK_WIDTH, MONKEY_HEIGHT);
			} else if (player1.jump) {
				drawTransparentImage(monkey_1_jump_left, player1.x, player1.y, MONKEY_WIDTH, MONKEY_HEIGHT);
			} else {
				drawTransparentImage(monkey_1_idle_left, player1.x, player1.y, MONKEY_WIDTH, MONKEY_HEIGHT);
			}
		} else {
			if (player1.jump && player1.attack) {
			drawTransparentImageAttack(monkey_1_jumpattack_right, player1.x, player1.y, MONKEY_ATTACK_WIDTH, MONKEY_HEIGHT);
			} else if (player1.attack) {
			drawTransparentImageAttack(monkey_1_attack_right, player1.x, player1.y, MONKEY_ATTACK_WIDTH, MONKEY_HEIGHT);
			} else if (player1.jump) {
				drawTransparentImage(monkey_1_jump_right, player1.x, player1.y, MONKEY_WIDTH, MONKEY_HEIGHT);
			} else {
				drawTransparentImage(monkey_1_idle_right, player1.x, player1.y, MONKEY_WIDTH, MONKEY_HEIGHT);
			}
		}
		
	}
	if (stage <= 5 && player2.alive) {
		if (player2.direction == left) {
			int offset_x = player2.x-(MONKEY_ATTACK_WIDTH-MONKEY_WIDTH);
			if (player2.jump && player2.attack) {
				drawTransparentImageAttack(monkey_2_jumpattack_left, offset_x, player2.y, MONKEY_ATTACK_WIDTH, MONKEY_HEIGHT);
			} else if (player2.attack) {
			drawTransparentImageAttack(monkey_2_attack_left, offset_x, player2.y, MONKEY_ATTACK_WIDTH, MONKEY_HEIGHT);
			} else if (player2.jump) {
				drawTransparentImage(monkey_2_jump_left, player2.x, player2.y, MONKEY_WIDTH, MONKEY_HEIGHT);
			} else {
				drawTransparentImage(monkey_2_idle_left, player2.x, player2.y, MONKEY_WIDTH, MONKEY_HEIGHT);
			}
		} else {			
			if (player2.jump && player2.attack) {
				drawTransparentImageAttack(monkey_2_jumpattack_right, player2.x, player2.y, MONKEY_ATTACK_WIDTH, MONKEY_HEIGHT);
			} else if (player2.attack) {
			drawTransparentImageAttack(monkey_2_attack_right, player2.x, player2.y, MONKEY_ATTACK_WIDTH, MONKEY_HEIGHT);
			} else if (player2.jump) {
				drawTransparentImage(monkey_2_jump_right, player2.x, player2.y, MONKEY_WIDTH, MONKEY_HEIGHT);
			} else {
				drawTransparentImage(monkey_2_idle_right, player2.x, player2.y, MONKEY_WIDTH, MONKEY_HEIGHT);
			}
		}
	}
}


void Menu() {
	
	ST7735_DrawBitmap(0,159,jungle_backdrop,128,160);	
	drawTransparentImageTitle(title, 22, 50, 83, 42);
	ST7735_SetTextColor(0xFFFF);
	
	while (Button_In_Player1() != 0x02) { }
	while (Button_In_Player1() == 0x02) { }
	
	ST7735_DrawBitmap(0,159,jungle_backdrop,128,160);	
	
	ST7735_SetCursor(1, 1);
	ST7735_OutString((char*)"Pick a Language");
	
	ST7735_SetCursor(1, 2);
	ST7735_OutString((char*)"Elige un idioma");
	
	uint32_t temp;
	while (Button_In_Player1() != 0x02) {
		my.Save(ADC_In());
		temp = my.Distance();
		ST7735_SetCursor(1, 4);
		if (temp >= 980) {
				ST7735_OutString((char*)">English    Espa\xA4ol");
		} else {
				ST7735_OutString((char*)" English   >Espa\xA4ol");
		}
	}
  
	if (temp >= 980) {
		language = english;
	} else {
		language = spanish;
	}
}

void EndScreen() {
	if (gamestatus == left_winner) {
		ST7735_DrawBitmap(0,159,factorybg,127,159);	
	} else {
		ST7735_DrawBitmap(0,159,marsbg,127,159);	
	}
	if (language == english) {
		ST7735_SetCursor(1, 1);
		ST7735_OutString((char*)"GAME OVER");
		ST7735_SetCursor(1, 2);
		ST7735_SetTextColor(ST7735_WHITE);
		if (gamestatus == left_winner) {
			ST7735_OutString((char*)"Player 1 Wins!");
		} else {
			ST7735_OutString((char*)"Player 2 Wins!");
		}
	} else if (language == spanish) {
		ST7735_SetCursor(1, 1);
		ST7735_OutString((char*)"FIN DE LA PARTIDA");
		ST7735_SetCursor(1, 2);
		ST7735_SetTextColor(ST7735_WHITE);
		if (gamestatus == left_winner) {
			ST7735_OutString((char*)"El Jugador 1 Gana!");
		} else {
			ST7735_OutString((char*)"El Jugador 2 Gana!");
		}
	}
}

int main(void){
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  // TExaS_Init();
  Random_Init(1);
  //Output_Init();
	Button_Init();
	ADC_Init();
	Joystick_Init();
	DAC_Init();
	//Joystick_Init_Player2();
	
	//SysTick_Init(80000000/10);
	ST7735_InitR(INITR_REDTAB);
  ST7735_FillScreen(0);
	
	Menu();
	
  Timer0_Init(&Move,80000000/50); // 50 Hz
//  Timer1_Init(&Coconut,80000000); // 1 Hz
  EnableInterrupts();
	
	Init();

	
  while(gamestatus == progress){
    while(Semaphore==0){};
    Semaphore = 0;
		//ST7735_FillScreen(0);
		Draw(); // 50 Hz
			
		// check win conditions
		if (stage == 6 && player1.x >= 58) {
			gamestatus = left_winner;
		} else if (stage == 0 && player2.x <= 69) {
			gamestatus = right_winner;
		}
  }
	
  EndScreen();
}

