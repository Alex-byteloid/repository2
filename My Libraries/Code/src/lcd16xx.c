/*********************** Description ************************/

/******************************************************************************************************
 *												  ***											    ***
 * Description of I2C controller ports (PCF8574AT)***		Display position DDRAM address		    ***
 *												  ***											    ***
 *****P7***P6***P5***P4***P3***P2***P1***P0*****  ***00*01*02*03*04*05*06*07*08*09*0A*0B*0C*0D*0E*0F***
 *****DB7**DB6**DB5**DB4**LED**E****R/W**R/S****  ***40*41*42*43*44*45*46*47*48*49*4A*4B*4C*4D*4E*4F***
 *												  ***		  									    ***
 ******************************************************************************************************/

/************************* Includes *************************/

#include "lcd16xx.h"

/********************* Global Variables *********************/

const uint8_t BuferLCDInit[] = 	{0x3C, 0x38, 											/*	 0 - 1	*/
								0x3C, 0x38,												/* 	 2 - 3 	*/
								0x2C, 0x28,												/*	 4 - 5 	*/
								0x2C, 0x28, 0xCC, 0xC8,									/*	 6 - 9 */
								0x0C, 0x08, 0x8C, 0x88, 0x0C, 0x08, 0x8C, 0x88,			/* 	10 - 17 */
								0x0C, 0x08, 0x6C, 0x68, 0x0C, 0x08, 0xCC, 0xC8,			/* 	18 - 25 */
								0x0C, 0x08, 0x1C, 0x18};								/* 	26 - 29 */


const uint8_t DDRAMLCD1602 [2][16] = {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
						  	  	   	  {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F}};

uint8_t I2C1Data [I2C1DataBuferLenght];

uint8_t i2cSendStates;
uint8_t _i2cSendStates;
uint8_t i2cEntry;

uint8_t I2C1BuferSendLenght;

uint8_t lcdStates;
uint8_t _lcdStates;
uint8_t lcdEntry;

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

void InitDMAI2C1 (void){

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

	DMA1_Stream1->CR &= ~DMA_SxCR_CHSEL;

	DMA1_Stream1->M0AR = (uint32_t) & I2C1Data[0];
	DMA1_Stream1->PAR = (uint32_t) & I2C1->DR;


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

	switch (i2cSendStates){

	case 0:

		if (i2cEntry == 1){
			DMA1_Stream1->CR &= ~DMA_SxCR_EN;
			I2C1BuferSendLenght = 0;
		}

		if (I2C1BuferSendLenght == 0) {
			i2cSendStates = 0;
			DMA1_Stream1->CR &= ~DMA_SxCR_EN;
		}

		if (GetMessage(I2C1StartTransaction)){
			i2cSendStates = 1;
			DMA1_Stream1->NDTR = I2C1BuferSendLenght;
			DMA1_Stream1->CR |= DMA_SxCR_EN;
		}

		break;

	case 1:

		if (i2cEntry == 1){
			I2C1->CR1 |= I2C_CR1_START;								// Генерируем СТАРТ условие
			i2cSendStates = 2;
		}

		break;

	case 2:

		if (GetGTimerVal(GTimer1) > 1000){
			i2cSendStates = 3;
			StopGTimer(GTimer1);
		}

		break;

	case 3:

		GPIOC->BSRR |= GPIO_BSRR_BS15;

		break;
	}

}

/*************************	 IRQ_Handler (Обработчики прерываний)	*************************/

void DMA1_Stream1_IRQHandler (void){

	if (DMA1->LISR & DMA_LISR_TCIF1){
		I2C1->CR1 |= I2C_CR1_STOP;
		I2C1->CR2 &= ~I2C_CR2_DMAEN;
		i2cSendStates = 0;
		DMA1->LIFCR |= DMA_LIFCR_CTCIF1;
	}

}

void I2C1_EV_IRQHandler (void){

	if (I2C1->SR1 & I2C_SR1_SB){
		(void) I2C1->SR1;
		StartGTimer(GTimer1);
		I2C1->DR = AddrDevice;
		I2C1->CR2 |= I2C_CR2_DMAEN;
	}

	if (I2C1->SR1 & I2C_SR1_ADDR){
		(void) I2C1->SR1;
		(void) I2C1->SR2;
		StopGTimer(GTimer1);
	}

}

/*************************	 Функции для дисплея 16xx (16xx Display function)	*************************/


