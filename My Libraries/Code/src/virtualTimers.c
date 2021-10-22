/*********************** Description ************************/



/************************* Includes *************************/

#include "virtualTimers.h"

/********************* Global Variables *********************/

uint8_t GTimerState [MaxGTimers];				// Хранение текущих состояний глобальных таймеров
uint32_t GTimerVal	[MaxGTimers];				// Хранение текущих значений глобальных таймеров

/*************************	 Code	*************************/

void InitHardwareTimer (void){

	SysTick->LOAD = 959999;						// Загрузка значения перезагрузки. При 96 МГц, данное занечение соотвествует прерыванию каждые 10 мс.
	SysTick->VAL = 959999;						// Обнуляем таймер и флаги.

	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
					SysTick_CTRL_TICKINT_Msk |
					SysTick_CTRL_ENABLE_Msk;

}

void InitGTimer(void){

	uint8_t i;

	for (i = 0; i < MaxGTimers; i++){
		GTimerState[i] = TimerStopped;
	}

}

void StartGTimer(uint8_t GTimerID){

	if (GTimerState[GTimerID] == TimerStopped){

		GTimerVal[GTimerID] = 0;
		GTimerState[GTimerID] = TimerRunning;
	}

}

void StopGTimer(uint8_t GTimerID){

	GTimerState[GTimerID] = TimerStopped;
}

void PauseGTimer(uint8_t GTimerID){

	if (GTimerState[GTimerID] == TimerRunning){

		GTimerState[GTimerID] = TimerPaused;
	}

}

void ReleaseGTimer(uint8_t GTimerID){

	if (GTimerState[GTimerID] == TimerPaused){
		GTimerState[GTimerID] = TimerRunning;

	}

}

uint32_t GetGTimerVal(uint8_t GTimerID){

	return GTimerVal[GTimerID];

}
