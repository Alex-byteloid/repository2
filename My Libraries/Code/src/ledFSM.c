/*********************** Description ************************/



/************************* Includes *************************/

#include "ledFSM.h"

/********************* Global Variables *********************/

uint8_t state;				// Переменная состояния автомата
uint8_t _state;				// Переменная предыдущего состояния автомата
uint8_t entry;				// Флаг перехода в новое состояние

/*************************	 Code	*************************/

void InitLedFSM (void){

	state = _state = 0;
	entry = 0;

}

void ProcessLedFSM (void){

	if (state != _state) entry = 1; else entry = 0;

	_state = state;

	switch (state){

	case 0:
		if (GetMessage(LedOnMsg)){

			state = 1;
		}
		break;

	case 1:
		if (entry == 1){
			GPIOC->BSRR |= GPIO_BSRR_BS13;
			StartGTimer(GTimer1);
		}

		if (GetGTimerVal(GTimer1) >= 1000){
			state = 2;
			StopGTimer(GTimer1);
		}
		break;

	case 2:
		if (entry == 1){
			GPIOC->BSRR |= GPIO_BSRR_BR13;
			StartGTimer(GTimer1);
		}

		if (GetGTimerVal(GTimer1) >= 1000){
			state = 1;
			StopGTimer(GTimer1);
		}
		break;
	}

}
