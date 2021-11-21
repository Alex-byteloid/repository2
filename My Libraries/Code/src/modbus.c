
/*********************** Description ************************/

/*
 * Version 0.1.1
 * 21.11.2021
 *
 * Закончил основную аппаратную реализацию протокола Modbus RTU для Slave устройства.
 *
 * На данный момент реализованы приём, инерпретация и отправка сообщений (с использованием DMA).
 * Функции Modbus протокола планируется добавлять по мере необходимости.
 *
 * Изменил режим работы таймеров, теперь в качестве основного используется таймер № 10.
 * Сделано это для гибкой настройки системы прерываний, чтобы не было простоя программы
 * при приёме сообщения Modbus RTU. Насколько это актуально - вопрос остаётся открытым,
 * так как у меня нет средств и знаний для точного и глубокого анализа работы программы
 * в данном режиме, чтобы сравнить с тем режимом, когда использовался системный таймер
 * и на время приёма сообщения изменялся режим его работы.
 *
 * */

/************************* Includes *************************/

#include "modbus.h"

/********************* Global Variables *********************/

extern uint16_t MRTUcount;					// Переменная счётчик, сбрасывается при каждом заходе в прерывание USART, когда не сбрасывается, то значит приёма нет. Её значение обрабатывается в обработчике прерывания таймера
extern uint16_t ReceptionStatus;			// Переменная, описывающая статус приёма Modbus RTU (запущен / остановлен)

uint8_t state;								// Переменная состояния автомата приёма сообщения
uint8_t _state;								// Переменная предыдущего состояния автомата приёма сообщения
uint8_t entry;								// Флаг перехода в новое состояние для автомата приёма сообщения

uint8_t stateMessageGenSlave;				// Переменная состояния автомата генерации ответного сообщения
uint8_t _stateMessageGenSlave;				// Переменная предыдущего состояния автомата генерации ответного сообщения
uint8_t entryMessageGenSlave;				// Флаг перехода в новое состояние для автомата генерации ответного сообщения

uint8_t ModbusData[ModbusBufSize];			// Буфер приёма данных Modbus
uint8_t ModbusSendData[ModbusBufSize];		// Буфер отправки сообщений Modbus

uint8_t CurrentItemOfBuf;					// Счётчик текущего элемента буфера Modbus
uint8_t _CurrentItemOfBuf;					// Счётчик текущего элемента буфера в отправляемом сообщении

uint16_t CRCVal;							// Вычисленное значение контрольной суммы
uint16_t CRCRecVal;							// Принятое значение контрольной суммы

/*************************	 Code	*************************/

void InitModbusUSART(uint32_t Speed, uint32_t ParityControl, uint32_t StopBit, uint32_t ModbusMode){

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


//		USART->CR1 |= USART_CR1_IDLEIE;										// Включаем детектирование свободной линии
		USART->CR1 |= USART_CR1_RE;											// Включаем приемник
		USART->CR1 |= USART_CR1_TE;											// Включаем передатчик

		USART->CR3 |= USART_CR3_DMAT;										// Разрешаем запросы к DMA от модуля USART

		USART->CR1 |= USART_CR1_UE;											// Включаем модуль USART в работу

	}

}

void InitDMAforUSART (void){

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

	DMA2_Stream7->CR |= DMA_SxCR_CHSEL_0 |
						DMA_SxCR_CHSEL_2;

	DMA2_Stream7->PAR = (uint32_t) & USART->DR;
	DMA2_Stream7->M0AR = (uint32_t) & ModbusSendData[0];

	DMA2_Stream7->CR &= ~DMA_SxCR_MSIZE;
	DMA2_Stream7->CR &= ~DMA_SxCR_PSIZE;

	DMA2_Stream7->CR |= DMA_SxCR_MINC;
	DMA2_Stream7->CR &= ~DMA_SxCR_PINC;

	DMA2_Stream7->CR |= DMA_SxCR_DIR_0;
	DMA2_Stream7->CR &= ~DMA_SxCR_CIRC;

	DMA2_Stream7->CR |= DMA_SxCR_TCIE;

}

void InitModbusFSM (uint32_t Baud, uint32_t Parity, uint32_t StopBit,uint32_t ModbusMode){

	InitModbusUSART(Baud, Parity, StopBit, ModbusMode);
	InitDMAforUSART();
	state = _state = 0;
	entry = 0;
	stateMessageGenSlave = _stateMessageGenSlave = 0;
	entryMessageGenSlave = 0;
	CurrentItemOfBuf = 0;

}

