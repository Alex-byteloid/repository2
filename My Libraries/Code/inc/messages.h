/*********************** Description ************************/

/*
 * 0 - Сообщение неактивно
 * 1 - Сообщение установлено, но не активно
 * 2 - Сообщение активно
 * */

/************************* Includes *************************/

#include "stm32f411xe.h"

/*************************	Define	*************************/

#define MaxNumbMessages			30

#define LedOnMsg				1

#define WSLedStart				2

#define I2C1StartTransaction	10					// Начать передачу
#define I2C1SendByteComplete	11					// Байт отправлен, стоп условие сгенерировано
#define I2C1PauseTransaction	12					// Приостановка отправки буфера I2C1
#define I2C1ReleaseTransaction	13					// Возобновить отправку буфера I2C1
#define I2C1EndOfTransaction	14					// Заданное количество элементов буфера отправлено

#define LCDStartInit			15					// Начать инициализацию дисплея

#define ModbusError				21					// Ошибка Modbus
#define ModbusReciveSymbol		22					// USART-Modbus принял символ
#define ModbusRTUTimeOut		23					// Актуально для режима RTU. Таймаут, конец сообщения
#define ModbusOverflowError		24					// Переполнение буфера Modbus
#define ModbusWaitingMessage	25					// Автомат в состоянии ожидания сообщения
#define ModbusCRCNotOk			26					// Несовпадение контрольной суммы
#define ModbusMessageReceived	27					// Сообщение получено

/*************************	 Code	*************************/

void InitMessage (void);
void SendMessage (uint8_t Message, uint16_t ParametrOne, uint16_t ParametrTwo);
void ProcessMessage (void);
uint8_t GetMessage (uint8_t Message);

