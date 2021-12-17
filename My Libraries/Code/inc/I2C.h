/*********************** Description ************************/

/**
	  PB6   ------> I2C1_SCL
	  PB7   ------> I2C1_SDA
**/

/************************* Includes *************************/

#include "stm32f411xe.h"
#include "messages.h"
#include "virtualTimers.h"

/*************************	Define	*************************/

#define AddrDevice			0x4E

#define I2C1DataBuferLenght	70


/************************* Messages *************************/
/*
 *
 * #define I2C1StartTransaction		10		// Начать передачу
 * #define I2C1SendByteComplete		11		// Есть ответ от устройства с заданным адресом
 * #define I2C1PauseTransaction		12		// Приостановка отправки буфера I2C1
 * #define I2C1ReleaseTransaction	13		// Возобновить отправку буфера I2C1
 * #define I2C1EndOfTransaction		14		// Заданное количество элементов буфера отправлено
 *
 * */

/*************************	 Code	*************************/

void InitI2C1 (void);

void InitI2C1FSM (void);
void ProcessI2CWriteFSM (void);
void I2C1_EV_IRQHandler (void);

void ClearI2C1DataBufer (void);
void WriteToI2C1DataBufer (uint8_t Data, uint8_t Position);
