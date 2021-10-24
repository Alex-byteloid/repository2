
/*********************** Description ************************/



/************************* Includes *************************/

#include "modbus.h"

/********************* Global Variables *********************/



/*************************	 Code	*************************/

void InitModbusUSART(uint32_t Speed, uint8_t ModbusMode){

	/**Включение тактирования модуля USART**/

	RCC->APB2ENR |= USARTClock;

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

	USART->BRR = Speed;									// Установка скорости

	USART->CR1 |= USART_CR1_RXNEIE;						// Разрешаем прерывание по приёму

	USART->CR1 |= USART_CR1_RE;							// Включаем приемник
	USART->CR1 |= USART_CR1_TE;							// Включаем передатчик

	__NVIC_EnableIRQ(IRQModbus);						// Разрешаем прерывания от USART

	USART->CR1 |= USART_CR1_UE;							// Включаем модуль USART в работу

}

