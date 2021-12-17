
/*********************** Description ************************/


/************************* Includes *************************/

#include "main.h"

/********************* Global Variables *********************/



/*************************	 Code	*************************/

int main (void){

	InitGPIO();
	GPIOC->BSRR |= GPIO_BSRR_BR14;
	GPIOC->BSRR |= GPIO_BSRR_BR15;
	GPIOC->BSRR |= GPIO_BSRR_BS13;
	InitRCC();
	UpdateNVICActiveAndPriority();
	InitGTimer();
	InitTIM10();
	InitMessage();

	InitLedFSM();
	SendMessage(LedOnMsg, 0, 0);

	while(1){

		ProcessLedFSM();
		ProcessMessage();

	}

}
