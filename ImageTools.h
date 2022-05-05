#ifndef IMAGE_TOOLS_H
#define IMAGE_TOOLS_H

#include <stdint.h>
#include "Images.h"
#include "ST7735.h"


void drawTransparentImage(const unsigned short im[][22], int x, int y, int w, int h) {
	int a = 0;
	for (int i = x; i < x+w; i++, a++) {
		int b = 0;
		for (int j = y-h+1; j <= y; j++, b++) {
			if (im[b][a] != 0xFFFF) {
				ST7735_DrawPixel(i, j, im[b][a]);
			}
		}
	}
}

void drawTransparentImageAttack(const unsigned short im[][27], int x, int y, int w, int h) {
	int a = 0;
	for (int i = x; i < x+w; i++, a++) {
		int b = 0;
		for (int j = y-h+1; j <= y; j++, b++) {
			if (im[b][a] != 0xFFFF) {
				ST7735_DrawPixel(i, j, im[b][a]);
			}
		}
	}
}

void drawTransparentImageCoconut(const unsigned short im[][11], int x, int y, int w, int h) {
	int a = 0;
	for (int i = x; i < x+w; i++, a++) {
		int b = 0;
		for (int j = y-h+1; j <= y; j++, b++) {
			if (im[b][a] != 0xFFFF) {
				ST7735_DrawPixel(i, j, im[b][a]);
			}
		}
	}
}

void drawTransparentImageArrow(const unsigned short im[][24], int x, int y, int w, int h) {
	int a = 0;
	for (int i = x; i < x+w; i++, a++) {
		int b = 0;
		for (int j = y-h+1; j <= y; j++, b++) {
			if (im[b][a] != 0xFFFF) {
				ST7735_DrawPixel(i, j, im[b][a]);
			}
		}
	}
}

void drawTransparentImageTitle(const unsigned short im[][83], int x, int y, int w, int h) {
	int a = 0;
	for (int i = x; i < x+w; i++, a++) {
		int b = 0;
		for (int j = y-h+1; j <= y; j++, b++) {
			if (im[b][a] != 0xFFFF) {
				ST7735_DrawPixel(i, j, im[b][a]);
			}
		}
	}
}

void drawTransparentImageBanana(const unsigned short im[][16], int x, int y, int w, int h) {
	int a = 0;
	for (int i = x; i < x+w; i++, a++) {
		int b = 0;
		for (int j = y-h+1; j <= y; j++, b++) {
			if (im[b][a] != 0xFFFF) {
				ST7735_DrawPixel(i, j, im[b][a]);
			}
		}
	}
}

#endif //IMAGE_TOOLS_H
