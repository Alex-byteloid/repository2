/*********************** Description ************************/



/************************* Includes *************************/

#include "ledFSM.h"

/********************* Global Variables *********************/

uint8_t state1;				// Переменная состояния автомата
uint8_t _state1;				// Переменная предыдущего состояния автомата
uint8_t entry1;				// Флаг перехода в новое состояние

/*************************	 Code	*************************/

void InitLedFSM (void){

	state1 = _state1 = 0;
	entry1 = 0;

}

void ProcessLedFSM (void){

	if (state1 != _state1) entry1 = 1; else entry1 = 0;

	_state1 = state1;

	switch (state1){

	case 0:
		if (GetMessage(LedOnMsg)){

			state1 = 1;
		}
		break;

	case 1:
		if (entry1 == 1){
			GPIOC->BSRR |= GPIO_BSRR_BS13;
			StartGTimer(GTimer1);
		}

		if (GetGTimerVal(GTimer1) >= 2000){
			state1 = 2;
			StopGTimer(GTimer1);
		}
		break;

	case 2:
		if (entry1 == 1){
			GPIOC->BSRR |= GPIO_BSRR_BR13;
			StartGTimer(GTimer1);
		}

		if (GetGTimerVal(GTimer1) >= 2000){
			state1 = 1;
			StopGTimer(GTimer1);
		}
		break;
	}

}
