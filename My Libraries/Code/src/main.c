
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
	InitGTimer();
	UpdateNVICActiveAndPriority();
	InitHardwareTimer();
	InitMessage();
	InitModbusFSM(Baud9600, ParityControlOff, TwoStopBit, RTUMode);

	while(1){

		ProcessSlaveModbusMessageReceptionRTUFSM();
		ProcessMessageGenerationSlaveModbusRTUFSM();
		ProcessLedFSM();
		ProcessMessage();

	}

}
