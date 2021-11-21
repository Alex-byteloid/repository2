/*********************** Description ************************/

/*
 * Концепт пока такой:
 * Глобальный таймер - значение которого используется в нескольких функциях
 * Локальный таймер - значение котоорого используется строго в одной функции
 * */

/************************* Includes *************************/

#include "virtualTimers.h"

/********************* Global Variables *********************/

uint8_t GTimerState [MaxGTimers];				// Хранение текущих состояний глобальных таймеров
uint32_t GTimerVal	[MaxGTimers];				// Хранение текущих значений глобальных таймеров

uint8_t LocTimerState [MaxLocTimers];			// Хранение текущих состояний локальных таймеров
uint32_t LocTimerVal [MaxLocTimers];			// Хранение текущих значений локальных таймеров

uint16_t MRTUcount;
uint16_t ReceptionStatus;


/*************************	 Code	*************************/

/*************************	 Функции аппаратной инициализации таймера	*************************/

void InitTIM10 (void){

	RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;

	TIM10->PSC = 96 - 1;

	TIM10->ARR = 500;

	TIM10->DIER |= TIM_DIER_UIE;

	MRTUcount = 0;
	ReceptionStatus = ReceptionStopped;

	TIM10->CR1 |= TIM_CR1_CEN;

	TIM10->EGR |= TIM_EGR_UG;

}

void TIM1_UP_TIM10_IRQHandler (void){

	if (MaxGTimers != 0){

		for (uint8_t i = 0; i <= MaxGTimers; i++){

				if (GTimerState[i] == TimerRunning){

					GTimerVal[i]++;
				}
			}
	}

	if (MaxLocTimers != 0){

		for (uint8_t i = 0; i < MaxLocTimers; i++){

			if (LocTimerState[i] == TimerRunning){

				LocTimerVal[i]++;
			}
		}
	}



	if (ReceptionStatus == ReceptionEnabled){

		MRTUcount++;

		if (MRTUcount >= 5){
			SendMessage(ModbusRTUTimeOut);
			ReceptionStatus = ReceptionStopped;
		}

	}

	TIM10->SR &= ~TIM_SR_UIF;

}

/*************************	 Функции глобальных таймеров	*************************/

void InitGTimer(void){

	for (uint8_t i = 0; i < MaxGTimers; i++){
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


/*************************	 Функции локальных таймеров	*************************/

void InitLocTimer (void){

	for (uint8_t f = 0; f < MaxLocTimers; f++){

		LocTimerState [f] = TimerStopped;
	}
}

void StartLocTimer (uint8_t LocTimID){

	if (LocTimerState[LocTimID] == TimerStopped){

		LocTimerVal[LocTimID] = 0;
		LocTimerState[LocTimID] = TimerRunning;
	}
}

void StopLocTimer (uint8_t LocTimID){

	LocTimerState[LocTimID] = TimerStopped;
}

void PauseLocTimer (uint8_t LocTimID){

	if (LocTimerState[LocTimID] == TimerRunning){

		LocTimerState[LocTimID] = TimerPaused;
	}
}

void ReleaseLocTimer (uint8_t LocTimID){

	if (LocTimerState[LocTimID] == TimerPaused){
		LocTimerState[LocTimID] = TimerRunning;
	}
}

uint32_t GetLocTimerVal (uint8_t LocTimID){

	return LocTimerVal[LocTimID];
}

