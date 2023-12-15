// CubeMars.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _USE_MATH_DEFINES
#define WIN32_LEAN_AND_MEAN 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include "stdStechoqMars.h"
#include <cmath>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include<stdio.h>
#include<winsock2.h>
#include <conio.h>
#include <ctime>
using namespace std;


#define OffDepan (38.245)
#define OffTengah (275.15)
#define OffBelakang (0)

Mars Motor[10];
int baud = 921600;

int MainState = 0;
int LastMainState = 1;
bool run = FALSE;
bool hbrake = FALSE;


clock_t lastreply = clock();
clock_t lastpos = clock();
clock_t looptime = clock();

double kp = 0;
double kd = 0;
double ki = 0;

double InvBackPos;
double InvMidPos;
double InvFrontPos;

const double MinMaxPos[3][2] = { // sudut range tiap motor untuk failsafe
	//min , max
	{-20, 20}, //motor Belakang
	{-10, 5}, // motor Tengah
	{-175, -24} // motor depan 
};

const double HomePos[3] =  {0,31, -69}; // homming 

double pos[3] = { 0,31, -69 }; //set awal harus homming


BOOL WINAPI ConsoleHandler(
	DWORD dwCtrlType   //  control signal type
);
BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	char mesg[128];

	switch (CEvent)
	{
	case CTRL_C_EVENT:
		for (int iter = 1; iter < 3; iter++) {
			cout << "CTRL+C received!";
			Motor[iter].xCurr = 0;// 21000;
			Motor[iter].setCurr();
			Sleep(100);
			Motor[iter].xCurr = 0;// 21000;
			Motor[iter].setCurr();
			Sleep(100);
		}
		
		exit(1);
		break;
	case CTRL_BREAK_EVENT:
		
			cout << "CTRL+BREAK received!";
			exit(1);
		break;
	case CTRL_CLOSE_EVENT:
		cout << "CTRL+BREAK received!";
		exit(1);
		break;
	case CTRL_LOGOFF_EVENT:
		cout << "CTRL+BREAK received!";
		exit(1);
		break;
	case CTRL_SHUTDOWN_EVENT:
		cout << "CTRL+BREAK received!";
		exit(1);
		break;

	}
	return TRUE;

}


void updateState (){
	Motor[0].getStream();
	Motor[1].getStream();
	Motor[2].getStream();

	Motor[0].Update();
	Motor[1].Update();
	Motor[2].Update();
}

void PIDSpeed (double Target[3], double kpIn, double kdIn, double kiIn) {

	double err[3];
	double Ierr[3];
	double out[3];

	updateState();
	for (int i = 0;i < 3;i++) {
		err[i] = Target[i] - Motor[i].yPos;
	}
	for (int i = 0;i < 3;i++) {
		out[i] = err[i] * kpIn + Ierr[i] * kiIn + Motor[i].ERPM * kdIn;
	}
	for (int i = 0;i < 3;i++) {
		Motor[i].xSpd = out[i];
	}
	for (int i = 0;i < 3;i++) {
		Motor[i].setSpd();
	}
	for (int i = 0;i < 3;i++) {
		Ierr[i] += err[i];
	}
}

void PIDCurrent(double Target[3], double kpIn, double kdIn, double kiIn) {

	double err[3];
	double Ierr[3];
	double out[3];

	updateState();
	for (int i = 0;i < 3;i++) {
		err[i] = Target[i] - Motor[i].yPos;
	}
	for (int i = 0;i < 3;i++) {
		out[i] = err[i] * kpIn / 1000 + Ierr[i] * kiIn / 1000 + Motor[i].ERPM * kdIn / 1000;
	}
	for (int i = 0;i < 3;i++) {
		Motor[i].xSpd = out[i];
	}
	for (int i = 0;i < 3;i++) {
		Motor[i].setCurr();
	}
	for (int i = 0;i < 3;i++) {
		Ierr[i] += err[i];
	}
}

void PIDDuty(double Target[3], double kpIn, double kdIn, double kiIn) {

	double err[3];
	double Ierr[3];
	double out[3];

	updateState();
	for (int i = 0;i < 3;i++) {
		err[i] = Target[i] - Motor[i].yPos;
	}
	for (int i = 0;i < 3;i++) {
		out[i] = err[i] * kpIn / 50000 + Ierr[i] * kiIn / 50000 + Motor[i].ERPM * kdIn / 50000;
	}
	for (int i = 0;i < 3;i++) {
		Motor[i].xSpd = out[i];
	}
	for (int i = 0;i < 3;i++) {
		Motor[i].setDuty();
	}
	for (int i = 0;i < 3;i++) {
		Ierr[i] += err[i];
	}
}

