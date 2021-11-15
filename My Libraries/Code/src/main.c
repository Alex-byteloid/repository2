
/*********************** Description ************************/



/************************* Includes *************************/

#include "main.h"

/********************* Global Variables *********************/



/*************************	 Code	*************************/

int main (void){

	InitGPIO();
	InitRCC();
	InitGTimer();
	UpdateNVICActiveAndPriority();
	InitHardwareTimer();
	InitMessage();
	InitModbusFSM(Baud9600, ParityControlOff, TwoStopBit, RTUMode);
	GPIOC->BSRR |= GPIO_BSRR_BR14;
//	SendMessage(LedOnMsg);

	while(1){

		ProcessSlaveModbusMessageReceptionRTUFSM();
		ProcessMessageGenerationSlaveModbusRTUFSM();
//		ProcessLedFSM();
		ProcessMessage();

	}

}
