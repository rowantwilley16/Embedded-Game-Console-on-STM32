/*
 * tennisGame.c
 *
 *  Created on: 23 May 2021
 *      Author: Rowan Twilley
 */
#include "main.h"
#include "UserGeneral.h"
#include "tennisGame.h"


extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;

extern uint8_t arrRow[8];
extern uint8_t arrCol[8];

extern uint8_t flag_8ms;
extern uint16_t counterTennisUART;

extern uint16_t counter_BallUpdate;
extern uint8_t flag_BallUpdate;

typedef struct button
{
	uint8_t needsAction;
	uint16_t debounce;
	GPIO_TypeDef *port;
	uint16_t pin;
}Button;

typedef struct bat
{
	uint8_t ypos; //top LED of the bat
	uint8_t xpos;
	uint8_t upperBatPos;
	uint8_t lowerBatPos;
}Bat;

Bat bat = {3,0,3,4};
typedef struct Court
{
	uint8_t row[8];
	uint8_t col[8];

}court;

court court1;

typedef struct TennisBall
{
	uint8_t xpos;
	uint8_t ypos;
	uint8_t direction; //0=L, 1=R, 2 =RD, 3 =LU, 4 =RU, 5=LD
	uint8_t hitBatCounter;
	uint8_t velocity;
}tennisBall;

tennisBall tBall;

uint8_t IMUDirectionT = 0;

extern Button ButtonM ;
extern Button ButtonL ;
extern Button ButtonR ;
extern Button ButtonD ;
extern Button ButtonU ;

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
extern uint8_t inTennisGameFlag;
extern uint8_t initilizeTennisGameFlag;


//ball movement
extern uint16_t counter_BallUpdate;
extern uint8_t flag_BallUpdate;
extern uint16_t ballUpdateTime;


uint32_t sliderInput;


const uint8_t courtLayout[8][8] = {
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0}
	};

void tennisGameLoop(){



	//clearScreen();

	if (initilizeTennisGameFlag){
		//once off
		initilize();
		initilizeTennisGameFlag=0;

	}




	sendTennisUART();

	//sampleSlider(); //gets bat position not needed for demo 4

	//looping every 1 ms

	//updateCourt();
	colomnScanCourt();

	writeCourtToArray();
	updateScreen();
	checkBounds();

	updateBallSpeed();
	updateBallPosition();


	//exit the tennis game
	if (ButtonM.needsAction){
		inTennisGameFlag =0;
		clearScreen();
		ButtonM.needsAction=0;
	}
}
void MoveBatIMU(){
	i2c_transferData[0] = outxl_reg;
	accRes = HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, i2c_transferData,1,1);
	accRes2 = HAL_I2C_Master_Receive(&hi2c1,i2cAddress, i2c_accData, 6, 1);

	accz = (int16_t)((int8_t)i2c_accData[5]);
	accy = (int16_t)((int8_t)i2c_accData[3]);
	accx = (int16_t)((int8_t)i2c_accData[1]);

	if (accx>31){
		IMUDirectionT = 2; //left
	}else
	if (accx<-31){
		IMUDirectionT =1 ;//right
	}else
	if (accy<-31){
		IMUDirectionT=3; //up
	}else
	if (accy>31){
		IMUDirectionT = 4; //down
	}else
	{
		IMUDirectionT=0; //no dirction
	}

}

