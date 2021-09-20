/*
 * mazeGame.h
 *
 *  Created on: Apr 12, 2021
 *      Author: Rowan
 */
#include "main.h"
#include "mazeGame.h"
#include "UserGeneral.h"
#include "mazeGameSelect.h"

extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;

const uint8_t layout_M1[8][8] = {
			{0,0,0,0,0,0,1,0},
			{1,1,0,1,1,0,0,0},
			{0,0,0,0,0,0,1,0},
			{0,1,1,1,1,1,0,1},
			{0,0,0,0,1,0,0,0},
			{1,1,0,1,1,0,1,0},
			{0,0,0,1,0,0,1,0},
			{0,1,0,0,0,1,1,0}
	};
const uint8_t layout_M2[8][8] = {
			{0,0,0,0,0,0,0,0},
			{0,1,0,1,0,1,1,0},
			{0,0,1,0,1,0,0,0},
			{0,1,0,0,0,0,1,1},
			{0,0,1,1,1,1,0,0},
			{1,0,1,0,0,1,1,0},
			{0,0,1,0,0,0,1,0},
			{0,0,0,0,1,0,0,0}
	};
const uint8_t layout_M3[8][8] = {
			{0,0,0,1,1,0,0,0},
			{0,1,0,0,1,0,1,0},
			{0,0,1,0,1,0,1,0},
			{1,0,1,0,0,0,1,0},
			{0,0,1,0,1,1,1,0},
			{0,1,0,0,1,0,0,0},
			{0,1,0,1,1,0,1,1},
			{0,0,0,0,1,0,0,0}
	};
const uint8_t layout_M4[8][8] = {
			{0,1,0,1,0,0,0,0},
			{0,1,0,0,0,1,1,0},
			{0,1,1,1,1,0,0,0},
			{0,1,0,0,1,0,1,0},
			{0,1,0,1,0,0,1,0},
			{0,0,0,0,0,1,0,0},
			{1,1,1,1,1,1,0,1},
			{0,0,0,0,0,0,0,0}
	};
typedef struct player{
	uint8_t xpos;
	uint8_t ypos;
	uint8_t state;
}Player;

Player ball = {0,0,1}; //initilise the ball as 1

typedef struct finishGoal{
	uint8_t xpos;
	uint8_t ypos;
	uint8_t state;
}FinishGoal;

FinishGoal endGoal = {7,7,1};

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
Maze Maze2;
Maze Maze3;
Maze Maze4;

extern uint8_t mazeNumber;

extern Button ButtonM ;
extern Button ButtonL ;
extern Button ButtonR ;
extern Button ButtonD ;
extern Button ButtonU ;


extern uint8_t arrRow[8];
extern uint8_t arrCol[8];
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

extern uint8_t enterMazeFlag;

extern HAL_StatusTypeDef accRes;
extern HAL_StatusTypeDef accRes2;

extern uint8_t i2c_transferData[8];
extern uint8_t i2c_accData[8] ;

extern uint16_t i2cAddress ;
extern uint8_t ControlRegister1 ;
extern uint8_t ControlRegister1Data ;
extern uint8_t outxl_reg ;

extern int16_t accx ;
extern int16_t accy ;
extern int16_t accz ;

//0=N,R=1,L=2,U=3,D=4
uint8_t IMUDirection = 0;

