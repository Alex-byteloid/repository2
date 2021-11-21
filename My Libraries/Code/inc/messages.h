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

#define I2CStartBitTimeOut		10					// Не сгенерировано старт условие для I2C
#define I2CAddrTimeOut			11					// Проблемы с обращением по адресу I2C
#define I2CStartTransaction		12					// Начать передачу
#define I2CDataSendTimeOut		13					// Таймаут отправки сооьщения I2C

/*************************	 Code	*************************/

void InitMessage (void);
void SendMessage (int Msg);
void ProcessMessage (void);
uint8_t GetMessage (int Msg);

