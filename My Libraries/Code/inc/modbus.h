/*********************** Description ************************/


/************************* Includes *************************/

#include "stm32f411xe.h"
#include "messages.h"

/*************************	Define	*************************/

#define NinthTimer			9
#define SixthUSART 			6					// TX - A11; RX - A12

/****/
#ifdef NinthTimer

#define TimerBus			APB2ENR
#define TimerClock			RCC_APB2ENR_TIM9EN

#define	Timer				TIM9

#endif

/****/
#ifdef SixthUSART

#define USART				USART6

#define IRQModbus			USART6_IRQn
#define USART_IRQHandler	USART6_IRQHandler

#define Gpio 				GPIOA
#define USARTBus			APB2ENR
#define USARTClock			RCC_APB2ENR_USART6EN

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

/****/

#define Baud9600			0x2710

#define ParityControlOn		1
#define ParityControlOff	2

#define OneStopBit			3
#define TwoStopBit			4

#define RTUMode				5
#define ASCIIMode			6

#define ModbusBufSize		256


/************************* Messages *************************/

#define ModbusInitOk			20					// Инициализация ModbusFSM прошла успешно
#define ModbusError				21					// Ошибка Modbus
#define ModbusRecyiveSymbol		22					// USART-Modbus принял символ
#define ModbusRTUTimeOut		23					// Актуально для режима RTU. Таймаут, конец сообщения
#define ModbusOverflowError		24					// Переполнение буфера Modbus
#define ModbusWaitingMessage	25					// Автомат в состоянии ожидания сообщения

/*************************	 Code	*************************/

void InitModbusUSART(uint32_t Speed, uint8_t ParityControl, uint8_t StopBit, uint8_t ModbusMode);