void mazeGameLoop(){

	//performs on startup of the loop
	if (initilizeMazeGameFlag){
		clearScreen();
		initialisePosition();
		initilizeMazeGameFlag=0;
	}

	//send UART



	if (flag_100ms==1){
		if (endGoal.state ==0){
			endGoal.state =1;
		}else if (endGoal.state==1){
			endGoal.state =0;
		}

		flag_100ms 	= 0;

	}

	if(flag_300ms ==1){
		MovePlayerIMU();

		updatePlayerPos();
		sendMazeUART();
		if( (ball.xpos == endGoal.xpos) && (ball.ypos == endGoal.ypos) ){
			enterMazeFlag=0;
			mazeNumber=1;
			inMazeGameFlag=0;
			//lightUpCorners();
		}

		if(ball.state == 0){//toggle ball state
			ball.state = 1;
		}else if(ball.state == 1){
			ball.state = 0;
		}


		flag_300ms = 0;

	}

	//loop every 1 ms


	updatePlayerPos();//only for x at the moment
	//updatePosition(); //writes to matrix

	switch (mazeNumber){
	case 1:
		colomnScanMaze1();
		break;
	case 2:
		colomnScanMaze2();
		break;
	case 3:
		colomnScanMaze3();
		break;
	case 4:
		colomnScanMaze4();
		break;
	}



	//check the effect of this == no effect
	clearScreen();



	writeMazeToArray();
	updateScreen();	  //displays position


}
void MovePlayerIMU(){
	i2c_transferData[0] = outxl_reg;
	accRes = HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, i2c_transferData,1,1);
	accRes2 = HAL_I2C_Master_Receive(&hi2c1,i2cAddress, i2c_accData, 6, 1);

	accz = (int16_t)((int8_t)i2c_accData[5]);
	accy = (int16_t)((int8_t)i2c_accData[3]);
	accx = (int16_t)((int8_t)i2c_accData[1]);


	if (accx>31){
		IMUDirection = 2; //left
	}else
	if (accx<-31){
		IMUDirection =1 ;//right
	}else
	if (accy<-31){
		IMUDirection=3; //up
	}else
	if (accy>31){
		IMUDirection = 4; //down
	}else
	{
		IMUDirection=0; //no dirction
	}
}
void initialisePosition(){
	ball.xpos = 0;
	ball.ypos = 0;

	if (mazeNumber==1) {
	endGoal.xpos = 7;
	endGoal.ypos = 7;
	}
	if (mazeNumber==2) {
	endGoal.xpos = 3;
	endGoal.ypos = 3;
	}
	if (mazeNumber==3) {
	endGoal.xpos = 7;
	endGoal.ypos = 7;
	}
	if (mazeNumber==4) {
	endGoal.xpos = 2;
	endGoal.ypos = 0;
	}


	writeMazeToArray();
	updateScreen();

}
void sendMazeUART(){

	uint8_t positionMsg[] = "$3xxxxx__\n";

		positionMsg[2] = ball.xpos 		+ 48;
		positionMsg[3] = ball.ypos 		+ 48;

		positionMsg[4] = ball.state 	+ 48;
		positionMsg[5] = endGoal.state	+ 48;

		if (IMUDirection==0){
			positionMsg[6] = 'N';
		}else
		if (IMUDirection==1){
			positionMsg[6] = 'R';
		}else
		if (IMUDirection==2){
			positionMsg[6] = 'L';
		}else
		if (IMUDirection==3){
			positionMsg[6] = 'U';
		}else
		if (IMUDirection==4){
			positionMsg[6] = 'D';
		}

		HAL_UART_Transmit(&huart2,positionMsg,10,100);
}
void resetElements(){//resets the elements of the row and col for the maze1 variable

	for (int i=0; i <8; i ++){
		arrRow[i]= 0;
		Maze1.row[i]=0;
	}
	for (int j=0; j <8; j ++){
			arrCol[j]= 0;
			Maze1.col[j]=0;
		}
}
void writeMazeToArray(){

	for (int i=0; i <8; i ++){
		arrRow[i]= Maze1.row[i];
	}
	for (int j=0; j <8; j ++){
			arrCol[j]= Maze1.col[j];
		}
}
void updatePosition(){ //writes to matrix

	for (int i =0; i<8;i ++){
		if (i==ball.xpos){
			arrCol[i]=1;
		}else
			arrCol[i]=0;

	}
	for (int j =0; j<8;j ++){
			if (j==ball.ypos){
				arrRow[j]=1;
			}else
				arrRow[j]=0;
	}
}

void clearScreen(){

	for (int i =0; i<8;i ++){
			arrCol[i]=0;
			arrRow[i]=0;
	}
			updateScreen();

}

