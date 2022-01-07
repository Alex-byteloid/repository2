/*********************** Description ************************/



/************************* Includes *************************/

#include "WS2812.h"

/********************* Global Variables *********************/

struct Pointer{
	uint16_t StepOfStairs;
	uint16_t LedOfStep;
}StairsPointer;

uint32_t GRB;

uint8_t stateWS281x;
uint8_t _stateWS281x;
uint8_t entryWS281x;

uint32_t Stairs [NumberOfStairs][NumberOfLeds];

uint16_t DmaBufer0 [24];
uint16_t DmaBufer1 [24];

const RGBColorType Red = {128, 0, 0};
const RGBColorType Orange = {128, 34, 0};
const RGBColorType Yellow = {128, 128, 0};
const RGBColorType Green = {0, 128, 0};
const RGBColorType DeepSkyBlue = {0, 95, 128};
const RGBColorType NavyBlue = {0, 0, 128};
const RGBColorType Violet = {74, 0, 105};
const RGBColorType DeepPink = {128, 0, 128};
const RGBColorType White = {128, 128, 128};
const RGBColorType Black = {0, 0, 0};

/*************************	 Code	*************************/

/*************************	Блок функций инициализации аппаратной части	 *************************/

/* Функция инициализации таймера для формирования импульсов */

void InitTIM3 (void){

	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;						// Включение тактирования Tim3

	GPIOA->OTYPER &= ~GPIO_OTYPER_OT6;						// Настройка порта PA6 на выход альтернативной функции в режиме пуш-пулл
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED6;					// Скорость - высокая
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL6_1;					// Назначение альтернативной функции таймера 3 канала 1, порту PA6
	GPIOA->MODER &= ~GPIO_MODER_MODE6;
	GPIOA->MODER |= GPIO_MODER_MODE6_1;

	TIM3->PSC &= ~TIM_PSC_PSC;								// Предделитель частоты равен 0, т.е. частота не делится
	TIM3->ARR = 120;										// Период длительности импульса (значение перезагрузки счётчика)

	TIM3->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;		// Насторйка таймера на режим PWM2
	TIM3->CCMR1 |= TIM_CCMR1_OC1PE;							// Предзагрузка регистра CCR1
	TIM3->CCER  &= ~TIM_CCER_CC1P;							// Активный уровень - высокий (OC1 active HIGH)

	TIM3->DIER |= TIM_DIER_CC1DE;							// Разрешаем запросы для DMA от таймера

	TIM3->CR1 |= TIM_CR1_ARPE;								// Включаем автоперезагрузку по границе счета

	TIM3->CR2 &= ~TIM_CR2_MMS;								// Таймер в основном режиме (Master mode). Событие обновления счетчика выбирается
	TIM3->CR2 |= TIM_CR2_MMS_1;								// в качестве выхода триггера

	TIM3->CCER |= TIM_CCER_CC1E;							// Включение выхода сигнала таймера на соответствующий вывод (PA6)

	TIM3->EGR |= TIM_EGR_UG;								// Генерируем событие обновления для инициализации регистров таймера
}

/* Функция инициализации контроллера ПДП для 1-го канала 3-го таймера */

