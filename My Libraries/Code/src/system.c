
/*********************** Description ************************/



/************************* Includes *************************/

#include "system.h"

/********************* Global Variables *********************/



/*************************	 Code	*************************/

void InitRCC (void){

	/*Функция настройки тактовой частоты системной шины процессора на 96 MHz*/

	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	FLASH->ACR |= FLASH_ACR_LATENCY_3WS;					// Настройка задержки чтения флеш памяти в зависимости от частоты МК

	PWR->CR |= PWR_CR_VOS;									// Настройка внутреннего регулятора напряжения на режим 1 (Scale 1)

	RCC->CR |= RCC_CR_HSEON;								// Включение HSE

	while(!(RCC->CR & RCC_CR_HSERDY));						// Ожидание готовности HSE

	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;					// Выбор генератора HSE в качестве источника тактирования PLL

	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;						// Обнуляем регистр делителя PLL
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLM_0;						// Значение делителя PLL равно 25 (PLLM = 25)
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLM_3;
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLM_4;

	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;						// Обнуляем регистр множителя PLL
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_6;						// Значение множителя PLL равно 192
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_7;

	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;						// Обнуляем регист делителя на выходе PLL, значение делителя на выходе равно 2

	RCC->CR |= RCC_CR_PLLON;								// Включаем модуль PLL

	while ((RCC->CR & RCC_CR_PLLRDY) == 0);					// Ожидание готовности модуля PLL

	RCC->CFGR &= ~RCC_CFGR_HPRE;							// Частота шины AHB = 96 MHz

	RCC->CFGR &= ~RCC_CFGR_PPRE1;
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;						// Частота шины APB1 = 48 MHz

	RCC->CFGR &= ~RCC_CFGR_PPRE2;
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;						// Частота шины APB2 = 96 MHz

	RCC->CFGR &= ~RCC_CFGR_SW;								// Обнуляем регистр выбора источника тактирования системной шины
	RCC->CFGR |= RCC_CFGR_SW_PLL;							// Выбираем в качестве источника тактирования системной шины PLL

	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);	// Ожидание установки модуля PLL в качестве источника частоты сист. шины

	RCC->APB1ENR |= RCC_APB1ENR_PWREN;						// Вкдючение тактирования Power interface

	SystemCoreClockUpdate();

}

void UpdateNVICActiveAndPriority (void){

	__NVIC_SetPriorityGrouping(4);											// 16 групп прерываний и 16 подгрупп

//	__NVIC_EnableIRQ(USART6_IRQn);											// Разрешаем прерывания от USART для Modbus
	__NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);									// Разрешаем прерывания от 10-го таймера
	__NVIC_EnableIRQ(DMA1_Stream4_IRQn);									// Зазрешаем прерывание от 4 потока ПДП контроллера обслуживающего таймер №3

//	__NVIC_EnableIRQ(DMA2_Stream7_IRQn);									// Разрешаем прерывания от DMA обслуживающего USART Modbus
//	__NVIC_EnableIRQ(I2C1_EV_IRQn);											// Разрешаем прерывания событий I2C1

//	__NVIC_SetPriority(USART6_IRQn, NVIC_EncodePriority(1, 0, 2));			// Устанавливаем Modbus прерывание в первую группу, первым приоритетом
	__NVIC_SetPriority(TIM1_UP_TIM10_IRQn, NVIC_EncodePriority(2, 1, 3));	// Прервания 10го таймера за прерыванием Modbus
	__NVIC_SetPriority(DMA1_Stream4_IRQn, NVIC_EncodePriority(2, 0, 0));
	//	__NVIC_SetPriority(DMA2_Stream7_IRQn, NVIC_EncodePriority(1, 1, 3));
//	__NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(3, 2, 3));

}

void InitGPIO (void){

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;					// Включение тактирования порта ввода-вывода C
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;					// Включение тактирования порта ввода-вывода B
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;					// Включение тактирования порта ввода-вывода A

	GPIOC->OTYPER &= ~GPIO_OTYPER_OT13;						// Настройка порта PC13 на выход push-pull
	GPIOC->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR13;
	GPIOC->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR13;				// Настройка на высокую выходную скорость
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD13;						// No pull, no down (без подтяжки к плюсу или минусу питания)
	GPIOC->MODER &= ~GPIO_MODER_MODE13;
	GPIOC->MODER |= GPIO_MODER_MODE13_0;					// Настройка порта PC13 на выход

	GPIOC->OTYPER &= ~GPIO_OTYPER_OT14;						// Настройка порта PC14 на выход push-pull
	GPIOC->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR14;
	GPIOC->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR14;				// Настройка на высокую выходную скорость
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD14;						// No pull, no down (без подтяжки к плюсу или минусу питания)
	GPIOC->MODER &= ~GPIO_MODER_MODE14;
	GPIOC->MODER |= GPIO_MODER_MODE14_0;					// Настройка порта PC14 на выход

	GPIOC->OTYPER &= ~GPIO_OTYPER_OT15;						// Настройка порта PC15 на выход push-pull
	GPIOC->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR15;
	GPIOC->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR15;				// Настройка на высокую выходную скорость
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD15;						// No pull, no down (без подтяжки к плюсу или минусу питания)
	GPIOC->MODER &= ~GPIO_MODER_MODE15;
	GPIOC->MODER |= GPIO_MODER_MODE15_0;					// Настройка порта PC15 на выход
}
