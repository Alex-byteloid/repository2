/*********************** Description ************************/



/************************* Includes *************************/

#include "ledFSM.h"

/********************* Global Variables *********************/

extern uint8_t state;				// Переменная состояния автомата
extern uint8_t _state;				// Переменная предыдущего состояния автомата
extern uint8_t entry;				// Флаг перехода в новое состояние

/*************************	 Code	*************************/

void InitLedFSM (void){

	state = _state = 0;
	entry = 0;

}

void ProcessLedFSM (void){

	if (state != _state) entry = 1; else entry = 0;

	switch (state){

	case 0:
		state = 1;
		break;

	case 1:
		if (entry == 1){
			GPIOC->BSRR |= GPIO_BSRR_BS13;
			StartGTimer(GTimer1);
			_state = state;
		}

		if (GetGTimerVal(GTimer1) >= 100){
			state = 2;
			StopGTimer(GTimer1);
		}
		break;

	case 2:
		if (entry == 1){
			GPIOC->BSRR |= GPIO_BSRR_BR13;
			StartGTimer(GTimer1);
			_state = state;
		}

		if (GetGTimerVal(GTimer1) >= 100){
			state = 1;
			StopGTimer(GTimer1);
		}
		break;
	}

}