void moveBat(){
	if ( ((ButtonU.needsAction) ||(accy<-31)) && (bat.upperBatPos>=1) ){
		bat.upperBatPos --;
		bat.lowerBatPos = bat.upperBatPos +1;
		ButtonU.needsAction=0;
	}
	if (((ButtonD.needsAction) ||(accy>31)) && (bat.upperBatPos<=5)){
			bat.upperBatPos ++;
			bat.lowerBatPos = bat.upperBatPos +1;
			ButtonD.needsAction=0;
		}

	if (((ButtonL.needsAction) ||(accx >31)) && (bat.xpos >=1)){
			bat.xpos -- ;
			bat.lowerBatPos = bat.upperBatPos +1;
			ButtonL.needsAction=0;
		}
	if (((ButtonR.needsAction) || (accx <-31)) && (bat.xpos<=6)){
			bat.xpos ++;
			bat.lowerBatPos = bat.upperBatPos +1;
			ButtonR.needsAction=0;
		}
}
void updateBallSpeed(){

	if (tBall.hitBatCounter <3){
		ballUpdateTime = 700;
		tBall.velocity =1;
	}else
	if ((tBall.hitBatCounter>=3) && (tBall.hitBatCounter<6)){
		ballUpdateTime = 650;
		tBall.velocity =2;
	}else
	if ((tBall.hitBatCounter>=6) && (tBall.hitBatCounter<9)){
		ballUpdateTime = 600;
		tBall.velocity =3;
	}else
	if ((tBall.hitBatCounter>=9) && (tBall.hitBatCounter<12)){
		ballUpdateTime = 550;
		tBall.velocity =4;
	}else
	if ((tBall.hitBatCounter>=12) && (tBall.hitBatCounter<15)){
		ballUpdateTime = 500;
		tBall.velocity =5;
	}else
	if ((tBall.hitBatCounter>=15) && (tBall.hitBatCounter<18)){
		ballUpdateTime = 450;
		tBall.velocity =6;
	}else
	if ((tBall.hitBatCounter>=18) && (tBall.hitBatCounter<21)){
		ballUpdateTime = 400;
		tBall.velocity =7;
	}else
	if ((tBall.hitBatCounter>=21) && (tBall.hitBatCounter<24)){
		ballUpdateTime = 350;
		tBall.velocity =8;
	}else
	if (tBall.hitBatCounter > 24){
		ballUpdateTime = 300;
		tBall.velocity =9;
	}


}
void sendTennisUART(){

	if (counterTennisUART >= 100){
		MoveBatIMU();
		moveBat();
		uint8_t positionMsg[] = "$2xxxxxxx\n";

			positionMsg[2] = tBall.xpos 		+ 48;
			positionMsg[3] = tBall.ypos 		+ 48;
			positionMsg[4] = tBall.velocity 	+ 48;
			positionMsg[5] = tBall.direction	+ 48;
			positionMsg[6] = bat.xpos			+ 48;
			positionMsg[7] = bat.upperBatPos 	+ 48;

			if (IMUDirectionT==0){
				positionMsg[8] = 'N';
			}else
			if (IMUDirectionT==1){
				positionMsg[8] = 'R';
			}else
			if (IMUDirectionT==2){
				positionMsg[8] = 'L';
			}else
			if (IMUDirectionT==3){
				positionMsg[8] = 'U';
			}else
			if (IMUDirectionT==4){
				positionMsg[8] = 'D';
			}
			HAL_UART_Transmit(&huart2,positionMsg,10,100);

			//reset counterTennisUART
			counterTennisUART = 0;

			if (tBall.xpos==0){
				//exit the game
					inTennisGameFlag=0;
			}
	}

}
void checkBounds(){


	//hit bat checking
	if (((tBall.xpos -1) == bat.xpos) && ((tBall.ypos ==bat.lowerBatPos) || (tBall.ypos==bat.upperBatPos))){

		//update counter for number of times ball has hit the bat


		switch(tBall.direction){
		case 0 :
			tBall.direction = 4;
			tBall.hitBatCounter +=1;
			break ;
		case 3 :
			tBall.direction = 4;
			tBall.hitBatCounter +=1;
			break ;
		case 5 :
			tBall.direction = 2;
			tBall.hitBatCounter +=1;
			break;
		}//end of bat checking

	}

	//2 special cases of when the ball is coming at the bat at the angle and hitting the edge of the bat

		// tBall coming in from top right to hit top bat edge
		if ((tBall.xpos-1 ==bat.xpos) && (tBall.ypos+1 ==bat.upperBatPos) && (tBall.direction==5)){

			tBall.hitBatCounter+=1;
			tBall.direction =4;
		}

		//tBall coming in from bottom right to hit bottom bat edge
		if ((tBall.xpos-1 == bat.xpos) && (tBall.ypos-1 == bat.lowerBatPos) && (tBall.direction ==3)){

			tBall.hitBatCounter+=1;
			tBall.direction =2;
		}

		//check edges of the screen

		//Right side (back wall)
		if (tBall.xpos == 7){

			switch (tBall.direction){
			case 2:
				tBall.direction =5;
				break;

			case 4 :
				tBall.direction = 3;
				break;
			}
		}//end (back wall)

		//Top side (top wall)
		if (tBall.ypos == 0){

			switch (tBall.direction){
				case 3:
					tBall.direction = 5;
					break;

				case 4 :
					tBall.direction = 2;
					break;
				}

		}//end of top side checking

		if (tBall.ypos==7){

			switch (tBall.direction){
				case 2:
					tBall.direction = 4;
					break;

				case 5:
					tBall.direction = 3;
					break;
				}
		}//end of bottom checking

}
void resetElementsCourt(){
	for (int i=0; i <8; i ++){
		//arrRow[i]= 0;
		court1.row[i]=0;
	}
	for (int j=0; j <8; j ++){
			//arrCol[j]= 0;
			court1.col[j]=0;
		}
}


void updateCourt(){

	resetElementsCourt();					//clears the court

}
void initilize(){

	//initilize bat
	bat.xpos =0;
	bat.upperBatPos = 3;
	bat.lowerBatPos = bat.upperBatPos + 1 ;

	//initilize ball start position
	tBall.xpos = 7;
	tBall.ypos = 4;
	tBall.direction = 0;					 //0 = left
	tBall.hitBatCounter=0;
	tBall.velocity =1; 						//starting velocity

	counter_BallUpdate = 0;
	flag_BallUpdate =0;

	counterTennisUART=0;
}
void updateBallPosition(){

	if (flag_BallUpdate){

		switch (tBall.direction){
		case 0: //L
			if (tBall.xpos>0){				//not equal to (i think)
				tBall.xpos --;
			}

			break ;
		case 1: //R
			if (tBall.xpos<7){
				tBall.xpos ++;
			}

			break ;
		case 2: //RD
			if ((tBall.xpos>0) && (tBall.ypos<8)){
			tBall.xpos ++;
			tBall.ypos ++;
			}

			break ;
		case 3: //LU
			if ((tBall.xpos>0) && (tBall.ypos>=0)){
			tBall.xpos --;
			tBall.ypos --;
			}
			break ;
		case 4: //RU
			if ((tBall.xpos<8) && (tBall.ypos>=0)){
			tBall.xpos ++;
			tBall.ypos --;
			}
			break ;
		case 5: //LD
			if ((tBall.xpos>0) && (tBall.ypos<8)){
				tBall.xpos --;
			tBall.ypos ++;
			}

			break ;
		}

		//reset flag for update
		flag_BallUpdate=0;
		counter_BallUpdate=0;
	}
}