void ProcessSlaveModbusMessageReceptionRTUFSM (void){

	if (state != _state) entry = 1; else entry = 0;

	_state = state;

	switch (state){

	case 0:

		if (entry == 1){
			CurrentItemOfBuf = 0;
		}

		if(GetMessage(ModbusReciveSymbol)){
			state = 1;
		}

		break;

	case 1:

		if (entry == 1){
			ReceptionStatus = ReceptionEnabled;
		}

		if(CurrentItemOfBuf >= ModbusBufSize){

			state = 4;
		}

		if(GetMessage(ModbusRTUTimeOut)){
			state = 2;
		}

		break;

	case 2:

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

		CRCVal = 0;
		CRCRecVal = 0;

		CRCVal = CRC16(ModbusData, CurrentItemOfBuf - 2);					// Вычисляем CRC16

		uint8_t CrcHi;
		uint8_t CrcLo;

		CrcHi = ModbusData [CurrentItemOfBuf - 2];
		CrcLo = ModbusData [CurrentItemOfBuf - 1];

		CRCRecVal = ((CrcHi << 8) | CrcLo);									// Записываем во временную переменную значение принятой контрольной суммы

		if (CRCVal == CRCRecVal) {											// Сравниваем значения контрольных сумм
			SendMessage(ModbusMessageReceived);								// Сообщение Modbus получено
			_CurrentItemOfBuf = CurrentItemOfBuf;
			state = 0;
		}
		else {
			SendMessage(ModbusCRCNotOk);
			state = 4;
		}

		break;

	case 4:

		SendMessage(ModbusError);
		state = 0;

		break;

	}

}

void ProcessMessageGenerationSlaveModbusRTUFSM (void){

	if (stateMessageGenSlave != _stateMessageGenSlave) entryMessageGenSlave = 1; else entryMessageGenSlave = 0;

	_stateMessageGenSlave = stateMessageGenSlave;

	switch (stateMessageGenSlave){

	case 0:
		if (entryMessageGenSlave == 1){

			for(uint8_t i = 0; i < _CurrentItemOfBuf; i++){

				ModbusSendData[i] = 0;

			}

		}

		if (GetMessage(ModbusMessageReceived)){
			stateMessageGenSlave = 1;
		}
			break;

	case 1:

		if (ModbusData[1] == 0x05){

			WriteSingleCoil();
		}

		break;

	case 2:

		CRCRecVal = CRC16(ModbusSendData, _CurrentItemOfBuf - 2);

		ModbusSendData[_CurrentItemOfBuf-2] |= CRCRecVal;

		CRCRecVal = CRCRecVal >> 8;

		ModbusSendData[_CurrentItemOfBuf-1] |= CRCRecVal;

		stateMessageGenSlave = 3;

		break;

	case 3:


		if (entryMessageGenSlave == 1){

			USART->SR &= ~USART_SR_TC;

			DMA2_Stream7->NDTR = _CurrentItemOfBuf;

			DMA2_Stream7->CR |= DMA_SxCR_EN;

		}

		break;

	}
}


void ProcessModbusSlaveFSM (void){

	ProcessSlaveModbusMessageReceptionRTUFSM();
	ProcessMessageGenerationSlaveModbusRTUFSM();

}


/*************************	 IRQ_Handler (Обработчики прерываний)	*************************/


void ModbusUSART_IRQHandler (void){

	if (USART->SR & USART_SR_RXNE){

		ReceptionStatus = ReceptionEnabled;
		MRTUcount = 0;

		ModbusData[CurrentItemOfBuf] = USART->DR;				// Помещаем содержимое регистра данных USART  буфер сообщения Modbus
		CurrentItemOfBuf++;										// Инкрементируем указатель на текущий элемент буфера
		SendMessage(ModbusReciveSymbol);						// Активируем сообщение ModbusReciveSymbol
	}

	if (USART->SR & USART_SR_ORE){
		state = 4;
	}

}

void DMA2_Stream7_IRQHandler (void){

	if (DMA2->HISR & DMA_HISR_TCIF7){

		stateMessageGenSlave = 0;

		DMA2->HIFCR |= DMA_HIFCR_CTCIF7;

	}

}


/*************************	 Modbus Function (Modbus функции)	*************************/


/* Запись состояния одного выхода (Код функции 0x05) */
void WriteSingleCoil (void){

	if (ModbusData[3] == 0x50){

		if (ModbusData[4] == 0xff && ModbusData[5] == 0x00){

			GPIOC->BSRR |= GPIO_BSRR_BS14;

			for (uint8_t i = 0; i < _CurrentItemOfBuf - 2; i++){
				ModbusSendData[i] = ModbusData[i];
			}
				stateMessageGenSlave = 2;
			}

			if (ModbusData[4] == 0x00 && ModbusData[5] == 0xff){

			GPIOC->BSRR |= GPIO_BSRR_BR14;

			for (uint8_t i = 0; i < _CurrentItemOfBuf - 2; i++){
				ModbusSendData[i] = ModbusData[i];
			}
				stateMessageGenSlave = 2;
			}

	}

}
/**/
