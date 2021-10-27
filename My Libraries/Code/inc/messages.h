/*********************** Description ************************/


/************************* Includes *************************/

#include "stm32f411xe.h"

/*************************	Define	*************************/

#define MaxNumbMessages		5

#define LedOnMsg			0

/*************************	 Code	*************************/

void InitMessage (void);
void SendMessage (int Msg);
void ProcessMessage (void);
uint8_t GetMessage (int Msg);

