/*
 * UserGeneral.c
 *
 *  Created on: 12 Apr 2021
 *      Author: Rowan
 */
#include "main.h"
#include "UserGeneral.h"
#include "mazeGame.h"
#include "tennisGame.h"
#include "mazeGameSelect.h"
//Button debounce code, reference : Nicol Visser 16986431
//--------------------------------
#define BUTTON_DEBOUNCE_TIME  20
//--------------------------------
uint8_t ballx,bally;

uint8_t arrRow[8]={0,0,0,0,0,0,0,0};
uint8_t arrCol[8]={0,0,0,0,0,0,0,0};

uint8_t screenCount	= 0;
uint8_t screenUpdateFlag = 0;

uint8_t middleButtonFlag = 0;
uint8_t leftButtonFlag = 0;
uint8_t rightButtonFlag = 0;
uint8_t inMazeGameFlag =0; 		//0 is not in the game, 1 is in the game

uint32_t sliderInput;
uint8_t  UARTCount =0;

uint8_t flag_1ms = 0;
uint8_t flag_8ms = 0;
uint8_t flag_100ms =0;
uint8_t flag_300ms = 0;
uint16_t counterTennisUART=0;
uint8_t counter_100ms =0;
uint16_t counter_300ms =0; 		//can't use 8 bits as 2^8 is only 256

//tennis ball update speed
uint16_t counter_BallUpdate = 0;
uint8_t flag_BallUpdate = 0;
uint16_t ballUpdateTime = 700; 	//starts at 700ms

uint8_t initilizeMazeGameFlag=0;
uint8_t initilizeTennisGameFlag =0;

HAL_StatusTypeDef accRes;
HAL_StatusTypeDef accRes2;

uint8_t i2c_transferData[8] = {0};
uint8_t i2c_accData[8] = {0};

int16_t accx = 0;
int16_t accy = 0;
int16_t accz = 0;

uint16_t i2cAddress = 0x30;
uint8_t ControlRegister1 = 0x20;
uint8_t ControlRegister1Data = 0b01000011;
uint8_t outxl_reg = 0x28 | 0x80;


extern uint8_t enterMazeFlag;

extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;

uint8_t inTennisGameFlag = 0;



//Button debounce code, reference : Nicol Visser 16986431
//-------------------------------------------------------
typedef struct button
{
	uint8_t needsAction;
	uint16_t debounce;
	GPIO_TypeDef *port;
	uint16_t pin;
}Button;

Button ButtonM = {0,0,GPIOB,GPIO_PIN_0}; // middle button
Button ButtonL = {0,0,GPIOA,GPIO_PIN_0}; //left button
Button ButtonR = {0,0,GPIOC,GPIO_PIN_0}; //right button
Button ButtonD = {0,0,GPIOC,GPIO_PIN_3}; //down button
Button ButtonU = {0,0,GPIOC,GPIO_PIN_2}; // up button
//--------------------------------------------------------
void updateButton(Button *btn);


void start(){

	calibrationSequence();
}

void mainloop(){

 	if (flag_1ms){

		updateScreen();
		updateButton(&ButtonM);
		updateButton(&ButtonL);
		updateButton(&ButtonR);
		updateButton(&ButtonD);
		updateButton(&ButtonU);

		selectGame();
		state();
		flag_1ms = 0;
	}

}
//Button debounce code, reference : Nicol Visser 16986431
//-------------------------------------------------------------
void updateButton(Button *btn)
{
	if (HAL_GPIO_ReadPin(btn->port,btn->pin)== GPIO_PIN_SET)
		btn ->debounce = BUTTON_DEBOUNCE_TIME;
	else
	{
		btn ->debounce--;
		if (btn->debounce ==0)
			btn ->needsAction=1;
	}
}
//--------------------------------------------------------------

void selectGame(){
	if ((!inMazeGameFlag)  && (!inTennisGameFlag)){
	if (ButtonL.needsAction)
		{
			inMazeGameFlag=1;
			initilizeMazeGameFlag=1;
			ButtonL.needsAction = 0;
		}
	}

	if ( (!inTennisGameFlag) && (!inMazeGameFlag)){
		if (ButtonM.needsAction){
			inTennisGameFlag=1;
			initilizeTennisGameFlag=1;
			ButtonM.needsAction=0;
		}
	}
}
void state(){

	//if else in this is VERY IMPORTANT ELSE IT GOES WILL NOT LOOP THE CURRENT GAME, will go back to main menu
	if (inMazeGameFlag){
		if (enterMazeFlag==0){
			mazeSelect();
		}

		if (enterMazeFlag==1){
			mazeGameLoop();
		}


	}else
	if (inTennisGameFlag){
		tennisGameLoop();
	}else
	{
		//display the main screen
		clearScreen();
		lightUpCorners();
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); //debug 1
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); //debug 2
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); //debug 3
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); //debug 4
	}

}

