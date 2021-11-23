/*********************** Description ************************/



/************************* Includes *************************/

#include "lcd16xx.h"

/********************* Global Variables *********************/

uint8_t I2C1Data [I2C1DataBuferLenght];

uint8_t i2cSendStates;
uint8_t _i2cSendStates;
uint8_t i2cEntry;

uint8_t I2C1CurrentBuferElement;
uint8_t I2C1BuferLenght;

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

	I2C1->CR1 |= I2C_CR1_PE;							// I2C1 Enable

}

void InitDMAI2C1 (void){

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

	DMA1_Stream1->CR &= ~DMA_SxCR_CHSEL;

	DMA1_Stream1->PAR = (uint32_t) & I2C1->DR;
	DMA1_Stream1->M0AR = (uint32_t) & I2C1Data[0];

	DMA1_Stream1->CR &= ~DMA_SxCR_MSIZE;
	DMA1_Stream1->CR &= ~DMA_SxCR_PSIZE;

	DMA1_Stream1->CR |= DMA_SxCR_MINC;
	DMA1_Stream1->CR &= ~DMA_SxCR_PINC;

	DMA1_Stream1->CR &= ~DMA_SxCR_PL;
	DMA1_Stream1->CR |= DMA_SxCR_DIR_0;
	DMA1_Stream1->CR &= ~DMA_SxCR_CIRC;

	DMA1_Stream1->CR |= DMA_SxCR_TCIE;

}

void InitI2C1FSM (void){

	InitI2C1();
	InitDMAI2C1();

	i2cSendStates = 0;
	_i2cSendStates = 0;
}

void ProcessI2CWriteFSM (void){

	if ( i2cSendStates != _i2cSendStates) i2cEntry = 1; else i2cEntry = 0;

	_i2cSendStates = i2cSendStates;

	if (I2C1BuferLenght == 0){
		i2cSendStates = 0;
	}

	switch (i2cSendStates){

	case 0:
		if (GetMessage(I2CStartTransaction)){
			i2cSendStates = 1;
		}
		break;

	case 1:
		if (i2cEntry == 1){
			I2C1->CR1 |= I2C_CR1_START;						// Формируем сигнал на отправку старт бита (SB(Start condition generated))
			StartGTimer(GTimer1);
		}

		if (GetGTimerVal(GTimer1) > 300){
			SendMessage(I2CStartBitTimeOut);
			StopGTimer(GTimer1);
			i2cSendStates = 5;
		}

		if (I2C1->SR1 & I2C_SR1_SB){
			i2cSendStates = 2;
			StopGTimer(GTimer1);
			(void) I2C1->SR1;								// Очищаем SB(Start condition generated) в регистре SR1
		}
		break;

	case 2:
		if (i2cEntry == 1){
			DMA1_Stream1->NDTR = I2C1BuferLenght;
			DMA1_Stream1->CR |= DMA_SxCR_EN;
			I2C1->DR = AddrDevice;							// Отправка адреса устройства на линии I2C1 (Address sent (master mode))
			I2C1->CR2 |= I2C_CR2_DMAEN;						// Enable DMA
			StartGTimer(GTimer1);
		}

		if (GetGTimerVal(GTimer1) > 300){
			SendMessage(I2CAddrTimeOut);
			I2C1->CR2 &= ~I2C_CR2_DMAEN;
			StopGTimer(GTimer1);
			i2cSendStates = 5;
		}

		if (I2C1->SR1 & I2C_SR1_ADDR){						// Адрес совпал (Received address matched)
			(void) I2C1->SR1;								// Сбрасываем бит совадения адреса (cleared by software reading SR1 register
			(void) I2C1->SR2;								// followed reading SR2, or by hardware when PE=0)
			GPIOC->BSRR |= GPIO_BSRR_BR15;
			StopGTimer(GTimer1);
			i2cSendStates = 3;
		}
		break;

	case 3:

		if (GetMessage(I2CDataSendComplete)){
			i2cSendStates = 4;
		}
		break;

	case 4:
		if (i2cEntry == 1){
			DMA1_Stream1->CR &= ~DMA_SxCR_EN;
			I2C1->CR1 |= I2C_CR1_STOP;
//			GPIOC->BSRR |= GPIO_BSRR_BR14;
		}

		if (I2C1->SR1 & I2C_SR1_BTF){
			i2cSendStates = 0;
		}

		break;

	case 5:
		if (GetMessage(I2CStartBitTimeOut)){
			GPIOC->BSRR |= GPIO_BSRR_BS15;
			i2cSendStates = 0;
		}

		if (GetMessage(I2CAddrTimeOut)){
			GPIOC->BSRR |= GPIO_BSRR_BS15;
			i2cSendStates = 0;
		}
		break;
	}

}

/*************************	 IRQ_Handler (Обработчики прерываний)	*************************/

void DMA1_Stream1_IRQHandler (void){

	if (DMA1->LISR & DMA_LISR_TCIF1){
		I2C1->CR2 &= ~I2C_CR2_DMAEN;
		SendMessage(I2CDataSendComplete);
		I2C1BuferLenght = 0;
	}
	GPIOC->BSRR |= GPIO_BSRR_BS14;
	DMA1->LIFCR |= DMA_LIFCR_CTCIF1;
	DMA1_Stream1->CR &= ~DMA_SxCR_EN;
}

/*************************	 Функции для дисплея 16xx (16xx Display function)	*************************/

void LCDInit (void){

	I2C1Data[0] = 0x3C;
	I2C1Data[1] = 0x38;
	I2C1Data[2] = 0x3C;
	I2C1Data[3] = 0x38;
	I2C1Data[4] = 0x3C;
	I2C1Data[5] = 0x38;

	I2C1Data[6] = 0x2C;
	I2C1Data[7] = 0x28;

	I2C1Data[8] = 0x2C;
	I2C1Data[9] = 0x28;
	I2C1Data[10] = 0xCC;
	I2C1Data[11] = 0xC8;

	I2C1Data[12] = 0x0C;
	I2C1Data[13] = 0x08;
	I2C1Data[14] = 0x8C;
	I2C1Data[15] = 0x88;

	I2C1Data[16] = 0x0C;
	I2C1Data[17] = 0x08;
	I2C1Data[18] = 0x8C;
	I2C1Data[19] = 0x88;

	I2C1Data[20] = 0x0C;
	I2C1Data[21] = 0x08;
	I2C1Data[22] = 0x6C;
	I2C1Data[23] = 0x68;

	I2C1Data[24] = 0x0C;
	I2C1Data[25] = 0x08;
	I2C1Data[26] = 0xCC;
	I2C1Data[27] = 0xC8;

	I2C1Data[28] = 0x0C;
	I2C1Data[29] = 0x08;
	I2C1Data[30] = 0x1C;
	I2C1Data[31] = 0x18;

	I2C1BuferLenght = 4;

}




