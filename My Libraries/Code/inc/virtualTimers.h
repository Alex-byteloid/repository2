/*********************** Description ************************/


/************************* Includes *************************/

#include "stm32f411xe.h"
#include "core_cm4.h"

/*************************	Define	*************************/

#define MaxGTimers 3 // Максимальное количество глобальных таймеров

// Идентификаторы глобальных таймеров
#define GTimer1	0
#define GTimer2 1
#define GTimer3 2

// Состояния таймера
#define TimerStopped	0	// Таймер остановлен
#define TimerRunning	1	// Таймер работает
#define TimerPaused		2	// Таймер на паузе

/*************************	 Code	*************************/

void InitHardwareTimer (void);				// Функция инициализации аппаратного таймера SysTick
void InitGTimer(void);						// Функция инициализации глобальных таймеров
void StartGTimer(uint8_t GTimerID);			// Функция зауска выбранного глобального таймера
void StopGTimer(uint8_t GTimerID);			// Функция остановки выбранного глобального таймера
void PauseGTimer(uint8_t GTimerID);			// Функция приостановки выбранного глобального таймера
void ReleaseGTimer(uint8_t GTimerID);		// Функция продолжения счета выбранного глобального таймера
uint32_t GetGTimerVal(uint8_t GTimerID);	// Функция получения значения выбранного глобального таймера
