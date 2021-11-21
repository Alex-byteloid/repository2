/*********************** Description ************************/



/************************* Includes *************************/

#include "lcd16xx.h"

/********************* Global Variables *********************/

uint8_t I2CData [I2CDataBuferLenght];

uint8_t Status;
uint8_t SendStates;
uint8_t CurrentBuferElement;
uint8_t BuferLenght;

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

void InitI2C1FSM (void){

	SendStates = 0;

}

void ProcessWriteI2C1FSM (void){

	Status = 1;

	switch (SendStates){

	case 0:

		GPIOC->BSRR |= GPIO_BSRR_BS14;

		if (GetMessage(I2CStartTransaction) && (BuferLenght != 0)){
			SendStates = 1;
			GPIOC->BSRR |= GPIO_BSRR_BR15;
		}
		break;

	case 1:
		GPIOC->BSRR |= GPIO_BSRR_BR14;
		I2C1->CR1 |= I2C_CR1_START;						// Sending START I2C1
		SendStates = 2;
		break;

	case 2:

		StartLocTimer(LocTimer1);

			while(Status){

				if (I2C1->SR1 & I2C_SR1_SB){
					Status = 0;
					SendStates = 3;
				}

				if (GetLocTimerVal(LocTimer1) >= 20){
					StopLocTimer(LocTimer1);
					Status = 0;
					SendMessage(I2CStartBitTimeOut);
					SendStates = 5;
				}
			}				// Waiting for the end of sending start
		break;

	case 3:

		(void) I2C1->SR1;

		I2C1->DR = AddrDevice;

		StartLocTimer(LocTimer1);
		Status = 1;

			while(Status){

				if (I2C1->SR1 & I2C_SR1_ADDR){
					Status = 0;
					SendStates = 4;
				}

				if (GetLocTimerVal(LocTimer1) >= 40){
					Status = 0;
					SendMessage(I2CAddrTimeOut);
					SendStates = 5;
				}
			}

			(void) I2C1->SR1;
			(void) I2C1->SR2;

		StopLocTimer(LocTimer1);
		break;

	case 4:

		StartLocTimer(LocTimer1);

		Status = 1;

		while (Status){

			if (GetLocTimerVal(LocTimer1) >= 3){
				I2C1->DR = I2CData[CurrentBuferElement];

				CurrentBuferElement++;
				Status = 0;
			}

		}

		StopLocTimer(LocTimer1);


/*		if (CurrentBuferElement == 2){
			Status = 1;
			StartLocTimer(LocTimer1);
			while (Status){
				if (GetLocTimerVal(LocTimer1) >= 12){
					Status = 0;
				}
			}
			StopLocTimer(LocTimer1);
		}


		StartLocTimer(LocTimer1);

			while(Status){

				if (I2C1->SR1 & I2C_SR1_BTF){
					Status = 0;
				}

				if (GetLocTimerVal(LocTimer1) >= 40){
					Status = 0;
					SendMessage(I2CDataSendTimeOut);
					I2C1->CR1 |= I2C_CR1_STOP;
				break;
				}
			}
*/

		if (CurrentBuferElement == BuferLenght){
			I2C1->CR1 |= I2C_CR1_STOP;
			CurrentBuferElement = 0;
			BuferLenght = 0;
			SendStates = 0;
		}
		break;

	case 5:

		if(GetMessage(I2CAddrTimeOut)){
			GPIOC->BSRR |= GPIO_BSRR_BS15;
			SendStates = 0;
		}

		if(GetMessage(I2CStartBitTimeOut)){
			GPIOC->BSRR |= GPIO_BSRR_BS15;
			SendStates = 0;
		}

		if(GetMessage(I2CDataSendTimeOut)){
			GPIOC->BSRR |= GPIO_BSRR_BS15;
			SendStates = 0;
		}
		break;

	}

}

void LCDInit (void){

	I2CData[0] = 0x3C;
	I2CData[1] = 0x38;
	I2CData[2] = 0x3C;
	I2CData[3] = 0x38;
	I2CData[4] = 0x3C;
	I2CData[5] = 0x38;

	I2CData[6] = 0x2C;
	I2CData[7] = 0x28;

	I2CData[8] = 0x2C;
	I2CData[9] = 0x28;
	I2CData[10] = 0xCC;
	I2CData[11] = 0xC8;

	I2CData[12] = 0x0C;
	I2CData[13] = 0x08;
	I2CData[14] = 0x8C;
	I2CData[15] = 0x88;

	I2CData[16] = 0x0C;
	I2CData[17] = 0x08;
	I2CData[18] = 0x8C;
	I2CData[19] = 0x88;

	I2CData[20] = 0x0C;
	I2CData[21] = 0x08;
	I2CData[22] = 0x6C;
	I2CData[23] = 0x68;

	I2CData[24] = 0x0C;
	I2CData[25] = 0x08;
	I2CData[26] = 0xCC;
	I2CData[27] = 0xC8;

	I2CData[28] = 0x0C;
	I2CData[29] = 0x08;
	I2CData[30] = 0x1C;
	I2CData[31] = 0x18;

	CurrentBuferElement = 0;
	BuferLenght = 30;

	SendMessage(I2CStartTransaction);

}


/*************************	 IRQ_Handler (Обработчики прерываний)	*************************/

