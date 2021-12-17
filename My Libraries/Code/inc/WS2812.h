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

#define NumberOfLeds		32
#define Bufer0				0
#define Bufer1				1
#define BuferIsActive		1
#define BuferIsDisable		0

/*************************	 Code	*************************/
void InitTIM3 (void);
void InitDMAforTIM3 (void);
void DMA1_Stream4_IRQHandler (void);

void InitWS281xFSM (void);
void ProcessWS281xFSM (void);
void InsertToRingBufferForDMA (RGBColorType Color);