void updatePlayerPos(){//scans for button presses

	switch (mazeNumber){
	case 1 :
		if ((ButtonL.needsAction) || (accx > 31))
	{
		//moveLeft
		if (ball.xpos>0){
			if (layout_M1[ball.ypos][ball.xpos-1] == 0 ){
				ball.xpos --;
			}
		}
		ButtonL.needsAction=0;
		accx = 0;
		accy = 0;
	}else
	if ((ButtonR.needsAction)  || (accx < -31))
		{
		//moveRight
			if (ball.xpos <7){
				if (layout_M1[ball.ypos][ball.xpos+1] ==0){
					ball.xpos ++;
				}
			}
			ButtonR.needsAction=0;
			accx = 0;
			accy = 0;
		}else
	if (ButtonU.needsAction || (accy < -31)){
		//move up
		if (ball.ypos>0){
			if (layout_M1[ball.ypos-1][ball.xpos] == 0){
				ball.ypos --;
			}
		}
		ButtonU.needsAction=0;
		accx = 0;
		accy = 0;
	}else
	if ((ButtonD.needsAction) || (accy > 31)){
		if (ball.ypos<7){
			if (layout_M1[ball.ypos+1][ball.xpos]==0){
				ball.ypos ++;
			}
		}
		ButtonD.needsAction=0;

		accx = 0;
		accy = 0;
	}
		break;
	case 2:
		if ((ButtonL.needsAction) || (accx >31))
			{
				//moveLeft
				if (ball.xpos>0){
					if (layout_M2[ball.ypos][ball.xpos-1] == 0 ){
						ball.xpos --;
					}
				}
				ButtonL.needsAction=0;
				accx = 0;
				accy = 0;
			}else
			if ((ButtonR.needsAction) || (accx < -31))
				{
				//moveRight
					if (ball.xpos <7){
						if (layout_M2[ball.ypos][ball.xpos+1] ==0){
							ball.xpos ++;
						}
					}
					ButtonR.needsAction=0;
					accx = 0;
					accy = 0;
				}else
			if ((ButtonU.needsAction) || (accy < -31)){
				//move up
				if (ball.ypos>0){
					if (layout_M2[ball.ypos-1][ball.xpos] == 0){
						ball.ypos --;
					}
				}
				ButtonU.needsAction=0;
				accx = 0;
				accy = 0;
			}else
			if ((ButtonD.needsAction) || (accy > 31)){
				if (ball.ypos<7){
					if (layout_M2[ball.ypos+1][ball.xpos]==0){
						ball.ypos ++;
					}
				}
				ButtonD.needsAction=0;

				accx = 0;
				accy = 0;
			}
		break;
	case 3:
		if ((ButtonL.needsAction)  || (accx >31))
					{
						//moveLeft
						if (ball.xpos>0){
							if (layout_M3[ball.ypos][ball.xpos-1] == 0 ){
								ball.xpos --;
							}
						}
						ButtonL.needsAction=0;
						accx = 0;
						accy = 0;
					}else
					if ((ButtonR.needsAction) || (accx < -31))
						{
						//moveRight
							if (ball.xpos <7){
								if (layout_M3[ball.ypos][ball.xpos+1] ==0){
									ball.xpos ++;
								}
							}
							ButtonR.needsAction=0;
							accx = 0;
							accy = 0;
						}else
					if ((ButtonU.needsAction) || (accy < -31)){
						//move up
						if (ball.ypos>0){
							if (layout_M3[ball.ypos-1][ball.xpos] == 0){
								ball.ypos --;
							}
						}
						ButtonU.needsAction=0;
						accx = 0;
						accy = 0;
					}else
					if ((ButtonD.needsAction) || (accy >31)){
						if (ball.ypos<7){
							if (layout_M3[ball.ypos+1][ball.xpos]==0){
								ball.ypos ++;
							}
						}
						ButtonD.needsAction=0;
						accx = 0;
						accy = 0;
					}
		break;

	case 4:
		if ((ButtonL.needsAction) || (accx > 31))
					{
						//moveLeft
						if (ball.xpos>0){
							if (layout_M4[ball.ypos][ball.xpos-1] == 0 ){
								ball.xpos --;
							}
						}
						ButtonL.needsAction=0;
						accx = 0;
						accy = 0;
					}else
					if ((ButtonR.needsAction) || (accx < -31))
						{
						//moveRight
							if (ball.xpos <7){
								if (layout_M4[ball.ypos][ball.xpos+1] ==0){
									ball.xpos ++;
								}
							}
							ButtonR.needsAction=0;
							accx = 0;
							accy = 0;
						}else
					if ((ButtonU.needsAction) || (accy < -31)){
						//move up
						if (ball.ypos>0){
							if (layout_M4[ball.ypos-1][ball.xpos] == 0){
								ball.ypos --;
							}
						}
						ButtonU.needsAction=0;
						accx = 0;
						accy = 0;
					}else
					if ((ButtonD.needsAction) || (accy > 31)){
						if (ball.ypos<7){
							if (layout_M4[ball.ypos+1][ball.xpos]==0){
								ball.ypos ++;
							}
						}
						ButtonD.needsAction=0;
						accx = 0;
						accy = 0;
					}
			break;
	}//switch

	if (ButtonM.needsAction)
		{
			resetElements();
			clearScreen();

			enterMazeFlag=0;
			inMazeGameFlag =0;
			mazeNumber =1;
		 	ButtonM.needsAction=0;

		}
}




