/*
 * tennisGame.h
 *
 *  Created on: 23 May 2021
 *      Author: Rowan
 */

#ifndef INC_TENNISGAME_H_
#define INC_TENNISGAME_H_

void tennisGameLoop();
void sampleSlider();
void resetElementsCourt();

void initilize();
void updateCourt();
void colomnScanCourt();
void writeCourtToArray();
void updateBallPosition();
void checkBounds();
void updateBallSpeed();
void sendTennisUART();
void moveBat();
void MoveBatIMU();

#endif /* INC_TENNISGAME_H_ */
