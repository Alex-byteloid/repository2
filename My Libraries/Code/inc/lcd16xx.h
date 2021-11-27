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

#define I2C1DataBuferLenght	100


/************************* Messages *************************/
/*
 * #define I2CStartTransaction		12					// Начать передачу
 * */

/*************************	 Code	*************************/

void InitI2C1 (void);
void InitDMAI2C1 (void);

void InitI2C1FSM (void);
void ProcessI2CWriteFSM (void);
void DMA1_Stream1_IRQHandler (void);
void I2C1_EV_IRQHandler (void);