void colomnScanMaze1(){

	switch (flag_8ms)
	{
	case 0 :resetElements();
		Maze1.col[0] = 1;
		Maze1.row[1] = 1;
		Maze1.row[5] = 1;

		if(ball.xpos == flag_8ms){
			setBall();

		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}
		break;


	case 1 :resetElements();
		Maze1.col[1] = 1;
		Maze1.row[1] = 1;
		Maze1.row[3] = 1;
		Maze1.row[5] = 1;
		Maze1.row[7] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break;

	case 2 :resetElements();
		Maze1.col[2] = 1;
		Maze1.row[3] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break;


	case 3 :resetElements();
		Maze1.col[3] = 1;
		Maze1.row[1] = 1;
		Maze1.row[3] = 1;
		Maze1.row[5] = 1;
		Maze1.row[6] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;

	case 4 :resetElements();
		Maze1.col[4] = 1;
		Maze1.row[1] = 1;
		Maze1.row[3] = 1;
		Maze1.row[4] = 1;
		Maze1.row[5] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;


	case 5 :resetElements();
		Maze1.col[5] = 1;
		Maze1.row[3] =1 ;
		Maze1.row[7] =1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;



	case 6 :resetElements();
		Maze1.col[6] = 1;
		Maze1.row[0] = 1;
		Maze1.row[2] = 1;
		Maze1.row[5] = 1;
		Maze1.row[6] = 1;
		Maze1.row[7] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;


	case 7 :resetElements();
		Maze1.col[7] = 1;
		Maze1.row[3] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;
	}
}

