
/*********************** Description ************************/



/************************* Includes *************************/

#include "modbus.h"

/********************* Global Variables *********************/



/*************************	 Code	*************************/

void InitModbusUSART(uint32_t Speed, uint8_t ModbusMode, uint8_t ParityControl){

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
			USART->CR2 &= ~USART_CR2_STOP;									// Один стоп-бит
			USART->CR1 |= ~USART_CR1_M;										// Структура слова: 1 Start bit, 9 Data bits, n Stop bit
		}

		if(ParityControl == ParityControlOff){
			USART->CR1 &= ~USART_CR1_PCE;									// Выключение контроля чётности
			USART->CR2 &= ~USART_CR2_STOP;
			USART->CR2 |= USART_CR2_STOP_1;									// Два стоп-бита
			USART->CR1 &= ~USART_CR1_M;										// Структура слова: 1 Start bit, 8 Data bits, n Stop bit
		}

		USART->CR1 |= USART_CR1_RE;											// Включаем приемник
		USART->CR1 |= USART_CR1_TE;											// Включаем передатчик

		USART->CR1 |= USART_CR1_UE;											// Включаем модуль USART в работу

	}

}

