/*********************** Description ************************/



/************************* Includes *************************/

#include "ledFSM.h"

/********************* Global Variables *********************/

uint8_t statePis;				// Переменная состояния автомата
uint8_t _statePis;				// Переменная предыдущего состояния автомата
uint8_t entryPis1;				// Флаг перехода в новое состояние

/*************************	 Code	*************************/

void InitLedFSM (void){

	statePis = _statePis = 0;
	entryPis1 = 0;

}

void ProcessLedFSM (void){

	if (statePis != _statePis) entryPis1 = 1; else entryPis1 = 0;

	_statePis = statePis;

	switch (statePis){

	case 0:
		if (GetMessage(LedOnMsg)){

			statePis = 1;
		}
		break;

	case 1:
		if (entryPis1 == 1){
			GPIOC->BSRR |= GPIO_BSRR_BS13;
			StartLocTimer(LocTimer2);
		}

		if (GetLocTimerVal(LocTimer2) >= 2000){
			statePis = 2;
			StopLocTimer(LocTimer2);
		}
		break;

	case 2:
		if (entryPis1 == 1){
			GPIOC->BSRR |= GPIO_BSRR_BR13;
			StartLocTimer(LocTimer2);
		}

		if (GetLocTimerVal(LocTimer2) >= 2000){
			statePis = 1;
			StopLocTimer(LocTimer2);
		}
		break;
	}

}
