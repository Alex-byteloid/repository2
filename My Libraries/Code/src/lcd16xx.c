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

const uint8_t BuferLCDInit[] = 	{0x3C, 0x38,  									/*	 0 - 1	-> пауза 6 м/с */
								 0x3C, 0x38, 									/* 	 2 - 3 	-> пауза 2 м/с */
								 0x3C, 0x38,									/*	 4 - 5 	*/
								 0x2C, 0x28, 									/*	 6 - 7	*/
								 0x2C, 0x28, 0x8C, 0x88,						/*	 8 - 11  */
								 0x0C, 0x08, 0xFC, 0xF8,						/*	12 - 15 */
								 0x0C, 0x08, 0x1C, 0x18,						/*	16 - 19 */
								 0x0C, 0x08, 0x6C, 0x68,						/*	20 - 23 */
								 0x0C, 0x08, 0xFC, 0xF8,						/*	24 - 27 */
								 0x0C, 0x08, 0x1C, 0x18							/*	28 - 31	*/
};

const uint8_t DDRAMLCD1602 [2][16] = {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
						  	  	   	  {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F}};

uint8_t I2C1Data [I2C1DataBuferLenght];

uint8_t i2cSendStates;
uint8_t _i2cSendStates;
uint8_t i2cEntry;

uint8_t I2C1NumberOfTransaction;						//	количество передач по I2C
uint8_t I2C1SendBuferLenght;							//	длина перевадаемого буфера

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

void InitI2C1FSM (void){

	InitI2C1();

	i2cSendStates = 0;
	_i2cSendStates = 0;
	I2C1NumberOfTransaction = 0;

	for (uint8_t i = 0; i < I2C1DataBuferLenght; i++){
		I2C1Data[i] = 0x00;
	}
}

void ProcessI2CWriteFSM (void){

	if ( i2cSendStates != _i2cSendStates) i2cEntry = 1; else i2cEntry = 0;

	_i2cSendStates = i2cSendStates;

	switch (i2cSendStates){

	case 0:

		if (GetMessage(I2C1StartTransaction)){
			if (I2C1SendBuferLenght != 0){
				i2cSendStates = 1;
			}
		}
		break;

	case 1:

		if (lcdEntry == 1){
			I2C1->CR1 |= I2C_CR1_START;
		}

		if (GetGTimerVal(I2C1Timer) >= 1000){
			i2cSendStates = 4;
		}
		break;

	case 2:
		if (lcdEntry == 1) StopGTimer(I2C1Timer);
		break;

	case 3:

		if (I2C1NumberOfTransaction == I2C1SendBuferLenght){
			i2cSendStates = 0;
			I2C1NumberOfTransaction = 0;
		}
		else {
			i2cSendStates = 1;
		}
		break;

	case 4:
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
		i2cSendStates = 2;
		I2C1->DR = I2C1Data[I2C1NumberOfTransaction];
	}

	if (I2C1->SR1 & I2C_SR1_BTF){
		I2C1->CR1 |= I2C_CR1_STOP;
		I2C1NumberOfTransaction++;
		i2cSendStates = 3;
	}

}

/*************************	 Функции для дисплея 16xx (16xx Display function)	*************************/

void InitLcdFSM (void){

	lcdStates = 0;
	_lcdStates = 0;
	I2C1LeftBorder = 0;
}

void ProcessLcdFSM (void){

	if (lcdStates != _lcdStates) lcdEntry = 1; else lcdEntry = 0;
	_lcdStates = lcdStates;

	switch (lcdStates){

	case 0:
		if (GetMessage(LCDStartInit)){

			for (uint8_t i = 0; i < 32; i++){
				I2C1Data[i] = BuferLCDInit[i];
			}
			lcdStates = 1;
		}
		break;

	case 1:

		if (lcdEntry == 1){
			I2C1LeftBorder = 0;
			DMA1_Stream1->NDTR = 2;
			SendMessage(I2C1StartTransaction);
		}

		if (I2C1NumberOfTransaction == 1){
			StartGTimer(LCDTimer);
		}

		if (I2C1NumberOfTransaction == 2){
			StopGTimer(LCDTimer);
			I2C1LeftBorder = 4;
			SendMessage(I2C1StartTransaction);
			StartGTimer(LCDTimer);
		}

		if (GetGTimerVal(LCDTimer) > 10){
			if (I2C1NumberOfTransaction == 1){
				StopGTimer(LCDTimer);
				I2C1LeftBorder = 2;
				SendMessage(I2C1StartTransaction);
				StartGTimer(LCDTimer);
			}
			if (I2C1NumberOfTransaction == 3){
				StopGTimer(LCDTimer);
				I2C1LeftBorder = 6;
				SendMessage(I2C1StartTransaction);
			}
			if (I2C1NumberOfTransaction == 4){
				DMA1_Stream1->NDTR = 4;
				lcdStates = 2;
			}
		}
		break;

	case 2:

		if (lcdEntry == 1){
			I2C1LeftBorder = 8;
			SendMessage(I2C1StartTransaction);
		}

		switch (I2C1NumberOfTransaction){

					case 4:

						break;
					case 5:
						I2C1LeftBorder = 8;
						SendMessage(I2C1StartTransaction);
						break;
					case 6:
						I2C1LeftBorder = 12;
						SendMessage(I2C1StartTransaction);
						break;
					case 7:
						I2C1LeftBorder = 16;
						SendMessage(I2C1StartTransaction);

						break;
					case 8:
						I2C1LeftBorder = 20;
						SendMessage(I2C1StartTransaction);

						break;
					case 9:
						I2C1LeftBorder = 24;
						SendMessage(I2C1StartTransaction);

						break;
					case 10:
						I2C1LeftBorder = 28;
						SendMessage(I2C1StartTransaction);

						break;
					case 11:
						lcdStates = 3;
						I2C1NumberOfTransaction = 0;
						break;
					}
		break;

	case 3:

		break;

	}
}

