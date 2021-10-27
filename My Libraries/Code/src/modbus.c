
/*********************** Description ************************/

/*Пока реализация такова:
 * По приёму первого байта, заходим в прерывание USART, там пишем сообщение приёма символа, переводим автомат в состояние №1(Приём сообщения).
 * В этом состоянии в первом проходе отключаем прерывания от таймера SysTick, чтобы он не перебивал нам прерывания от USART. Считать при этом как я понимаю он не перестаёт.
 * Также нужно инициализировать доп таймер, который будет считать тайм аут без приёма сообщения, примерно так:
 * При прерывании USART по приёму байта, обнуляем счётчик таймера, он начинает считать с нуля. Как только приём закончен, таймер может досчитать до нужной нам велечины
 * и в своём перерывании пошлёт сообщение ModbusRTUTimeOut, что приведёт к переходу автомата в состояние 2.
*/
/************************* Includes *************************/

#include "modbus.h"

/********************* Global Variables *********************/

uint8_t state;						// Переменная состояния автомата
uint8_t _state;						// Переменная предыдущего состояния автомата
uint8_t entry;						// Флаг перехода в новое состояние

uint8_t ModbusData[ModbusBufSize];	// Буфер данных Modbus

uint8_t CurrentItemOfBuf;			// Счётчик текущего элемента буфера Modbus

/*************************	 Code	*************************/

void InitModbusTimer(void){

	/**Включение тактирования модуля таймера**/

	RCC->TimerBus |= TimerClock;



}

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

void ProcessReceiveModbusRTUFSM (void){

	if (state != _state) entry = 1; else entry = 0;

	_state = state;

	switch (state){

	case 0:
		CurrentItemOfBuf = 0;
		SendMessage(ModbusWaitingMessage);
		if(GetMessage(ModbusRecyiveSymbol)){

			state = 1;
		}

		break;

	case 1:

		if (entry == 1){
			SysTick->CTRL = ~SysTick_CTRL_TICKINT_Msk;
		}

		if(GetMessage(ModbusRecyiveSymbol)){
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
			SysTick->CTRL = SysTick_CTRL_TICKINT_Msk;
		}


	}

}

void USART_IRQHandler (void){

	if (USART->SR & USART_SR_RXNE){

		ModbusData[CurrentItemOfBuf] = USART->DR;
		CurrentItemOfBuf++;
		SendMessage(ModbusRecyiveSymbol);
	}

}