void InitDMAforTIM3 (void){

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;						// Включаем тактирование DMA1

	DMA1_Stream4->CR |= DMA_SxCR_CHSEL_0;
	DMA1_Stream4->CR |= DMA_SxCR_CHSEL_2;					// Выбираем канал 5, 4 потока DMA для работы с 1 каналом таймера 3

	DMA1_Stream4->PAR = (uint32_t) & TIM3->CCR1;			// Адрес переферийного модуля
	DMA1_Stream4->M0AR = (uint32_t) & DmaBufer0[0];			// Адрес первого буфера в памяти
	DMA1_Stream4->M1AR = (uint32_t) & DmaBufer1[0];			// Адрес второго буфера в памяти

	DMA1_Stream4->NDTR = 24;								// Количество передач DMA

	DMA1_Stream4->CR |= DMA_SxCR_PSIZE_0;					// Размер данных перефирии - полуслово
	DMA1_Stream4->CR |= DMA_SxCR_MSIZE_0;					// Размер данных в памяти - полуслово

	DMA1_Stream4->CR &= ~DMA_SxCR_PINC;						// Не увеличивать адрес памяти в перифирии
	DMA1_Stream4->CR |= DMA_SxCR_MINC;						// Увеличивать адрес в памяти

	DMA1_Stream4->CR |= DMA_SxCR_DIR_0;						// Направление передачи: из памяти в периферию

	DMA1_Stream4->CR |= DMA_SxCR_TCIE;						// Включение генерации прерывания после окончания передачи
	DMA1_Stream4->CR |= DMA_SxCR_HTIE;						// Включение генерации прерывания после половины передачи
	DMA1_Stream4->CR |= DMA_SxCR_DBM;						// Включение режима двойного буфера
	DMA1_Stream4->CR |= DMA_SxCR_CIRC;						// Цикличный режим включен
	DMA1_Stream4->CR &= ~DMA_SxCR_CT;						// Область памяти для первой передачи --> M0AR

}

void DMA1_Stream4_IRQHandler (void){

		if (DMA1->HISR & DMA_HISR_HTIF4){
			++StairsPointer.LedOfStep;
			if (StairsPointer.LedOfStep == NumberOfLeds){
				StairsPointer.LedOfStep = 0;
				++StairsPointer.StepOfStairs;
			}

			if ((StairsPointer.StepOfStairs < NumberOfStairs)&&(StairsPointer.LedOfStep < NumberOfLeds)){
				if(DMA1_Stream4->CR & DMA_SxCR_CT){

					GRB = Stairs[StairsPointer.StepOfStairs][StairsPointer.LedOfStep];

					for (int8_t count = 0; count < 24; count++){
						if ((GRB & 0x800000) == 0x800000){
							DmaBufer0[count] = 82;
						}else{
							DmaBufer0[count] = 39;
						}
						GRB = GRB << 1;
					}
				}else {

					GRB = Stairs[StairsPointer.StepOfStairs][StairsPointer.LedOfStep];

					for (int8_t count = 0; count < 24; count++){

						if ((GRB & 0x800000) == 0x800000){
							DmaBufer1[count] = 82;
						}else{
							DmaBufer1[count] = 39;
						}
						GRB = GRB << 1;
					}
				}
			}
		DMA1->HIFCR |= DMA_HIFCR_CHTIF4;										// Сбрасываем флаг прерывания по половине передачи
	}

	if (DMA1->HISR & DMA_HISR_TCIF4){

		if (StairsPointer.StepOfStairs == NumberOfStairs){
			SendMessage(WS28EndOfTransfer, 0, 0);
			TIM3->DIER &= ~TIM_DIER_CC1DE;										// Отключение запросов к DMA от таймера
			TIM3->CR1 &= ~TIM_CR1_CEN;											// Отключение таймера
		}

		DMA1->HIFCR |= DMA_HIFCR_CTCIF4;										// Сбрасываем флаг прерывания по окончанию передачи
	}
}
/*************************	Конец блока функций инициализации аппаратной части	*************************/

/*************************	Блок функций для описания КА WS28xx	*************************/

void InitWS281xFSM (void){
	InitTIM3();
	InitDMAforTIM3();

	SendingCountForEffect = 0;
	StairsPointer.LedOfStep = 0;
	StairsPointer.StepOfStairs = 0;
	GRB = 0;

	stateWS281x = 0;
	_stateWS281x = 0;

	stateEffectsWS28 = 0;
	_stateEffectsWS28 = 0;
}

