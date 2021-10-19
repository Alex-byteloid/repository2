
/*********************** Description ************************/



/************************* Includes *************************/

#include "main.h"

/********************* Global Variables *********************/



/*************************	 Code	*************************/

int main (void){

	InitRCC();
	InitGPIO();

	while(1){

		GPIOC->BSRR |= GPIO_BSRR_BR13;
		for(uint32_t i = 0; i < 10000000; i++);
		GPIOC->BSRR |= GPIO_BSRR_BS13;
		for(uint32_t d = 0; d < 10000000; d++);

	}

}
