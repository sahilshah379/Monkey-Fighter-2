// FIFO.cpp
// Runs on any microcontroller
// Provide functions that initialize a FIFO, put data in, get data out,
// and return the current size.  The file includes a transmit FIFO
// using index implementation and a receive FIFO using pointer
// implementation.  Other index or pointer implementation FIFOs can be
// created using the macros supplied at the end of the file.
// Modified: 1/12/2022
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly


/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to the Arm Cortex M3",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2011
   Programs 3.7, 3.8., 3.9 and 3.10 in Section 3.7

 Copyright 2019 by Jonathan W. Valvano, valvano@mail.utexas.edu
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
#include <stdint.h>

#include "FIFO.h"
#include "ST7735.h"
#include "print.h"

// A class named Queue that defines a FIFO
Queue::Queue(){
  // Constructor - make FIFO initially empty
  // write this
	this->PutI = FIFOSIZE-1;
	this->GetI = FIFOSIZE-1;
	this->count = 0;
}

// To check whether Queue is empty or not
bool Queue::IsEmpty(void){
  // write this
  if (this->count == 0) {
		return true;
	}
	return false;
}

  // To check whether Queue is full or not
bool Queue::IsFull(void){
  // write this
  if (this->count == FIFOSIZE) {
		return true;
	}
	return false;
}

  // Inserts an element in queue at rear end
bool Queue::Put(char x){
  // write this
  if (this->IsFull()) {
		return false;
	}
	this->Buf[PutI] = x;
	this->PutI -= 1;
	if (PutI < 0) PutI = FIFOSIZE - 1;
	this->count++;
	
	return true;
}

  // Removes an element in Queue from front end. 
bool Queue::Get(char *pt){
  // write this
  if (this->IsEmpty()) {
		return false;
	} 
	*pt = this->Buf[GetI];
	this->GetI -= 1;
	if (GetI < 0) GetI = FIFOSIZE-1;
	this->count--;

	return true;
}

  /* 
     Printing the elements in queue from front to rear. 
     This function is only to test the code. 
     This is not a standard function for Queue implementation. 
  */
void Queue::Print(void){
  // write this
	int i = 0;
	while (i < this->count) {
		ST7735_OutChar(this->Buf[FIFOSIZE-1-i] + 0x30);
		ST7735_OutChar('\n');
		i++;
	}
}


