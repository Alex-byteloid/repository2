/*********************** Description ************************/



/************************* Includes *************************/

#include "WS2812.h"

/********************* Global Variables *********************/

uint8_t stateWS281x;
uint8_t _stateWS281x;
uint8_t entryWS281x;


struct RingBuffer{
	uint16_t Push;						// Текущая позиция вставки
	uint16_t Out;						// Текущая позиция выборки
	uint16_t Size;						// Размер кольцевого буфера
	uint16_t Count;						// Количество байт, сохранённых в буфере на текущий момент
}RingBufferWS281x;

uint8_t BufferDMAforWS [240];

uint32_t LedStrip [NumberOfLeds];
uint32_t SendLed;

uint16_t DmaBufer0 [24];
uint16_t DmaBufer1 [24];

uint8_t Bufer0Pointer;
uint8_t Bufer1Pointer;



/*const struct*/ RGBColorType Red = {128, 0, 0};
/*const struct*/ RGBColorType Orange = {128, 34, 0};
/*const struct*/ RGBColorType Yellow = {128, 128, 0};
/*const struct*/ RGBColorType Green = {0, 128, 0};
/*const struct*/ RGBColorType DeepSkyBlue = {0, 95, 128};
/*const struct*/ RGBColorType NavyBlue = {0, 0, 128};
/*const struct*/ RGBColorType Violet = {74, 0, 105};
/*const struct*/ RGBColorType DeepPink = {128, 0, 128};
/*const struct*/ RGBColorType White = {128, 128, 128};
/*const struct*/ RGBColorType Black = {0, 0, 0};

/*************************	 Code	*************************/

/*************************	Блок функций инициализации аппаратной части	 *************************/

/* Функция инициализации таймера для формирования импульсов */

void InitTIM3 (void){

	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;						// Включение тактирования Tim3

	GPIOA->OTYPER &= ~GPIO_OTYPER_OT6;						// Настройка порта PA6 на выход альтернативной функции в режиме пуш-пулл
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED6;					// Скорость - высокая
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD6;						// Без подтяжки к плюсу/минусу питания
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL6_1;					// Назначение альтернативной функции таймера 3 канала 1, порту PA6
	GPIOA->MODER &= ~GPIO_MODER_MODE6;
	GPIOA->MODER |= GPIO_MODER_MODE6_1;

	TIM3->PSC &= ~TIM_PSC_PSC;								// Предделитель частоты равен 0, т.е. частота не делится
	TIM3->ARR = 120;										// Период длительности импульса (значение перезагрузки счётчика)

	TIM3->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;		// Насторйка таймера на режим PWM1
	TIM3->CCMR1 |= TIM_CCMR1_OC1PE;							// Предзагрузка регистра CCR1

	TIM3->DIER |= TIM_DIER_CC1DE;							// Разрешаем запросы для DMA от таймера

	TIM3->CR1 |= TIM_CR1_ARPE;								// Включаем автоперезагрузку по границе счета

	TIM3->CR2 &= ~TIM_CR2_MMS;								// Таймер в основном режиме (Master mode). Событие обновления счетчика выбирается
	TIM3->CR2 |= TIM_CR2_MMS_1;								// в качестве выхода триггера

	TIM3->CCER |= TIM_CCER_CC1E;							// Включение выхода сигнала таймера на соответствующий вывод (PA6)

	TIM3->EGR |= TIM_EGR_UG;								// Генерируем событие обновления для инициализации регистров таймера
//	TIM3->CR1 |= TIM_CR1_CEN;								// Включаем таймер
}

/* Функция инициализации контроллера ПДП для 1-го канала 3-го таймера */

void InitDMAforTIM3 (void){

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;						// Включаем тактирование DMA1

	DMA1_Stream4->CR |= DMA_SxCR_CHSEL_0;
	DMA1_Stream4->CR |= DMA_SxCR_CHSEL_2;					// Выбираем канал 5, 4 потока DMA для работы с 1 каналом таймера 3

	DMA1_Stream4->PAR = (uint32_t) & TIM3->CCR1;			// Адрес переферийного модуля
	DMA1_Stream4->M0AR = (uint32_t) & DmaBufer0 [0];;		// Адрес первого буфера в памяти
	DMA1_Stream4->M1AR = (uint32_t) & DmaBufer1 [0];		// Адрес второго буфера в памяти

	DMA1_Stream4->NDTR = 24;								// Количество передач DMA

	DMA1_Stream4->CR |= DMA_SxCR_PSIZE_0;					// Размер данных перефирии - полуслово
	DMA1_Stream4->CR |= DMA_SxCR_MSIZE_0;					// Размер данных в памяти - полуслово

	DMA1_Stream4->CR &= ~DMA_SxCR_PINC;						// Не увеличивать адрес памяти в перифирии
	DMA1_Stream4->CR |= DMA_SxCR_MINC;						// Увеличивать адрес в памяти

	DMA1_Stream4->CR |= DMA_SxCR_DIR_0;						// Направление передачи: из памяти в периферию

	DMA1_Stream4->CR |= DMA_SxCR_TCIE;						// Включение генерации прерывания после окончания передачи
	DMA1_Stream4->CR |= DMA_SxCR_DBM;						// Включение режима двойного буфера
	DMA1_Stream4->CR |= DMA_SxCR_CIRC;						// Цикличный режим включен
	DMA1_Stream4->CR &= ~DMA_SxCR_CT;						// Область памяти для первой передачи --> M0AR

//	DMA1_Stream4->CR |= DMA_SxCR_EN;						// Включение DMA
}

