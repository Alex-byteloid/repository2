/*********************** Description ************************/


/************************* Includes *************************/

#include "stm32f411xe.h"
#include "messages.h"
#include "virtualTimers.h"
#include "CRC16.h"

/*************************	Define	*************************/

#define SixthUSART 			6					// TX - A11; RX - A12

/****/
#ifdef SixthUSART

#define USART				USART6

#define IRQModbus			USART6_IRQn
//#define USART_IRQHandler	USART6_IRQHandler

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

#define ParityControlOn		10001
#define ParityControlOff	10000

#define OneStopBit			10002
#define TwoStopBit			10003

#define RTUMode				10004
#define ASCIIMode			10005

#define ModbusBufSize		256

#define ModbusSlaveAdress	0x12

/************************* Messages *************************/

/**

#define ModbusError				21					// Ошибка Modbus
#define ModbusReciveSymbol		22					// USART-Modbus принял символ
#define ModbusRTUTimeOut		23					// Актуально для режима RTU. Таймаут, конец сообщения
#define ModbusOverflowError		24					// Переполнение буфера Modbus
#define ModbusWaitingMessage	25					// Автомат в состоянии ожидания сообщения
#define ModbusCRCONotOk			26					// Несовпадение контрольной суммы
#define ModbusMessageReceived	27					// Сообщение получено
**/

/*************************	 Code	*************************/

void InitModbusUSART(uint32_t Speed, uint32_t ParityControl, uint32_t StopBit, uint32_t ModbusMode);
void InitModbusFSM (uint32_t Baud, uint32_t Parity, uint32_t StopBit,uint32_t ModbusMode);
void ProcessSlaveModbusMessageReceptionRTUFSM (void);
void ProcessMessageGenerationSlaveModbusRTUFSM (void);
void ProcessModbusSlaveFSM (void);
void USART6_IRQHandler (void);