void sampleSlider(){

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	sliderInput = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	if ((sliderInput>=0) && (sliderInput<=512)){
		bat.upperBatPos= 0;
		bat.lowerBatPos= bat.upperBatPos +1;
	}else
	if ((sliderInput>512) && (sliderInput<=1024)){
		bat.upperBatPos= 1;
		bat.lowerBatPos= bat.upperBatPos +1;
	}else
	if ((sliderInput>1024) && (sliderInput<=1536)){
		bat.upperBatPos= 2;
		bat.lowerBatPos= bat.upperBatPos +1;
	}else
	if ((sliderInput>1536) && (sliderInput<=2048)){
		bat.upperBatPos= 3;
		bat.lowerBatPos= bat.upperBatPos +1;
	}else
	if ((sliderInput>2048) && (sliderInput<=2560)){
		bat.upperBatPos= 4;
		bat.lowerBatPos= bat.upperBatPos +1;
	}else
	if ((sliderInput>2560) && (sliderInput<=3072)){
		bat.upperBatPos= 5;
		bat.lowerBatPos= bat.upperBatPos +1;
	}else
	if ((sliderInput>3072) && (sliderInput<=3584)){
		bat.upperBatPos= 6;
		bat.lowerBatPos= bat.upperBatPos +1;
	}else
	if ((sliderInput>3584) && (sliderInput<=4096)){
		bat.upperBatPos= 6; 							//since we dont want it to go to 7 so stays 6
		bat.lowerBatPos= bat.upperBatPos +1;
	}
}
void colomnScanCourt(){

	switch (flag_8ms)
		{
		case 0 :resetElementsCourt();
			court1.col[0]=1;

			if (bat.xpos==0){
				court1.row[bat.upperBatPos]=1;
				court1.row[bat.lowerBatPos]=1;
			}

			if (tBall.xpos == 0){
				court1.col[tBall.ypos] =1;

			}
			break;


		case 1 :resetElementsCourt();
			court1.col[1] = 1;

			if (tBall.xpos == 1){
				court1.row[tBall.ypos] =1;
			}
			if (bat.xpos==1){
				court1.row[bat.upperBatPos]=1;
				court1.row[bat.lowerBatPos]=1;
			}

			break;

		case 2 :resetElementsCourt();
			court1.col[2] = 1;

			if (tBall.xpos == 2){

				court1.row[tBall.ypos] =1;
			}
			if (bat.xpos==2){
				court1.row[bat.upperBatPos]=1;
				court1.row[bat.lowerBatPos]=1;
			}
			break;


		case 3 :resetElementsCourt();
			court1.col[3] = 1;

			if (tBall.xpos == 3){

				court1.row[tBall.ypos] =1;

			}
			if (bat.xpos==3){
				court1.row[bat.upperBatPos]=1;
				court1.row[bat.lowerBatPos]=1;
			}
			break ;

		case 4 :resetElementsCourt();
			court1.col[4] = 1;

			if (tBall.xpos == 4){

				court1.row[tBall.ypos] =1;

			}
			if (bat.xpos==4){
				court1.row[bat.upperBatPos]=1;
				court1.row[bat.lowerBatPos]=1;
			}
			break ;


		case 5 :resetElementsCourt();
			court1.col[5] = 1;

			if (tBall.xpos == 5){

				court1.row[tBall.ypos] =1;

			}
			if (bat.xpos==5){
				court1.row[bat.upperBatPos]=1;
				court1.row[bat.lowerBatPos]=1;
			}
			break ;



		case 6 :resetElementsCourt();
			court1.col[6] = 1;

			if (tBall.xpos == 6){

				court1.row[tBall.ypos] =1;

			}
			if (bat.xpos==6){
				court1.row[bat.upperBatPos]=1;
				court1.row[bat.lowerBatPos]=1;
			}
			break ;


		case 7 :resetElementsCourt();
			court1.col[7] = 1;

			if (tBall.xpos == 7){

				court1.row[tBall.ypos] =1;

			}
			if (bat.xpos==7){
				court1.row[bat.upperBatPos]=1;
				court1.row[bat.lowerBatPos]=1;
			}
			break ;
		}
	}

void writeCourtToArray(){

	for (int i=0; i <8; i ++){
		arrRow[i]= court1.row[i];
	}
	for (int j=0; j <8; j ++){
			arrCol[j]= court1.col[j];
		}
}




