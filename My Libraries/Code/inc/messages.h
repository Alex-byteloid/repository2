/*********************** Description ************************/


/************************* Includes *************************/

#include "stm32f411xe.h"

/*************************	Define	*************************/

#define MaxNumbMessages		30

#define LedOnMsg			1



#define ModbusError				21					// Ошибка Modbus
#define ModbusReciveSymbol		22					// USART-Modbus принял символ
#define ModbusRTUTimeOut		23					// Актуально для режима RTU. Таймаут, конец сообщения
#define ModbusOverflowError		24					// Переполнение буфера Modbus
#define ModbusWaitingMessage	25					// Автомат в состоянии ожидания сообщения
#define ModbusCRCNotOk			26					// Несовпадение контрольной суммы
#define ModbusMessageReceived	27					// Сообщение получено

#define I2C1StartTransaction	10					// Начать передачу
#define LcdStartInit			11					// Старт инициализации дисплея

/*************************	 Code	*************************/

void InitMessage (void);
void SendMessage (int Msg);
void ProcessMessage (void);
uint8_t GetMessage (int Msg);

