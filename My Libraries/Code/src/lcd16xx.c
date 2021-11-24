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

uint8_t I2C1Data [I2C1DataBuferLenght];

uint8_t i2cSendStates;
uint8_t _i2cSendStates;
uint8_t i2cEntry;

uint8_t I2C1BuferSendLenght;

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

void InitLCD (void){

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

	I2C1Data[32] = 0x4C;
	I2C1Data[33] = 0x48;
	I2C1Data[34] = 0xC;
	I2C1Data[35] = 0x8;

	I2C1BuferSendLenght = 6;
	SendMessage(I2C1StartTransaction);
}

uint8_t WriteCommand (uint8_t Data, uint8_t BuferLeftBorder){

	uint8_t Up = Data & 0xF0;
	uint8_t Low = (Data<<4) & 0xF0;

	uint8_t Send;

	Send = (Up | 0x0C);
	I2C1Data[BuferLeftBorder] = Send;
	Send = (Up | 0x08);
	I2C1Data[BuferLeftBorder + 1] = Send;

	Send = (Low | 0x0C);
	I2C1Data[BuferLeftBorder + 2] = Send;
	Send = (Low | 0x08);
	I2C1Data[BuferLeftBorder + 3] = Send;

	return (BuferLeftBorder + 3);

}

void WriteDataToLCD (uint8_t X,uint8_t Y, char *Str){

	for (uint8_t i = 0; i < I2C1DataBuferLenght; i++){
		I2C1Data[i] = 0;
	}

	uint8_t BuferRightBorder = 0;

	if (X == 1){

			switch (Y){

			case 1:
				BuferRightBorder = WriteCommand(0x80, 0);
				break;

			case 2:
				BuferRightBorder = WriteCommand(0x81, 0);
				break;

			case 3:
				BuferRightBorder = WriteCommand(0x82, 0);
				break;

			case 4:
				BuferRightBorder = WriteCommand(0x83, 0);
				break;

			case 5:
				BuferRightBorder = WriteCommand(0x84, 0);
				break;

			case 6:
				BuferRightBorder = WriteCommand(0x85, 0);
				break;

			case 7:
				BuferRightBorder = WriteCommand(0x86, 0);
				break;

			case 8:
				BuferRightBorder = WriteCommand(0x87, 0);
				break;

			case 9:
				BuferRightBorder = WriteCommand(0x88, 0);
				break;

			case 10:
				BuferRightBorder = WriteCommand(0x89, 0);
				break;

			case 11:
				BuferRightBorder = WriteCommand(0x8A, 0);
				break;

			case 12:
				BuferRightBorder = WriteCommand(0x8B, 0);
				break;

			case 13:
				BuferRightBorder = WriteCommand(0x8C, 0);
				break;

			case 14:
				BuferRightBorder = WriteCommand(0x8D, 0);
				break;

			case 15:
				BuferRightBorder = WriteCommand(0x8E, 0);
				break;

			case 16:
				BuferRightBorder = WriteCommand(0x8F, 0);
				break;
			}

		}

		if (X == 2){

				switch (Y){

				case 1:
					BuferRightBorder = WriteCommand(0xC0, 0);
					break;

				case 2:
					BuferRightBorder = WriteCommand(0xC1, 0);
					break;

				case 3:
					BuferRightBorder = WriteCommand(0xC2, 0);
					break;

				case 4:
					BuferRightBorder = WriteCommand(0xC3, 0);
					break;

				case 5:
					BuferRightBorder = WriteCommand(0xC4, 0);
					break;

				case 6:
					BuferRightBorder = WriteCommand(0xC5, 0);
					break;

				case 7:
					BuferRightBorder = WriteCommand(0xC6, 0);
					break;

				case 8:
					BuferRightBorder = WriteCommand(0xC7, 0);
					break;

				case 9:
					BuferRightBorder = WriteCommand(0xC8, 0);
					break;

				case 10:
					BuferRightBorder = WriteCommand(0xC9, 0);
					break;

				case 11:
					BuferRightBorder = WriteCommand(0xCA, 0);
					break;

				case 12:
					BuferRightBorder = WriteCommand(0xCB, 0);
					break;

				case 13:
					BuferRightBorder = WriteCommand(0xCC, 0);
					break;

				case 14:
					BuferRightBorder = WriteCommand(0xCD, 0);
					break;

				case 15:
					BuferRightBorder = WriteCommand(0xCE, 0);
					break;

				case 16:
					BuferRightBorder = WriteCommand(0xCF, 0);
					break;
				}

			}

		while (*Str){

			uint8_t Walue = (uint8_t)*Str;
			uint8_t UpByte = Walue & 0xF0;
			uint8_t LowByte = (Walue<<4) & 0xF0;

			uint8_t WriteByte;

			WriteByte = (UpByte | 0x0D);
			I2C1Data[BuferRightBorder] = WriteByte;
			WriteByte = (UpByte | 0x09);
			I2C1Data[BuferRightBorder + 1] = WriteByte;

			WriteByte = (LowByte | 0x0D);
			I2C1Data[BuferRightBorder + 2] = WriteByte;
			WriteByte = (LowByte | 0x09);
			I2C1Data[BuferRightBorder + 3] = WriteByte;

			BuferRightBorder = BuferRightBorder + 3;
			Str++;

		}

		I2C1BuferSendLenght = BuferRightBorder;

}

void ClearGram (void){

	I2C1Data[0] = 0x0C;
	I2C1Data[1] = 0x08;
	I2C1Data[2] = 0x1C;
	I2C1Data[3] = 0x18;

	I2C1BuferSendLenght = 4;
}