void setHBrake (){
	Motor[0].HandBrakeCurr = 15;
	Motor[1].HandBrakeCurr = 15;
	Motor[2].HandBrakeCurr = 15;
	Motor[0].setHandBrake();
	Motor[1].setHandBrake();
	Motor[2].setHandBrake();
}

bool InRange(double value, double lowerBound, double upperBound) {
	
	return (value >= lowerBound && value <= upperBound);
}

bool safePos (){
	bool m[3];
	m[0] = InRange (Motor[0].yPos, MinMaxPos[0][0], MinMaxPos[0][1]);
	if (!m[0]) {
		cout << "Back Motor out of range" << endl;
	}
	m[1] = InRange (Motor[1].yPos, MinMaxPos[1][0], MinMaxPos[1][1]);
	if (!m[1]) {
		cout << "Middle Motor out of range"<<endl;
	}
	m[2] = InRange (Motor[2].yPos, MinMaxPos[2][0], MinMaxPos[2][1]);
	if (!m[2]) {
		cout << "Front Motor out of range" << endl;
	}
	return m[0] && m[1] && m[2];
}

void Inverse2Doff (float x, float y){
	float L1 = 320;
	float L2 = 285;
	float L3 = 70;
	float L4 = 315;
	float L5 = 45;

	double A = sqrt(pow(x, 2) + pow(y, 2));
	double alpha3 = acos((pow(L1, 2) + pow(A, 2) - pow(L2, 2)) / (2 * L1 * A));
	double alpha4 = atan (x/y);

	InvMidPos = (M_PI - alpha3 - alpha4) * 180.0 / M_PI;;

	double a = acos((pow(L1, 2) + pow(L2, 2) - pow(A, 2)) / (2 * L1 * L2));
	double B = sqrt(pow(L1, 2) + pow(L3, 2) - 2*L1*L3*cos (M_PI - a));
	double alpha1 = acos((pow(L5, 2) + pow(B, 2) - pow(L4, 2)) / (2 * L5 * B));
	double alpha2 = acos((pow(L1, 2) + pow(B, 2) - pow(L3, 2)) / (2 * L1 * B));

	InvFrontPos = (alpha1 + alpha2) * 180.0 / M_PI;;	

}

 bool safe;
 void SafetyStop() {
	 //Failsafe 
	 safe = safePos();
	 if (!safe && MainState > 1) {
		 run = false;
		 MainState = 0;
		 cout << "Failsafe!!!! " << endl;
	 }

	 // Safety exit (Esc Press)

	 if (_kbhit()) {
		 if (_getch() == 27) {
			 run = false;
			 MainState = 0;
			 cout << "Safety Stop Pressed!!!! " << endl;
		 }
	 }
}

 int ControlMode = 0;
 /*
 ControlMode : Mode
 0 : PID Position Control Using Speed Setting
 1 : PID Position Control Using Current Setting
 2 : PID Position Control Using Duty Cycle Setting
 */
 void ControlSwitch() {
	 switch (ControlMode)
	 {
	 case 0: {
		 PIDSpeed(pos, kp, kd, ki);
	 }
	 case 1: {
		 PIDCurrent(pos, kp, kd, ki);
	 }
	 case 2: {
		 PIDDuty(pos, kp, kd, ki);
	 }
	 default:
		 break;
	 }
 }

 void Routine() {
	 if (clock() - looptime > 5) {
		 if (!run) {
			 updateState();
		 }
		 if (MainState > 1) {
			 if (hbrake) {
				 setHBrake();
			 }
			 else {
				 if (run) {
					 ControlSwitch();
				 }
			 }
		 }
		 looptime = clock();
	 }
 }

 // Movement Function
 int stateMovement = 0;
 const double setJump[3][3] = {//perlu update
	{0,135, -18},
	{0,116, -117},
	{0,135, -18},

 };
 void Jump() {
	 if (stateMovement < 3) {
		 pos[0] = setJump[stateMovement][0];
		 pos[1] = setJump[stateMovement][1];
		 pos[2] = setJump[stateMovement][2];

		 if (abs(setJump[stateMovement][0] - Motor[0].yPos) < 1.0
			 && abs(setJump[stateMovement][1] - Motor[1].yPos) < 1.0
			 && abs(setJump[stateMovement][2] - Motor[2].yPos) < 1.0)
		 {
			 stateMovement++;
		 }
	 }
	 else {
		 MainState = 3;
		 stateMovement = 0;
	 }
 }

 int MovementType = 0;
 /*
 Movement Type : 
 0: Jump
 */
 void MovementSwitch() {
	 switch (MovementType)
	 {
	 case 0: {
		 Jump();
	 }
	 default:
		 break;
	 }
 }
 /*
 Main State Description
 0: Initial State Motor (Motor is inactive)
 1: Logging State (To show encoder read from motor) exit by press 1 and enter
 2: Homming (turn the leg into initial position)
 3: Standby (wait for user input to perform a movement)
 4: Movement
 */
 void MainStateMachine() {
	 switch (MainState) {
	 case 0: {				//inisialisasi 
		 run = false;
		 cout << "Intial state! press enter to next " << endl;
		 if (_getch() == '\r') {
			 MainState = 1;
		 }
		 break;
	 }

	 case 1: {   //check sudut manual & konfirmasi
		 if (clock() - lastreply > 50) {
			 Sleep(500);
			 run = false;

			 cout << "CONNECTED,   POS 0 : " << Motor[0].yPos << "   POS 1 :" << Motor[1].yPos << "   POS 2 :" << Motor[2].yPos << endl;

			 lastreply = clock();

		 }
		 if (_kbhit()) {
			 cout << "PRESS 1 TO APPLY TORQUE";
			 char inp;
			 cin >> inp;
			 if (inp == '1') {
				 MainState = 2;
			 }
		 }
		 break;
	 }

	 case 2: {		//HOMING program
		 if (clock() - lastpos > 5) {
			 memcpy(pos, HomePos, sizeof(HomePos));
			 kp = 20;
			 run = true;

			 if (
				 abs(HomePos[0] - Motor[0].yPos) < 1.0 
				 && abs(HomePos[1] - Motor[1].yPos) < 1.0
				 && abs(HomePos[2] - Motor[2].yPos) < 1.0
				) 
			 {
				 cout << "Initial Position Has Been Reached" << endl;
				 MainState = 3;
			 }
			 else {
				 if (LastMainState!= MainState) {
					 cout << "Go to Initial Position" << endl;

				 }
			 }
			 LastMainState = MainState;

			 lastpos = clock();
		 }
		 break;
	 }
	 case 3: {		//Standby
		 if (LastMainState != MainState) {
			 cout << "Select Movement" << endl;
			 cout << "Up Arrow for Jump" << endl;
			 cout << "Down Arrow for Jump" << endl;
			 cout << "Right Arrow for Jump" << endl;
			 cout << "Left Arrow for Jump" << endl;

		 }
		 if (_kbhit()) {
			 if (_getch() == 72) {// jika panah atas ditekan
				 MainState = 4;
				 MovementType = 0;
			 }
			 if (_getch() == 80) {// jika panah bawah ditekan
			 	MainState = 4;
				MovementType = 0;
			 }
			 if (_getch() == 75) {// jika panah kiri ditekan
			 	MainState = 4;
				MovementType = 0;
			 }
			 if (_getch() == 77) {// jika panah atas ditekan
			 	MainState = 4;
				MovementType = 0;
			 }
		 }
		 else {
			 MainState = 3;
		 }
		 LastMainState = MainState;
		 break;
	 }
	 case 4: {
		 MovementSwitch();
	 }
	 default: {
		 break;
	 }
	 }
 }

void setup()
{
	Motor[0].begin(baud, 19); //Back
	Motor[1].begin(baud,18); //Mid
	Motor[2].begin(baud, 20); //Front
	ControlMode = 0;
	Sleep(1000);
}

void loop()
{
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE)
	{
		cout << "Unable to install handler!";

	}

	SafetyStop(); //Safety Function

	Routine(); // Update and run the motor

	MainStateMachine();	//Main Program
}

int main()
{
	setup();	
	while (true)
		loop();
	return 0;
}