void exitGame(){

	clearScreen();
	lightUpCorners();

}

void lightUpCorners(){

	arrRow[0]=1;
	arrRow[7]=1;
	arrCol[0]=1;
	arrCol[7]=1;
	updateScreen();
}
void updateScreen(){

	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, arrRow[0]); 	//row1
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, arrRow[1]);	//row2
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, arrRow[2]); 	//row3
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, arrRow[3]); 	//row4
	 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12,arrRow[4]);	//row5
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12,arrRow[5]); 	//row6
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11,arrRow[6]);	//row7
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, arrRow[7]); 	//row8

	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, arrCol[0]);	//col1
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15,arrCol[1]);	//col2
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,arrCol[2]);	//col3
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13,arrCol[3]); 	//col4
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, arrCol[4]); 	//col5
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10,arrCol[5]); 	//col6
	 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10,arrCol[6]); 	//col7
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, arrCol[7]); 	//col8

	 //reset the flag
	 screenUpdateFlag = 0;

}
void HAL_SYSTICK_Callback(void)
{
	flag_1ms = 1 ;


	flag_8ms ++;
	counter_100ms ++;
	counter_300ms ++;
    counter_BallUpdate ++;
    counterTennisUART ++;

	if (flag_8ms == 8){
		flag_8ms = 0;
	}
	if (counter_100ms==100){
		flag_100ms = 1;
		counter_100ms=0;
	}
	if (counter_300ms == 300){
		flag_300ms = 1;
		counter_300ms =0;
	}
	if (counter_BallUpdate == ballUpdateTime){
		flag_BallUpdate = 1;
	}

	/*
	UARTCount++;

	if (UARTCount == 100){
		sendCoordinatesUART();
		UARTCount = 0;
	}*/
}

/*
void sendCoordinatesUART(){

	if (inMazeGameFlag==1){

		uint8_t positionMsg[] = "$3xx_____\n";
		positionMsg[2] = ballx+48;
		positionMsg[3] = bally+48;
		HAL_UART_Transmit(&huart2,positionMsg,10,100);

	}
}*/

void calibrationSequence(){

		//UART Student Number
		uint8_t StudentNumber[] = "$23105577\n"; // Data to Transmit through uART
		HAL_UART_Transmit(&huart2,StudentNumber,10,100);

	  //calibration Sequence

	  uint8_t message1[] = "$10______\n";
	  HAL_UART_Transmit(&huart2,message1,10,100);

	  //light up col 1
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); //col 1 GPIO
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //R1
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET); //R2
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET); //etc
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);

	  HAL_Delay(1000); // 1 second delay

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);


	  //light up col 2

	  uint8_t message2[] = "$11______\n";
	   HAL_UART_Transmit(&huart2,message2,10,100);

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET); //col 2 GPIO
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //R1
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET); //R2
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET); //etc
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);

	  HAL_Delay(1000); // 1 second delay

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);



	  //light up col 3

	  uint8_t message3[] = "$12______\n";
	     HAL_UART_Transmit(&huart2,message3,10,100);

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); //col 3 GPIO
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //R1
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET); //R2
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET); //etc
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);

	  HAL_Delay(1000); // 1 second delay

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);



	  //light up col 4

	  uint8_t message4[] = "$13______\n";
	     HAL_UART_Transmit(&huart2,message4,10,100);

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13,GPIO_PIN_SET); //col 4 GPIO
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //R1
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET); //R2
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET); //etc
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);

	  HAL_Delay(1000); // 1 second delay

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13,GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);



	  //light up col 5

	  uint8_t message5[] = "$14______\n";
	     HAL_UART_Transmit(&huart2,message5,10,100);

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET); //col 5 GPIO
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //R1
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET); //R2
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET); //etc
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);

	  HAL_Delay(1000); // 1 second delay

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);



	  //light up col 6

	  uint8_t message6[] = "$15______\n";
	     HAL_UART_Transmit(&huart2,message6,10,100);

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET); //col 6 GPIO
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //R1
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET); //R2
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET); //etc
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);

	  HAL_Delay(1000); // 1 second delay

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);


	  //light up col 7

	  uint8_t message7[] = "$16______\n";
	     HAL_UART_Transmit(&huart2,message7,10,100);

	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET); //col 7 GPIO
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //R1
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET); //R2
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET); //etc
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);

	  HAL_Delay(1000); // 1 second delay

	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);



	  //light up col 8
	  uint8_t message8[] = "$17______\n";
	     HAL_UART_Transmit(&huart2,message8,10,100);

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET); //col 8 GPIO
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //R1
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET); //R2
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET); //etc
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);

	  HAL_Delay(1000); // 1 second delay

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);


	  //Light up all 4 Corners

	  //top left
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);

	  //bottom left
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);

	  //top right
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);

	  //bottom right
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);

	  HAL_Delay(1000); // 1 second delay

}




