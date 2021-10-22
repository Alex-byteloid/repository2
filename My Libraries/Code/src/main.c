
/*********************** Description ************************/



/************************* Includes *************************/

#include "main.h"

/********************* Global Variables *********************/

extern uint8_t GTimerState [MaxGTimers];
extern uint32_t GTimerVal	[MaxGTimers];

uint8_t state;				// Переменная состояния автомата
uint8_t _state;				// Переменная предыдущего состояния автомата
uint8_t entry;				// Флаг перехода в новое состояние

/*************************	 Code	*************************/

int main (void){

	InitGPIO();
	InitRCC();
	InitGTimer();
	InitHardwareTimer();
	InitLedFSM();

	while(1){

		ProcessLedFSM();

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
