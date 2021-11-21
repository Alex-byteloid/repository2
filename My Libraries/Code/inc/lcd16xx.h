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

#define I2CDataBuferLenght	28

/************************* Messages *************************/
/*
 *
 * #define I2CStartBitTimeOut		10					// Не сгенерировано старт условие для I2C
 * #define I2CAddrTimeOut			11					// Проблемы с обращением по адресу I2C
 * #define I2CStartTransaction		12					// Начать передачу
 * #define I2CDataSendTimeOut		13					// Таймаут отправки сооьщения I2C
 * */

/*************************	 Code	*************************/

void InitI2C1 (void);
void InitI2C1FSM (void);
void ProcessWriteI2C1FSM (void);
void LCDInit (void);
