
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

uint8_t state;								// Переменная состояния автомата
uint8_t _state;								// Переменная предыдущего состояния автомата
uint8_t entry;								// Флаг перехода в новое состояние
extern uint8_t SysTickHandlerState;			// Переменная состояния системного таймера

uint8_t stateMessageGenSlave;				// Переменная состояния автомата генерации ответного сообщения
uint8_t _stateMessageGenSlave;				// Переменная предыдущего состояния автомата генерации ответного сообщения
uint8_t entryMessageGenSlave;				// Флаг перехода в новое состояние для автомата генерации ответного сообщения

uint8_t ModbusData[ModbusBufSize];			// Буфер приёма данных Modbus
uint8_t ModbusSendData[ModbusBufSize];		// Буфер отправки сообщений Modbus

uint8_t CurrentItemOfBuf;					// Счётчик текущего элемента буфера Modbus
uint8_t _CurrentItemOfBuf;

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
//		USART->CR1 |= USART_CR1_TXEIE;										// Разрешаем прерывание по передаче

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

		USART->CR1 |= USART_CR1_UE;											// Включаем модуль USART в работу

	}

}

void InitModbusFSM (uint32_t Baud, uint32_t Parity, uint32_t StopBit,uint32_t ModbusMode){

	InitModbusUSART(Baud, Parity, StopBit, ModbusMode);
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
			GPIOC->BSRR |= GPIO_BSRR_BR15;
		}
//		SendMessage(ModbusWaitingMessage);
		if(GetMessage(ModbusReciveSymbol)){

			state = 1;
		}

		break;

	case 1:

/*		if (entry == 1){
//			__disable_fault_irq();
//			PauseGTimer(GTimer1);
		}
*/
/*		if(GetMessage(ModbusReciveSymbol)){
			state = 1;
		}
*/

		if(CurrentItemOfBuf >= ModbusBufSize){
//			SendMessage(ModbusOverflowError);
			state = 4;
		}

		if(GetMessage(ModbusRTUTimeOut)){
			state = 2;
		}

		break;

	case 2:
		GPIOC->BSRR |= GPIO_BSRR_BS15;
		if (entry == 1){
//			__enable_fault_irq();
			SysTickHandlerState = 0;					// Обработчик прерывания системного таймера в состоянии 0

				SysTick->LOAD = 95999;						// Загрузка значения перезагрузки. При 96 МГц, данное занечение соотвествует прерыванию каждые 1 мс.
				SysTick->VAL = 95999;						// Обнуляем таймер и флаги.
			//InitHardwareTimer();
//			ReleaseGTimer(GTimer1);
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
//			GPIOC->BSRR |= GPIO_BSRR_BS15;
			_CurrentItemOfBuf = CurrentItemOfBuf;
			state = 0;
		}
		else {
			SendMessage(ModbusCRCNotOk);
			state = 4;
//			GPIOC->BSRR |= GPIO_BSRR_BS14;

		}

		break;

	case 4:

		SendMessage(ModbusError);
		//GPIOC->BSRR |= GPIO_BSRR_BS14;
		state = 0;

		break;

	}

}

void ProcessMessageGenerationSlaveModbusRTUFSM (void){

	if (stateMessageGenSlave != _stateMessageGenSlave) entryMessageGenSlave = 1; else entryMessageGenSlave = 0;

	_stateMessageGenSlave = stateMessageGenSlave;

	switch (stateMessageGenSlave){

	case 0:
		if (GetMessage(ModbusMessageReceived)){
			stateMessageGenSlave = 1;
		}
			break;

	case 1:



		if (ModbusData[1] == 0x05){

			if (ModbusData[3] == 0x50){

				if (ModbusData[4] == 0xff){

					GPIOC->BSRR |= GPIO_BSRR_BS14;
					SendMessage(LedOnMsg);
					for (uint8_t i = 0; i < _CurrentItemOfBuf - 2; i++){
						ModbusSendData[i] = ModbusData[i];
					}
					stateMessageGenSlave = 2;
				}

				if (ModbusData[4] == 0x00){

					GPIOC->BSRR |= GPIO_BSRR_BR14;

					for (uint8_t i = 0; i < _CurrentItemOfBuf - 2; i++){
						ModbusSendData[i] = ModbusData[i];
					}
					stateMessageGenSlave = 2;
				}

				ModbusSendData [6] = ModbusData [7];
				ModbusSendData [7] = ModbusData [6];


			}

		}

		break;

	case 2:


		for(uint8_t i = 0; i < _CurrentItemOfBuf; ){

			while (!(USART->SR & USART_SR_TC));

				USART->DR = ModbusSendData[i];
				i++;

		}

		stateMessageGenSlave = 0;
		break;

	}
}


void ProcessModbusSlaveFSM (void){

	ProcessSlaveModbusMessageReceptionRTUFSM();
	ProcessMessageGenerationSlaveModbusRTUFSM();

}


void USART6_IRQHandler (void){

	if (USART6->SR & USART_SR_RXNE){
//		SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
		SysTick->LOAD = 150000;									// Загрузка значения перезагрузки. При 96 МГц, данное занечение соотвествует прерыванию каждые 1 мс.
		SysTick->VAL = 150000;									// Обнуляем таймер и флаги.
		SysTickHandlerState = 1;
		ModbusData[CurrentItemOfBuf] = USART->DR;				// Помещаем содержимое регистра данных USART  буфер сообщения Modbus
		CurrentItemOfBuf++;										// Инкрементируем указатель на текущий элемент буфера
		SendMessage(ModbusReciveSymbol);
	}


/*	if (USART->SR & USART_SR_IDLE){
		SysTick->CTRL = SysTick_CTRL_TICKINT_Msk;
		state = 2;
		GPIOC->BSRR |= GPIO_BSRR_BS13;
		USART->SR;
	}
*/
	if (USART->SR & USART_SR_ORE){
		state = 4;
	}

}
