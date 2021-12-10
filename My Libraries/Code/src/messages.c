/*********************** Description ************************/



/************************* Includes *************************/

#include "messages.h"

/********************* Global Variables *********************/

typedef struct {
	char Msg;
	uint16_t ParamOne;
	uint16_t ParamTwo;
}MSG;

MSG Messages [MaxNumbMessages];

/*************************	 Code	*************************/

void InitMessage (void){

	int i;

	for (i = 0; i < MaxNumbMessages; i++){
		Messages[i].Msg = '0';
		Messages[i].ParamOne = 0;
		Messages[i].ParamTwo = 0;
	}
}

void SendMessage (uint16_t Message, uint16_t ParametrOne, uint16_t ParametrTwo){

	if (Messages[Message].Msg == '0'){

		Messages[Message].Msg = '1';
		Messages[Message].ParamOne = ParametrOne;
		Messages[Message].ParamTwo = ParametrTwo;
	}
}

void ProcessMessage (void){

	for(uint8_t i = 0; i < MaxNumbMessages; i++){

		if(Messages[i].Msg == '2'){
			Messages[i].Msg = '0';
			Messages[i].ParamOne = 0;
			Messages[i].ParamTwo = 0;
		}

		if(Messages[i].Msg == '1')Messages[i].Msg = '2';

		if(Messages[i].Msg == '0'){
			Messages[i].ParamOne = 0;
			Messages[i].ParamTwo = 0;
		}
	}
}

uint8_t GetMessage (uint8_t Message){

	if(Messages[Message].Msg == '2'){
		Messages[Message].Msg = '0';
		return 1;
	}
	return 0;
}
