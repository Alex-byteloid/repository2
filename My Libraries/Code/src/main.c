
/*********************** Description ************************/



/************************* Includes *************************/

#include "main.h"

/********************* Global Variables *********************/

extern uint8_t GTimerState [MaxGTimers];
extern uint32_t GTimerVal	[MaxGTimers];

/*************************	 Code	*************************/

int main (void){

	InitGPIO();
	InitRCC();
	InitGTimer();
	InitHardwareTimer();
	InitLedFSM();
	InitMessage();
	SendMessage(LedOnMsg);

	while(1){

		ProcessLedFSM();
		ProcessMessage();

	}

}


// Обработчики прерываний

void SysTick_Handler(void){

	uint8_t i;

	for (i = 0; i < MaxGTimers; i++){

		if (GTimerState[i] == TimerRunning){
			GTimerVal[i]++;

		}

	}

}
