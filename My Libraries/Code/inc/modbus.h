/*********************** Description ************************/


/************************* Includes *************************/

#include "stm32f411xe.h"

/*************************	Define	*************************/

#define SixthUSART 		6	// TX - A11; RX - A12

#ifdef SixthUSART

#define USART			USART6

#define IRQModbus		USART6_IRQn

#define Gpio 			GPIOA
#define USARTBus		APB2ENR
#define USARTClock		RCC_APB2ENR_USART6EN

#define	TXPinPUPDR		GPIO_PUPDR_PUPD11
#define TXPinOTYPER 	GPIO_OTYPER_OT11
#define TXPinOSPEEDR	GPIO_OSPEEDER_OSPEEDR11
#define TXPinAFRH		GPIO_AFRH_AFRH3_3
#define TXPinMODER		GPIO_MODER_MODE11
#define	TXPinMODER0		GPIO_MODER_MODE11_0
#define TXPinMODER1		GPIO_MODER_MODE11_1

#define RXPinPUPDR		GPIO_PUPDR_PUPD12
#define	RXPinOTYPER		GPIO_OTYPER_OT12
#define	RXPinAFRH		GPIO_AFRH_AFRH4_3
#define RXPinMODER		GPIO_MODER_MODE12
#define	RXPinMODER0		GPIO_MODER_MODE12_0
#define RXPinMODER1		GPIO_MODER_MODE12_1

#endif

#define Baud9600			0x2710

#define ParityControlOn		0x1
#define ParityControlOff	0x2

#define RTUMode				0x11
#define ASCIIMode			0x12

/*************************	 Code	*************************/

