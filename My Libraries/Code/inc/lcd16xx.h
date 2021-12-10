/*********************** Description ************************/

/**
	  PB6   ------> I2C1_SCL
	  PB7   ------> I2C1_SDA
**/

/************************* Includes *************************/

#include "stm32f411xe.h"
#include "messages.h"
#include "virtualTimers.h"
#include "I2C.h"

/*************************	Define	*************************/



/************************* Messages *************************/



/*************************	 Code	*************************/

void InitLcdFSM (void);
void ProcessLcdFSM (void);