void colomnScanMaze2(){

	switch (flag_8ms)
	{
	case 0 :resetElements();
		Maze1.col[0] = 1;
		Maze1.row[5] = 1;

		if(ball.xpos == flag_8ms){
			setBall();

		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}
		break;


	case 1 :resetElements();
		Maze1.col[1] = 1;
		Maze1.row[1] = 1;
		Maze1.row[3] = 1;


		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break;

	case 2 :resetElements();
		Maze1.col[2] = 1;
		Maze1.row[2] = 1;
		Maze1.row[4] = 1;
		Maze1.row[5] = 1;
		Maze1.row[6] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break;


	case 3 :resetElements();
		Maze1.col[3] = 1;
		Maze1.row[1] = 1;
		Maze1.row[4] = 1;


		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;

	case 4 :resetElements();
		Maze1.col[4] = 1;
		Maze1.row[2] = 1;
		Maze1.row[4] = 1;
		Maze1.row[7] = 1;


		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;


	case 5 :resetElements();
		Maze1.col[5] = 1;
		Maze1.row[1] =1;
		Maze1.row[4] =1;
		Maze1.row[5] =1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;



	case 6 :resetElements();
		Maze1.col[6] = 1;
		Maze1.row[1] = 1;
		Maze1.row[3] = 1;
		Maze1.row[5] = 1;
		Maze1.row[6] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;


	case 7 :resetElements();
		Maze1.col[7] = 1;
		Maze1.row[3] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;
	}
}
void colomnScanMaze3(){

	switch (flag_8ms)
	{
	case 0 :resetElements();
		Maze1.col[0] = 1;
		Maze1.row[3] = 1;

		if(ball.xpos == flag_8ms){
			setBall();

		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}
		break;


	case 1 :resetElements();
		Maze1.col[1] = 1;
		Maze1.row[1] = 1;
		Maze1.row[5] = 1;
		Maze1.row[6] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break;

	case 2 :resetElements();
		Maze1.col[2] = 1;
		Maze1.row[2] = 1;
		Maze1.row[3] = 1;
		Maze1.row[4] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break;


	case 3 :resetElements();
		Maze1.col[3] = 1;
		Maze1.row[0] = 1;
		Maze1.row[6] = 1;


		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;

	case 4 :resetElements();
		Maze1.col[4] = 1;
		Maze1.row[0] = 1;
		Maze1.row[1] = 1;
		Maze1.row[2] = 1;
		Maze1.row[4] = 1;
		Maze1.row[5] = 1;
		Maze1.row[6] = 1;
		Maze1.row[7] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;


	case 5 :resetElements();
		Maze1.col[5] = 1;
		Maze1.row[4] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;



	case 6 :resetElements();
		Maze1.col[6] = 1;
		Maze1.row[1] = 1;
		Maze1.row[2] = 1;
		Maze1.row[3] = 1;
		Maze1.row[4] = 1;
		Maze1.row[6] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;


	case 7 :resetElements();
		Maze1.col[7] = 1;
		Maze1.row[6] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;
	}
}
void colomnScanMaze4(){

	switch (flag_8ms)
	{
	case 0 :resetElements();
		Maze1.col[0] = 1;
		Maze1.row[6] = 1;

		if(ball.xpos == flag_8ms){
			setBall();

		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}
		break;


	case 1 :resetElements();
		Maze1.col[1] = 1;
		Maze1.row[0] = 1;
		Maze1.row[1] = 1;
		Maze1.row[2] = 1;
		Maze1.row[3] = 1;
		Maze1.row[4] = 1;
		Maze1.row[6] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break;

	case 2 :resetElements();
		Maze1.col[2] = 1;
		Maze1.row[2] = 1;
		Maze1.row[6] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break;


	case 3 :resetElements();
		Maze1.col[3] = 1;
		Maze1.row[0] = 1;
		Maze1.row[2] = 1;
		Maze1.row[4] = 1;
		Maze1.row[6] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;

	case 4 :resetElements();
		Maze1.col[4] = 1;
		Maze1.row[2] = 1;
		Maze1.row[3] = 1;
		Maze1.row[6] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;


	case 5 :resetElements();
		Maze1.col[5] = 1;
		Maze1.row[1] =1;
		Maze1.row[5] =1;
		Maze1.row[6] =1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;

	case 6 :resetElements();
		Maze1.col[6] = 1;
		Maze1.row[1] = 1;
		Maze1.row[3] = 1;
		Maze1.row[4] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}

		break ;


	case 7 :resetElements();
		Maze1.col[7] = 1;
		Maze1.row[6] = 1;

		if(ball.xpos == flag_8ms){
			setBall();
		}
		if(endGoal.xpos == flag_8ms){
			setEndGoal();
		}
		break ;
	}
}
void setBall(){
	if(ball.state == 1){
		Maze1.row[ball.ypos] = 1;
	}else if(ball.state == 0){
		Maze1.row[ball.ypos] = 0;
	}
}
void setEndGoal(){
	if(endGoal.state == 1){
			Maze1.row[endGoal.ypos] = 1;
		}else if(endGoal.state == 0){
			Maze1.row[endGoal.ypos] = 0;
		}
}


