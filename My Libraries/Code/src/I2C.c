/*********************** Description ************************/

/*
 * Первый параметр сообщения I2C1StartTransaction это стартовый адрес элемента буфера для отправки
 * Второй параметр сообщения I2C1StartTransaction это длина передаваемого буфера
 * */

/************************* Includes *************************/

#include "I2C.h"

/********************* Global Variables *********************/

uint8_t i2cSendStates;									// Текущее состояние конечного автомата отправки данных по I2C
uint8_t _i2cSendStates;									// Предыдущее состояние конечного автомата отправки данных по I2C
uint8_t i2cEntry;										// Переменная для индикации первого входа в состояние

uint8_t I2C1Data [I2C1DataBuferLenght];					// Буфер отправки данных I2C1

uint8_t I2C1NumberOfTransaction;						// Количество текущих передач по I2C1

uint16_t I2C1CurrentBuferItem;							// Текущий отправляемый элемент буфера
uint16_t I2C1SendBuferLenght;							// Длина перевадаемого буфера



/*************************	 Code	*************************/

void InitI2C1 (void){

/* I2C1 GPIO Configuration */

	GPIOB->MODER &= ~GPIO_MODER_MODER6;
	GPIOB->MODER |= GPIO_MODER_MODER6_1;				// Configuring the I / O Direction Mode for the Alternate Function
	GPIOB->OTYPER |= GPIO_OTYPER_OT6;					// Configuring the output type of the open-drain
	GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED6;
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED6_0;			// Configuring the output speed is low
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD6;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD6_0;					// Configuring the I / O pull-up
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFRL6;					// Enable AF4 (I2C1...3) on port PB6
	GPIOB->AFR[0] |= GPIO_AFRL_AFRL6_2;

	GPIOB->MODER &= ~GPIO_MODER_MODER7;
	GPIOB->MODER |= GPIO_MODER_MODER7_1;				// Configuring the I / O Direction Mode for the Alternate Function
	GPIOB->OTYPER |= GPIO_OTYPER_OT7;					// Configuring the output type of the open-drain
	GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED7;
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED7_0;			// Configuring the output speed is low
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD7;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD7_0;					// Configuring the I / O pull-up
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFRL7;					// Enable AF4 (I2C1...3) on port PB6
	GPIOB->AFR[0] |= GPIO_AFRL_AFRL7_2;

	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;					// Enable I2C1 clocking

	I2C1->OAR2 &= ~I2C_OAR2_ENDUAL;						// Disable Dual addressing mode

	I2C1->CR1 &= ~I2C_CR1_ENGC;							// Disable General call

	I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;					// Enable clock stretching

	I2C1->CR1 &= ~I2C_CR1_SMBUS;						// Enable I2C mode

	I2C1->CR2 &= ~I2C_CR2_FREQ;
	I2C1->CR2 |= 48;									// Set 48 MHz frequency

	I2C1->CCR &= ~I2C_CCR_FS;							// Sm mode I2C

	I2C1->CCR &= ~I2C_CCR_CCR;
	I2C1->CCR |= 240;

	I2C1->TRISE = 3;

	I2C1->CR2 |= I2C_CR2_ITEVTEN;						// Разрешаем прерывания по событиям отравки и др. (Event interrupt enable)

	I2C1->CR1 |= I2C_CR1_PE;							// I2C1 Enable

}

void InitI2C1FSM (void){

	InitI2C1();

	i2cSendStates = 0;
	_i2cSendStates = 0;
	I2C1NumberOfTransaction = 0;

	ClearI2C1DataBufer();
}

void ProcessI2CWriteFSM (void){

	if ( i2cSendStates != _i2cSendStates) i2cEntry = 1; else i2cEntry = 0;

	_i2cSendStates = i2cSendStates;

	switch (i2cSendStates){

	case 0:

		if (GetMessage(I2C1StartTransaction)){
			i2cSendStates = 1;
//			I2C1CurrentBuferItem = Messages[I2C1StartTransaction].ParamOne;
//			I2C1SendBuferLenght = Messages[I2C1StartTransaction].ParamTwo;
		}
		break;

	case 1:

		if (i2cEntry == 1){
			I2C1->CR1 |= I2C_CR1_START;
		}

		if (GetGTimerVal(I2C1Timer) >= 2000){
			i2cSendStates = 5;
		}

		if (GetMessage(I2C1SendByteComplete)){
			i2cSendStates = 2;
		}
		break;

	case 2:

		if(GetMessage(I2C1PauseTransaction)){
			i2cSendStates = 4;
		}
		else {
			if (I2C1NumberOfTransaction == I2C1SendBuferLenght){
				i2cSendStates = 3;
			}
			else {
				i2cSendStates = 1;
			}
		}
		break;

	case 3:
		SendMessage(I2C1EndOfTransaction, 0, 0);
		I2C1NumberOfTransaction = 0;
		I2C1SendBuferLenght = 0;
		I2C1CurrentBuferItem = 0;
		i2cSendStates = 0;
		break;

	case 4:
		if (GetMessage(I2C1ReleaseTransaction)){
			i2cSendStates = 1;
		}
		break;

	case 5:

		GPIOC->BSRR |= GPIO_BSRR_BS15;
		break;
	}

}



/*************************	 IRQ_Handler (Обработчики прерываний)	*************************/

void I2C1_EV_IRQHandler (void){

	if (I2C1->SR1 & I2C_SR1_SB){
		(void) I2C1->SR1;
		StartGTimer(I2C1Timer);
		I2C1->DR = AddrDevice;
	}

	if (I2C1->SR1 & I2C_SR1_ADDR){
		(void) I2C1->SR1;
		(void) I2C1->SR2;
		StopGTimer(I2C1Timer);
		I2C1->DR = I2C1Data [I2C1CurrentBuferItem];
	}

	if (I2C1->SR1 & I2C_SR1_BTF){
		I2C1->CR1 |= I2C_CR1_STOP;
		SendMessage(I2C1SendByteComplete, 0, 0);
		I2C1NumberOfTransaction++;
		I2C1CurrentBuferItem++;
	}

}

void ClearI2C1DataBufer (void){
	for (uint8_t Clear = 0; Clear < I2C1DataBuferLenght; Clear++){
		I2C1Data[Clear] = 0x00;
	}
}

void WriteToI2C1DataBufer (uint8_t Data, uint8_t Position){
	I2C1Data[Position] = Data;
}
