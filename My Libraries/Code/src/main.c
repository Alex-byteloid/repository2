
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
	InitWS281xFSM();

	InitLedFSM();
	SendMessage(LedOnMsg, 0, 0);
	SendMessage(WS28LedStart, 0, 0);
	StartGTimer(LCDTimer);

	while(1){
		if (GetGTimerVal(LCDTimer) >= 6000){
			StopGTimer(LCDTimer);
			ResetGTimerVal(LCDTimer);
			SendMessage(WS28StartEffect, 0, 0);
		}
		ProcessLedFSM();
		ProcessWS281xFSM();
		ProcessEffectsWS281xFSM();
		ProcessMessage();

	}

}
