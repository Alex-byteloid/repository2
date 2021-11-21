/*********************** Description ************************/


/************************* Includes *************************/

#include "stm32f411xe.h"
#include "system_stm32f4xx.h"
#include "modbus.h"

/*************************	Define	*************************/



/*************************	 Code	*************************/

void InitRCC (void);
void InitGPIO (void);
void UpdateNVICActiveAndPriority (void);
