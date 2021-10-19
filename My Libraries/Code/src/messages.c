/*********************** Description ************************/



/************************* Includes *************************/

#include "messages.h"

/********************* Global Variables *********************/

char Messages[MaxNumbMessages];

/*************************	 Code	*************************/

void InitMessage (void){

	int i;

	for (i = 0; i < MaxNumbMessages; i++){
		Messages[i] = '0';
	}
}

void SendMessage (int Msg){

	if (Messages[Msg] == '0'){

		Messages[Msg] = '1';
	}
}

void ProcessMessage (void){

	int i;
	for(i = 0; i < MaxNumbMessages; i++){

		if(Messages[i] == '2')Messages[i]='0';
		if(Messages[i] == '1')Messages[i]='2';
	}
}

char GetMessage (int Msg){

	if(Messages[Msg] == '2'){
		Messages[Msg] = '0';
		return '1';
	}
	return '0';
}
