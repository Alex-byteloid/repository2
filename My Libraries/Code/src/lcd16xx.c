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

uint8_t lcdStates;
uint8_t _lcdStates;
uint8_t lcdEntry;


/*************************	 Функции для дисплея 16xx (16xx Display function)	*************************/

void InitLcdFSM (void){

	lcdStates = 0;
	_lcdStates = 0;
}

void ProcessLcdFSM (void){

	if (lcdStates != _lcdStates) lcdEntry = 1; else lcdEntry = 0;
	_lcdStates = lcdStates;

	switch (lcdStates){

	case 0:
		if (GetMessage(LCDStartInit)){

			ClearI2C1DataBufer();
			lcdStates = 1;
		}
		break;

	case 1:

		if (lcdEntry == 1){
			uint16_t LenghtPackage = 4;
			uint16_t StartAdressPackage = 0;
			SendMessage(I2C1StartTransaction, LenghtPackage, StartAdressPackage);
		}
		break;

	case 2:


		break;

	case 3:

		break;

	}
}

