
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

	while(1){

		GPIOC->BSRR |= GPIO_BSRR_BR13;
		for(uint32_t i = 0; i < 10000000; i++);
		GPIOC->BSRR |= GPIO_BSRR_BS13;
		for(uint32_t d = 0; d < 10000000; d++);

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
