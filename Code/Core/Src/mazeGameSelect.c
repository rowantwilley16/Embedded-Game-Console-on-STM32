/*
 * mazeGameSelect.c
 *
 *  Created on: 12 Jun 2021
 *      Author: Rowan
 */
#include "main.h"
#include "mazeGame.h"
#include "UserGeneral.h"
#include "mazeGameSelect.h"
typedef struct button
{
	uint8_t needsAction;
	uint16_t debounce;
	GPIO_TypeDef *port;
	uint16_t pin;
}Button;

typedef struct Maze
{
	uint8_t row[8];
	uint8_t col[8];

}Maze;

Maze Maze1;

extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;

extern uint8_t arrRow[8];
extern uint8_t arrCol[8];

uint8_t mazeNumber = 1;
uint8_t enterMazeFlag =0;

extern Button ButtonM ;
extern Button ButtonL ;
extern Button ButtonR ;
extern Button ButtonD ;
extern Button ButtonU ;

extern uint8_t screenUpdateFlag ;
extern uint8_t screenCount;

extern uint8_t initilizeMazeGameFlag;

extern uint8_t ballx,bally;
extern uint8_t flag_8ms ;
extern uint8_t flag_1ms ;
uint32_t sliderInput;

extern uint8_t flag_100ms;
extern uint8_t flag_300ms ;

extern uint8_t counter_100ms ;
extern uint8_t counter_300ms ;

extern uint8_t inMazeGameFlag;


void mazeSelect(){

	clearScreen();
	updateScreen();

	if (ButtonU.needsAction){
		if (mazeNumber<4){
			mazeNumber ++;
		}
		ButtonU.needsAction=0;

	}
	if (ButtonD.needsAction){
		if (mazeNumber>=2){
		mazeNumber --;
		}
		ButtonD.needsAction=0;
	}
	if (ButtonM.needsAction){
		enterMazeFlag=1;
		ButtonM.needsAction=0;
	}
	switch (mazeNumber) {

	case 1:
		colomnScanNum1();
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET); //debug 1
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); //debug 2
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); //debug 3
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); //debug 4
			break ;
	case 2:
		colomnScanNum2();
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); //debug 1
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); //debug 2
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); //debug 3
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); //debug 4
			break ;
	case 3:
		colomnScanNum3();
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); //debug 1
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); //debug 2
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET); //debug 3
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); //debug 4
			break ;
	case 4:
		colomnScanNum4();
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); //debug 1
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); //debug 2
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); //debug 3
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET); //debug 4
			break ;

	}
	writeMazeToArray();
	updateScreen();
}

void colomnScanNum1(){

	switch (flag_8ms)
	{
	case 0 :resetElements();
		Maze1.col[0] = 0;
		break;


	case 1 :resetElements();
		Maze1.col[1] = 0;
		break;

	case 2 :resetElements();
		Maze1.col[2] = 0;
		break;


	case 3 :resetElements();
		Maze1.col[3] = 1;
		Maze1.row[1] = 1;
		Maze1.row[2] = 1;
		Maze1.row[3] = 1;
		Maze1.row[4] = 1;
		Maze1.row[5] = 1;
		Maze1.row[6] = 1;
		break ;

	case 4 :resetElements();
		Maze1.col[4] = 0;
		break ;


	case 5 :resetElements();
		Maze1.col[5] = 0;
		break ;

	case 6 :resetElements();
		Maze1.col[6] = 0;
		break ;

	case 7 :resetElements();
		Maze1.col[7] = 0;
		break ;
	}
}
void colomnScanNum2(){

	switch (flag_8ms)
	{
	case 0 :resetElements();
		Maze1.col[0] = 0;
		break;


	case 1 :resetElements();
		Maze1.col[1] = 0;
		break;

	case 2 :resetElements();
		Maze1.col[2] = 1;
		Maze1.row[2] = 1;
		Maze1.row[6] = 1;
		break;


	case 3 :resetElements();
		Maze1.col[3] = 1;
		Maze1.row[1] = 1;
		Maze1.row[5] = 1;
		Maze1.row[6] = 1;
		break ;

	case 4 :resetElements();
		Maze1.col[4] = 1;
		Maze1.row[1] = 1;
		Maze1.row[4] = 1;
		Maze1.row[6] = 1;
		break ;


	case 5 :resetElements();
		Maze1.col[5] = 1;
		Maze1.row[2] = 1;
		Maze1.row[3] = 1;
		Maze1.row[6] = 1;
		break ;

	case 6 :resetElements();
		Maze1.col[6] = 0;
		break ;

	case 7 :resetElements();
		Maze1.col[7] = 0;
		break ;
	}
}
void colomnScanNum3(){

	switch (flag_8ms)
	{
	case 0 :resetElements();
		Maze1.col[0] = 0;
		break;


	case 1 :resetElements();
		Maze1.col[1] = 0;
		break;

	case 2 :resetElements();
		Maze1.col[2] = 1;
		Maze1.row[1] = 1;
		Maze1.row[3] = 1;
		Maze1.row[6] = 1;
		break;


	case 3 :resetElements();
		Maze1.col[3] = 1;
		Maze1.row[1] = 1;
		Maze1.row[3] = 1;
		Maze1.row[6] = 1;
		break ;

	case 4 :resetElements();
		Maze1.col[4] = 1;
		Maze1.row[2] = 1;
		Maze1.row[4] = 1;
		Maze1.row[5] = 1;
		break ;


	case 5 :resetElements();
		Maze1.col[5] = 0;
		break ;

	case 6 :resetElements();
		Maze1.col[6] = 0;
		break ;

	case 7 :resetElements();
		Maze1.col[7] = 0;
		break ;
	}
}
void colomnScanNum4(){

	switch (flag_8ms)
	{
	case 0 :resetElements();
		Maze1.col[0] = 0;
		break;


	case 1 :resetElements();
		Maze1.col[1] = 0;
		break;

	case 2 :resetElements();
		Maze1.col[2] = 1;
		Maze1.row[5] = 1;
		break;


	case 3 :resetElements();
		Maze1.col[3] = 1;
		Maze1.row[4] = 1;
		Maze1.row[5] = 1;
		break ;

	case 4 :resetElements();
		Maze1.col[4] = 1;
		Maze1.row[3] = 1;
		Maze1.row[5] = 1;
		break ;


	case 5 :resetElements();
		Maze1.col[5] = 1;
		Maze1.row[2] = 1;
		Maze1.row[4] = 1;
		Maze1.row[5] = 1;
		Maze1.row[6] = 1;
		break ;

	case 6 :resetElements();
		Maze1.col[6] = 1;
		Maze1.row[1] = 1;
		Maze1.row[5] = 1;
		break ;

	case 7 :resetElements();
		Maze1.col[7] = 0;
		break ;
	}
}