void DMA1_Stream4_IRQHandler (void){

	SendLed++;																		// Инкрементируем переменную-счётчик

	if (RingBufferWS281x.Out == 240) RingBufferWS281x.Out = 0;

	if ((DMA1_Stream4->CR & DMA_SxCR_CT) == DMA_SxCR_CT){
		for(uint8_t s = 0; s < 24; s++){
			DmaBufer0[s] = (uint16_t)BufferDMAforWS[RingBufferWS281x.Out];		//
			RingBufferWS281x.Out++;
		}
	}
	else{
		for(uint8_t s = 0; s < 24; s++){
			DmaBufer1[s] = (uint16_t)BufferDMAforWS[RingBufferWS281x.Out];		//
			RingBufferWS281x.Out++;
		}
	}

	DMA1->HIFCR |= DMA_HIFCR_CTCIF4;												// Сбрасываем флаг прерывания
}
/*************************	Конец блока функций инициализации аппаратной части	*************************/

/*************************	Блок функций для описания КА WS28xx	*************************/

void InitWS281xFSM (void){
	InitTIM3();
	InitDMAforTIM3();

	SendLed = 0;
	stateWS281x = 0;
	_stateWS281x = 0;
}

void ProcessWS281xFSM (void){

	if(stateWS281x != _stateWS281x) entryWS281x = 1; else entryWS281x = 0;
	_stateWS281x = stateWS281x;

	switch(stateWS281x){

	case 0:
		if (GetMessage(WSLedStart)){
			stateWS281x = 1;
		}
		break;

	case 1:
		if (entryWS281x == 1){
			InsertToRingBufferForDMA(Black);
			InsertToRingBufferForDMA(Black);
			InsertToRingBufferForDMA(Red);
			InsertToRingBufferForDMA(Red);
			DMA1_Stream4->CR |= DMA_SxCR_EN;
			TIM3->EGR |= TIM_EGR_UG;
			TIM3->CR1 |= TIM_CR1_CEN;
		}
		stateWS281x = 2;
		break;

	case 2:
		if (SendLed< 32){
			InsertToRingBufferForDMA(Red);
			InsertToRingBufferForDMA(Red);
			InsertToRingBufferForDMA(Red);
			InsertToRingBufferForDMA(Red);
		}else {
			/*** Очищаем все флаги прерываний DMA контроллера ***/
			DMA1->HIFCR |= DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4 | DMA_HIFCR_CTCIF4 | DMA_HIFCR_CHTIF4 | DMA_HIFCR_CTEIF4;
			DMA1->HIFCR &= ~DMA_HIFCR_CDMEIF4 | ~DMA_HIFCR_CFEIF4 | ~DMA_HIFCR_CTCIF4 | ~DMA_HIFCR_CHTIF4 | ~DMA_HIFCR_CTEIF4;
			TIM3->DIER &= ~TIM_DIER_CC1DE;										// Запрещаем запросы к DMA от таймера
			DMA1_Stream4->CR &= ~DMA_SxCR_EN;									// Отключаем DMA
			stateWS281x = 3;
		}
		break;

	case 3:
		if (entryWS281x == 1){
			TIM3->CR1 &= ~TIM_CR1_CEN;											// Отключаем таймер TIM3
		}
		break;

	}

}

/*************************	Конец блока функций для описания КА WS28xx	*************************/

/*************************	Блок функций для работы с цветами и буфером	*************************/

void InsertToRingBufferForDMA (RGBColorType Color){

	uint32_t GRB = 0;

	GRB |= Color.Green;
	GRB <<= 8;
	GRB |= Color.Red;
	GRB <<= 8;
	GRB |= Color.Blue;

			for (int8_t count = 0; count < 24;){

					if ((GRB & 0x800000) == 0x800000){
						BufferDMAforWS[RingBufferWS281x.Push] = 82;
					}
					else{
						BufferDMAforWS[RingBufferWS281x.Push] = 39;
					}

					GRB = GRB << 1;

					RingBufferWS281x.Push++;
					count++;

					if (RingBufferWS281x.Push == 240) RingBufferWS281x.Push = 0;
			}
}

/*************************  Конец блока функций для работы с цветами и буфером	*************************/