void ProcessWS281xFSM (void){

	if(stateWS281x != _stateWS281x) entryWS281x = 1; else entryWS281x = 0;
	_stateWS281x = stateWS281x;

	switch(stateWS281x){

	case 0:
		if (GetMessage(WS28LedStart)){
			ClearStairsBuffer();
			stateWS281x = 1;
		}
		break;

	case 1:
		if (entryWS281x == 1){
			InsertColorToAllStairsBuffer(Black);
		}
		stateWS281x = 2;
		break;

	case 2:
		if (entryWS281x == 1){
			TIM3->CCR1 = 0;
			TIM3->CR1 |= TIM_CR1_CEN;
		}else{
			TIM3->CR1 &= ~TIM_CR1_CEN;
			stateWS281x = 3;
}
		break;

	case 3:
		if (entryWS281x == 1){

			GRB = Stairs[0][0];
			for (int8_t count = 0; count < 24; count++){
				if ((GRB & 0x800000) == 0x800000){
				DmaBufer0[count] = 82;
				}else{
				DmaBufer0[count] = 39;
				}
			GRB = GRB << 1;
			}

			DMA1_Stream4->CR &= ~DMA_SxCR_CT;									// Область памяти для первой передачи --> M0AR
			DMA1_Stream4->CR |= DMA_SxCR_EN;									// Включение DMA
			TIM3->DIER |= TIM_DIER_CC1DE;										// Включение запросов к DMA от таймера TIM3
			TIM3->CR1 |= TIM_CR1_CEN;											// Включение таймера TIM3
		}

		if (GetMessage(WS28EndOfTransfer)){
			DMA1->HIFCR |= DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4
						|  DMA_HIFCR_CTCIF4  | DMA_HIFCR_CHTIF4 				// Очищаем все флаги прерываний DMA контроллера
						|  DMA_HIFCR_CTEIF4;

			DMA1_Stream4->CR &= ~DMA_SxCR_EN;									// Отключение DMA
			StairsPointer.LedOfStep = 0;										// Очистка переменных-указателей буфера отправки WS281x
			StairsPointer.StepOfStairs = 0;										// Очистка переменных-указателей буфера отправки WS281x
			stateWS281x = 4;
		}
		break;

	case 4:

		if (entryWS281x == 1){
			SendMessage(WS28TransactionComplete, 0, 0);
		}

		if (GetMessage(WS28LedStart)){
			stateWS281x = 2;
		}

		break;
	}

}

/*************************	Конец блока функций для описания КА WS28xx	*************************/

/*************************	Блок функций для работы с цветами и буфером	*************************/

void ClearStairsBuffer (void){

	for (uint8_t StairsCount = 0; StairsCount < NumberOfStairs; StairsCount++){

		for (uint8_t LedCount = 0; LedCount < NumberOfLeds; LedCount++){
			Stairs [StairsCount][LedCount] = 0;
		}
	}
}

void InsertColorToONELEDStairsBuffer (RGBColorType Color, uint16_t StairsNumber, uint16_t LedNumber){

	--StairsNumber;
	--LedNumber;

	Stairs [StairsNumber][LedNumber] |= Color.Green;
	Stairs [StairsNumber][LedNumber] <<= 8;
	Stairs [StairsNumber][LedNumber] |= Color.Red;
	Stairs [StairsNumber][LedNumber] <<= 8;
	Stairs [StairsNumber][LedNumber] |= Color.Blue;
}

void InsertColorToMULTIPLELEDStairsBuffer (RGBColorType Colores, uint16_t StairsNumber, uint16_t LedNumberLeft, uint16_t LedNumberRight){

	uint16_t LedCount;

	LedCount = (LedNumberRight - LedNumberLeft) + 1;

	--StairsNumber;
	--LedNumberLeft;
	--LedNumberRight;

	for (uint16_t i = 0; i < LedCount; i++){
		Stairs [StairsNumber][LedNumberLeft] |= Colores.Green;
		Stairs [StairsNumber][LedNumberLeft] <<= 8;
		Stairs [StairsNumber][LedNumberLeft] |= Colores.Red;
		Stairs [StairsNumber][LedNumberLeft] <<= 8;
		Stairs [StairsNumber][LedNumberLeft] |= Colores.Blue;
		++LedNumberLeft;
	}
}

void InsertColorToAllStairsBuffer (RGBColorType Coloress){

	for (uint16_t StairsCount = 1; StairsCount <= NumberOfStairs; StairsCount++){

		for (uint16_t LedCount = 1; LedCount <= NumberOfLeds; LedCount++){
			InsertColorToONELEDStairsBuffer(Coloress, StairsCount, LedCount);
		}
	}
}

/*************************  Конец блока функций для работы с цветами и буфером	*************************/
