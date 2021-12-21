/*********************** Description ************************/


/************************* Includes *************************/

#include "stm32f411xe.h"
#include "virtualTimers.h"
#include "messages.h"

/*************************	Define	*************************/

typedef struct RGB {
	uint8_t Red;
	uint8_t Green;
	uint8_t Blue;
}RGBColorType;

#define NumberOfStairs		8
#define NumberOfLeds		4

/*************************	 Code	*************************/
void InitTIM3 (void);
void InitDMAforTIM3 (void);
void DMA1_Stream4_IRQHandler (void);

void InitWS281xFSM (void);
void ProcessWS281xFSM (void);
void ProcessEffectsWS281xFSM (void);

void ClearStairsBuffer (void);
void InsertColorToONELEDStairsBuffer (RGBColorType Color, uint16_t StairsNumber, uint16_t LedNumber);
void InsertColorToMULTIPLELEDStairsBuffer (RGBColorType Colores, uint16_t StairsNumber, uint16_t LedNumberLeft, uint16_t LedNumberRight);
void InsertColorToAllStairsBuffer (RGBColorType Coloress);
