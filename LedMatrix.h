//make sure the code is executed only once
#pragma once

//include the Arduino library for some useful keywords
#include "Arduino.h"



//constants
//a list which represents the "x to the power of n"-function for (x = 2) and (0 <= n <= 7)
const int TwoToThe[]	= { 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000 };
//a list which represents the binary inverse of the values in the list above
const int NotTwoToThe[]	= { 0b11111110, 0b11111101, 0b11111011, 0b11110111, 0b11101111, 0b11011111, 0b10111111, 0b01111111 };



//the LedMatrix class
class LedMatrix
{
private: //private class members

	//the arrangement of the matrices
	int* m_MatrixConfig;
	bool* m_MatrixDirSwitched;

	//number of matrices overall
	int m_iNumMatrices;

	//number of matrices in a column or in a row
	int m_iColumns;
	int m_iRows;
	
	//the state of each Led in the matrix
	bool* m_LedState;

	//the pin registers
	int* m_pMOSIPinReg;
	int* m_pCLKPinReg;
	int* m_pCSPinReg;
	//the pin numbers in their register
	int m_iMOSIPinNum;
	int m_iCLKPinNum;
	int m_iCSPinNum;


	//functions for sending data to the LED matrix
	//prepare for data transfer by pulling the CS-Pin to the ground
	void PullDownCS();
	//latch the data into the controller by pulling the CS-pin to the current voltage
	void PullUpCS();

	//send data to the matrix controller(s)
	void SendData(int iAddress, int iData);

	//send 8 values from m_LedState to the matrix controller
	void SendLEDStates(int* iMOSIPin, int* iNotMOSIPin, int* iCLKPin, int* iNotCLKPin, int iAddress, int iBegin);


public: //public class members

	//constructor and destructor
	LedMatrix(int iDataPin, //the ID of the pin, which is connected to "DIN" on the MAX7221
		int iClkPin, //the ID of the pin, which is connected to "CLK" on the MAX7221
		int iCSPin, //the ID of the pin, which is connected to "CS" on the MAX7221
		int iLEDIntensity = 8, //the intensity of the LEDs on the matrix
		int* MatrixConfig = 0, //how the matrices are connected to each other (see below)
		bool* bSwitchedDir = 0, //tells the library if the matrices are turned by 180°
		int iMatrixNumColumns = 0, // how much matrices are in a column
		int iMatrixNumRows = 0); //how much matrices are there in a row

	/*
	"MatrixConfig" tells the function which order the real matrices have

	e.g.
	real-world positions and wirings of the matrices (matrices are represented by the numbers):

	>-0--1--2
	        |
	  5--4--3
	  |
	  6--7--8

	--> "MatrixConfig" for this configuration would look like that:
	{ 0, 1, 2,
	5, 4, 3,
	6, 7, 8 }
	    or
	MatrixConfig[0] = 0; MatrixConfig[1] = 1; MatrixConfig[2] = 2;
	MatrixConfig[3] = 5; MatrixConfig[4] = 4; MatrixConfig[5] = 3;
	MatrixConfig[6] = 6; MatrixConfig[7] = 7; MatrixConfig[8] = 8;


	in the case above, the matrices 3, 4 and 5 probably light up in a wrong way (their content is rotated by 180°)
	with "bSwitchedDir", you can turn the matrices back around by 180°, so it displays correctly (in the case above, "bSwitchedDir would probably look like { false, false, false,
	                                                                                                                                                          true,  true,  true,
	                                                                                                                                                          false, false, false } )

	you can check, if the matrices are the right orientation, by letting a point walk from the top left to the bottom right:

	LedMatrix lm = **initialize LedMatrix**;

	ClearDisplay();
	for (unsigned int x = 0; x < iMatrixNumColumns * 8; x++)
	{
		for (unsigned int y = 0; y < iMatrixNumRows * 8; y++)
		{
			SetLed(x, y, true);
			delay(100);
			SetLed(x, y, false);
		}
	}

	if the point suddenly jumps when transitioning from one matrix to another, you need to invert the matrix

	*/

	~LedMatrix();


	//send different commands (universal and specific functions)
	//send any command
	void SendCommand(int iCommandID, int iData);

	//set the intensity
	void SetIntensity(int iIntensity, int iMatrixNum);


	//set the state of one or more LEDs
	//set every LED in every matrix either to be enabled or disabled
	void ClearDisplay(bool bState = false);

	//set every LED in one matrix either to be enabled or disabled
	void ClearMatrix(int iMatrix, bool bState = false);

	//invert every LED in one matrix
	void InvertMatrixStates(int iMatrix);
	
	//set one Led to a specific state
	void SetLed(int iCoordX, int iCoordY, bool bState);

	//set the LED in a specific area to a specific state
	//void SetSubMatrix(bool* bStates, int iMatrixWidth, int iMatrixHeight);

	//set each LED in the Matrix to a specific state
	void SetMatrix(bool* bStates);

	/*
	"bStates" tells the function, which LEDs should be illuminated

	e.g.
	3x2 LED matrix where the following LEDs should be on and off:
	
	on ; on ; off;
	off; on ; off;

	--> to achieve this result, "bStates" should look like this:
	{ true, true, false,
	false, true, false }
		or
	bStates[0] = true;  bStates[1] = true;  bStates[2] = false;
	bStates[3] = false; bStates[4] = true;  bStates[5] = false;
	*/


	//2D-drawing commands
	//draw a line
	void DrawLine(int iStartX, int iStartY, int iEndX, int iEndY);

	//draw a rectangle
	//void DrawRectangle(int iLeft, int iTop, int iRight, int iBottom, bool bFill);

	//draw an ellipse
	//void DrawEllipse(int iCenterX, int iCenterY, int iRadiusX, int iRadiusY, bool bFill);

	//draw a polygon
	//void DrawPolygon(int* PointX, int* PointX, int PointCount);


	//update the matrix
	void UpdateMatrix();
};