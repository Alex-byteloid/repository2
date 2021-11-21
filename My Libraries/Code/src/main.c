
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
	InitLocTimer();
	InitTIM10();
	InitMessage();
	InitModbusFSM(Baud9600, ParityControlOff, TwoStopBit, RTUMode);
	InitLedFSM();
	SendMessage(LedOnMsg);

	while(1){

		ProcessModbusSlaveFSM();
		ProcessLedFSM();
		ProcessMessage();

	}

}
