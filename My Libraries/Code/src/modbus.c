
/*********************** Description ************************/

/*Пока реализация такова:
 * По приёму первого байта, заходим в прерывание USART, там пишем сообщение приёма символа, переводим автомат в состояние №1(Приём сообщения).
 * Также нужно РЕинициализировать SysTick таймер, который будет считать тайм аут без приёма сообщения, примерно так:
 * При прерывании USART по приёму байта, обнуляем счётчик таймера, он начинает считать с нуля. Как только приём закончен, таймер может досчитать до нужной нам велечины
 * и в своём перерывании пошлёт сообщение ModbusRTUTimeOut, что приведёт к переходу автомата в состояние 2.
*/
/************************* Includes *************************/

#include "modbus.h"

/********************* Global Variables *********************/

uint8_t state;						// Переменная состояния автомата
uint8_t _state;						// Переменная предыдущего состояния автомата
uint8_t entry;						// Флаг перехода в новое состояние
extern uint8_t SysTickHandlerState;	// Переменная состояния системного таймера

uint8_t ModbusData[ModbusBufSize];	// Буфер данных Modbus

uint8_t CurrentItemOfBuf;			// Счётчик текущего элемента буфера Modbus

uint16_t CRCVal;					// Вычисленное значение контрольной суммы
uint16_t CRCRecVal;				// Принятое значение контрольной суммы

/*************************	 Code	*************************/

void InitModbusUSART(uint32_t Speed, uint8_t ParityControl, uint8_t StopBit, uint8_t ModbusMode){

	/**Включение тактирования модуля USART**/

	RCC->USARTBus |= USARTClock;

	/**Настройка TX на выход альтернативный пуш-пулл**/

	Gpio->PUPDR &= ~TXPinPUPDR;
	Gpio->OTYPER &= ~TXPinOTYPER;
	Gpio->OSPEEDR |= TXPinOSPEEDR;
	Gpio->AFR[1] |= TXPinAFRH;
	Gpio->MODER &= ~TXPinMODER;
	Gpio->MODER |= TXPinMODER1;

	/**Настройка RX на OD вход альт. функции**/

	Gpio->PUPDR &= ~RXPinPUPDR;
	Gpio->OTYPER &= ~RXPinOTYPER;
	Gpio->AFR[1] |= RXPinAFRH;
	Gpio->MODER &= ~RXPinMODER;
	Gpio->MODER |= RXPinMODER1;

	/**Настройка модуля USART**/

	if (ModbusMode == RTUMode){

		USART->BRR = Speed;													// Установка скорости

		USART->CR1 |= USART_CR1_RXNEIE;										// Разрешаем прерывание по приёму
		USART->CR1 |= USART_CR1_TXEIE;										// Разрешаем прерывание по передаче

		if(ParityControl == ParityControlOn){
			USART->CR1 |= USART_CR1_PCE;									// Включение бита контроля чётности
			USART->CR1 &= ~USART_CR1_PS;									// Включена "Чётная" чётность :)))))
		}

		if(ParityControl == ParityControlOff)USART->CR1 &= ~USART_CR1_PCE;	// Выключение контроля чётности

		if(StopBit == OneStopBit)USART->CR2 &= ~USART_CR2_STOP;				// Один стоп-бит

		if(StopBit == TwoStopBit){
			USART->CR2 &= ~USART_CR2_STOP;
			USART->CR2 |= USART_CR2_STOP_1;									// Два стоп-бита
		}

		USART->CR1 &= ~USART_CR1_M;											// Структура слова: 1 Start bit, 8 Data bits, n Stop bit
//		USART->CR1 |= ~USART_CR1_M;											// Структура слова: 1 Start bit, 9 Data bits, n Stop bit

		USART->CR1 |= USART_CR1_RE;											// Включаем приемник
		USART->CR1 |= USART_CR1_TE;											// Включаем передатчик

		USART->CR1 |= USART_CR1_UE;											// Включаем модуль USART в работу

	}

}

void InitModbusFSM (uint8_t Baud, uint8_t Parity, uint8_t StopBit,uint8_t ModbusMode){

	InitModbusUSART(Baud, Parity, StopBit, ModbusMode);
	state = _state = 0;
	entry = 0;
	SendMessage(ModbusInitOk);

}

void ProcessSlaveModbusRTUFSM (void){

	if (state != _state) entry = 1; else entry = 0;

	_state = state;

	switch (state){

	case 0:
		CurrentItemOfBuf = 0;
		SendMessage(ModbusWaitingMessage);
		if(GetMessage(ModbusReciveSymbol)){

			state = 1;
		}

		break;

	case 1:

		if (entry == 1){
			SysTick->LOAD = 18000;									// Загрузка значения перезагрузки. Вроде при 9600 бод это полтора символа (больше) и частоте шины 96 МГц
			SysTick->VAL = 18000;									// Обнуляем таймер и флаги.
		}

		if(GetMessage(ModbusReciveSymbol)){
			state = 1;
		}

		if(CurrentItemOfBuf >= ModbusBufSize){
			SendMessage(ModbusOverflowError);
			state = 6;
		}

		if(GetMessage(ModbusRTUTimeOut)){
			state = 2;
		}

		break;

	case 2:
		if (entry == 1){
			InitHardwareTimer();											// Реинициализируем системный таймер и переводим обработчик его прерывания в состояние 0
		}

		if (ModbusData[0] == ModbusSlaveAdress || ModbusData[0] == 0x00){	// Если адрес совпал, переходим в состояние 3 (вычисление CRC)
			state = 3;
		}
		else {
			state = 0;														// Eсли адрес не совпадает с адресом устройства или широковещательным, переходим в состояние 0 (ожидание приёма сообщения)
			for(uint8_t i = 0; i < CurrentItemOfBuf; i++){
				ModbusData[i] = 0;											// Затираем принятое сообщение Modbus
			}
		}

		break;

	case 3:

		CRCVal = CRC16(ModbusData, CurrentItemOfBuf);						// Вычисляем CRC16

		uint8_t CrcHi;
		uint8_t CrcLo;

		CrcHi = ModbusData [CurrentItemOfBuf - 2];
		CrcLo = ModbusData [CurrentItemOfBuf - 1];

		CRCRecVal = ((CrcHi << 8) | CrcLo);									// Записываем во временную переменную значение принятой контрольной суммы

		if (CRCVal == CRCRecVal) {											// Сравниваем значения контрольных сумм
			state = 4;
			SendMessage(ModbusMessageReceived);								// Сообщение Modbus получено
		}
		else {
			SendMessage(ModbusCRCNotOk);
			state = 6;
		}

		break;

	case 4:



		break;

	}

}

void USART_IRQHandler (void){

	if (USART->SR & USART_SR_RXNE_Msk){
		SysTick->VAL = 18000;										// Обнуляем таймер и флаги
		SysTickHandlerState = 1;									// Обработчик прерывания системного таймера переводим в состояние 1
		ModbusData[CurrentItemOfBuf] = USART->DR;					// Помещаем содержимое регистра данных USART  буфер сообщения Modbus
		CurrentItemOfBuf++;											// Инкрементируем указатель на текущий элемент буфера
		SendMessage(ModbusReciveSymbol);							// Помечаем активным сообщение "Modbus recyive symbol"
	}

}
