/*********************** Description ************************/



/************************* Includes *************************/

#include "virtualTimers.h"

/********************* Global Variables *********************/

uint8_t GTimerState [MaxGTimers];				// Хранение текущих состояний глобальных таймеров
uint32_t GTimerVal	[MaxGTimers];				// Хранение текущих значений глобальных таймеров

uint8_t SysTickHandlerState;					// Переменная состаяния обработчика прерывания системного таймера для работы с Modbus

/*************************	 Code	*************************/

void InitHardwareTimer (void){

	SysTickHandlerState = 0;					// Обработчик прерывания системного таймера в состоянии 0

	SysTick->LOAD = 95999;						// Загрузка значения перезагрузки. При 96 МГц, данное занечение соотвествует прерыванию каждые 1 мс.
	SysTick->VAL = 95999;						// Обнуляем таймер и флаги.

	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
					SysTick_CTRL_TICKINT_Msk |
					SysTick_CTRL_ENABLE_Msk;

}

void SysTick_Handler(void){

	switch (SysTickHandlerState){

	case 0:
		for(uint8_t i = 0; i < MaxGTimers; i++){

			if (GTimerState[i] == TimerRunning){
				GTimerVal[i]++;
			}
		}
		break;

	case 1:
		SendMessage(ModbusRTUTimeOut);
		SysTickHandlerState = 0;
		break;

	}

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
