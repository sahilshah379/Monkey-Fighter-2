#include "Sound.h"
#include "DAC.h"
#include <stdint.h>

typedef struct {
	const unsigned char *wave;
	uint32_t length;
} sound_t;

int play;

sound_t sounds[5] ={
	{sword, 4550},
	{jump, 2035},
	{selection, 265},
	{attack, 4699},
	{clear,1},
	//{sahilmom, 31080}
};


extern void Timer1_Init(void(*task)(void), uint32_t period);
void Timer1A_Stop(void);


uint32_t pIndex=0;
void playSample() {
	DAC_Out(sounds[play].wave[pIndex++]);
   if (pIndex >= sounds[play].length){
     Timer1A_Stop();
     pIndex=0;
   }
}

void playSound(soundeffect_t sound) {
	if (sound == SWORD) play = 0;
	else if (sound == JUMP) play = 1;
	else if (sound == SELECTION) play = 2;
	else if (sound == ATTACK) play = 3;
	else if (sound == CLEAR) play = 4;
	else if (sound == DRIFT) play = 5;
	Timer1_Init(&playSample, 80000000/11025);
}
