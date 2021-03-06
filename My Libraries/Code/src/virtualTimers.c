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

		for (uint8_t i = 0; i <= MaxGTimers; i++){

				if (GTimerState[i] == TimerRunning){

					GTimerVal[i]++;
				}
			}

	if (ReceptionStatus == ReceptionEnabled){

		MRTUcount++;

		if (MRTUcount >= 5){
			SendMessage(ModbusRTUTimeOut, 0, 0);
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

void ResetGTimerVal (uint8_t GTimerID){
	if (GTimerState[GTimerID] == TimerStopped){
			GTimerVal[GTimerID] = 0;
	}
}
