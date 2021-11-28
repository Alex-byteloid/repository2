
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
	InitModbusFSM(Baud9600, ParityControlOff, TwoStopBit, RTUMode);
	InitLedFSM();
	SendMessage(LedOnMsg);

	InitI2C1FSM();

	InitLcdFSM();

	SendMessage(LCDStartInit);

	while(1){

		ProcessI2CWriteFSM();
		ProcessLcdFSM();
		ProcessModbusSlaveFSM();
		ProcessLedFSM();
		ProcessMessage();

	}

}
