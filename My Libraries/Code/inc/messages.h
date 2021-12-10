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

#define I2C1StartTransaction	10					// Начать передачу
#define I2CAddrOk				11					// Есть ответ от устройства с заданным адресом

#define LCDStartInit			13					// Начать инициализацию дисплея

#define ModbusError				21					// Ошибка Modbus
#define ModbusReciveSymbol		22					// USART-Modbus принял символ
#define ModbusRTUTimeOut		23					// Актуально для режима RTU. Таймаут, конец сообщения
#define ModbusOverflowError		24					// Переполнение буфера Modbus
#define ModbusWaitingMessage	25					// Автомат в состоянии ожидания сообщения
#define ModbusCRCNotOk			26					// Несовпадение контрольной суммы
#define ModbusMessageReceived	27					// Сообщение получено

/*************************	 Code	*************************/

void InitMessage (void);
void SendMessage (uint16_t Message, uint16_t ParametrOne, uint16_t ParametrTwo);
void ProcessMessage (void);
uint8_t GetMessage (uint8_t Message);

