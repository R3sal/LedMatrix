//include the header file
#include "LedMatrix.h"

//the class constructor
LedMatrix::LedMatrix(int iDataPin, int iClkPin, int iCSPin, int iLEDIntensity,
	int* MatrixConfig, bool* bSwitchedDir, int iMatrixNumColumns, int iMatrixNumRows)
{
	//set the number of rows and columns
	m_iColumns = iMatrixNumColumns;
	m_iRows = iMatrixNumRows;

	//determin the number of matrices overall by the multiplication of matrix rows and columns
	m_iNumMatrices = m_iColumns * m_iRows;

	//copy the order of the matrices and their direction into the class member list
	m_MatrixConfig = MatrixConfig;
	m_MatrixDirSwitched = bSwitchedDir;

	//initialize the state of each Led in the matrix
	m_LedState = new bool;
	for (int i = 0; i < 64 * m_iNumMatrices; i++)
		m_LedState[i] = false;

	//the pin registers
	m_pMOSIPinReg = (iDataPin >= 8) ? (int*)&PORTB : (int*)&PORTD;
	m_pCLKPinReg = (iClkPin >= 8) ? (int*)&PORTB : (int*)&PORTD;
	m_pCSPinReg = (iCSPin >= 8) ? (int*)&PORTB : (int*)&PORTD;
	//the pin numbers in their register
	m_iMOSIPinNum = (iDataPin >= 8) ? iDataPin - 8 : iDataPin;
	m_iCLKPinNum = (iClkPin >= 8) ? iClkPin - 8 : iClkPin;
	m_iCSPinNum = (iCSPin >= 8) ? iCSPin - 8 : iCSPin;

	//set the pinmodes of the pins selected as outputs
	pinMode(iDataPin, OUTPUT);
	pinMode(iClkPin, OUTPUT);
	pinMode(iCSPin, OUTPUT);


	//initialize the matrices
	//repeat every command for each matrix and latch it into the controllers registers afterwards
	//prepare the controllers for receiving data
	PullDownCS();
	for (int i = 0; i < m_iNumMatrices; i++)
	{
		//disable display test mode
		SendData(15, 0);
	}
	//latch the data into the controllers
	PullUpCS();

	//prepare the controllers for receiving data
	PullDownCS();
	for (int i = 0; i < m_iNumMatrices; i++)
	{
		//go out of shutdown mode
		SendData(12, 1);
	}
	//latch the data into the controllers
	PullUpCS();

	//prepare the controllers for receiving data
	PullDownCS();
	for (int i = 0; i < m_iNumMatrices; i++)
	{
		//set the scan limit to the maximum, so every digit is displayed
		SendData(11, 7);
	}
	//latch the data into the controllers
	PullUpCS();

	//prepare the controllers for receiving data
	PullDownCS();
	for (int i = 0; i < m_iNumMatrices; i++)
	{
		//turn off any decoding
		SendData(9, 0);
	}
	//latch the data into the controllers
	PullUpCS();

	//prepare the controllers for receiving data
	PullDownCS();
	for (int i = 0; i < m_iNumMatrices; i++)
	{
		//set the intensity to the value which was passed to this function
		SendData(10, iLEDIntensity);
	}
	//latch the data into the controllers
	PullUpCS();
}

//the class destructor
LedMatrix::~LedMatrix()
{

}



//private functions
//functions for sending data trough the defined output pins
//prepare for data transfer by pulling the CS-Pin to the ground
void LedMatrix::PullDownCS()
{
	*m_pCSPinReg &= NotTwoToThe[m_iCSPinNum];
}


//latch the data into the controller by pulling the CS-pin to the current voltage
void LedMatrix::PullUpCS()
{
	*m_pCSPinReg |= TwoToThe[m_iCSPinNum];
}


//send any data to the matrix
void LedMatrix::SendData(int iAddress, int iData)
{
	//the first four bits are ignored by the controller, so just clock through them
	*m_pMOSIPinReg &= NotTwoToThe[m_iMOSIPinNum];
	for (int i = 0; i < 4; i++)
	{
		//set the clock pin to a low power state
		*m_pCLKPinReg &= NotTwoToThe[m_iCLKPinNum];

		//set the clock pin to a high power state to send the data
		*m_pCLKPinReg |= TwoToThe[m_iCLKPinNum];
	}

	//the next four bits indicate the address (in our case the row of the matrix)
	for (int i = 0; i < 4; i++)
	{
		//set the clock pin to a low power state
		*m_pCLKPinReg &= NotTwoToThe[m_iCLKPinNum];

		//send one bit, either 0 or 1, depending on the value in "iAddress"
		if (iAddress >= TwoToThe[3 - i])
		{
			//if bit "i" in "iAddress" is 1, set the data pin to HIGH...
			*m_pMOSIPinReg |= TwoToThe[m_iMOSIPinNum];
			//and subtract "iAddress" by the binary significance value of "i"
			iAddress -= TwoToThe[3 - i];
		}
		else
		{
			//...if bit "i" in "iAddress" is 0, set the data pin to LOW
			*m_pMOSIPinReg &= NotTwoToThe[m_iMOSIPinNum];
		}

		//set the clock pin to a high power state to send the data
		*m_pCLKPinReg |= TwoToThe[m_iCLKPinNum];
	}

	//the last eight bits indicate the data (in our case the state of each LED in the row)
	for (int i = 0; i < 8; i++)
	{
		//set the clock pin to a low power state
		*m_pCLKPinReg &= NotTwoToThe[m_iCLKPinNum];

		//send one bit, either 0 or 1, depending on the value in "iData"
		if (iData >= TwoToThe[7 - i])
		{
			//if bit "i" in "iData" is 1, set the data pin to HIGH...
			*m_pMOSIPinReg |= TwoToThe[m_iMOSIPinNum];
			//and subtract "iData" by the binary significance value of "i"
			iData -= TwoToThe[7 - i];
		}
		else
		{
			//...if bit "i" in "iData" is 0, set the data pin to LOW
			*m_pMOSIPinReg &= NotTwoToThe[m_iMOSIPinNum];
		}

		//set the clock pin to a high power state to send the data
		*m_pCLKPinReg |= TwoToThe[m_iCLKPinNum];
	}
}


//send 8 values from m_LedState to the matrix controller
void LedMatrix::SendLEDStates(int* iMOSIPin, int* iNotMOSIPin, int* iCLKPin, int* iNotCLKPin, int iAddress, int iBegin)
{
	//the first four bits are ignored by the controller, so just clock through them
	*m_pMOSIPinReg &= *iNotMOSIPin;
	for (int i = 0; i < 4; i++)
	{
		//set the clock pin to a low power state
		*m_pCLKPinReg &= *iNotCLKPin;

		//make a small delay, so the controller has enough time to register the clocks
		__asm ("nop ;");

		//set the clock pin to a high power state to send the data
		*m_pCLKPinReg |= *iCLKPin;
	}

	//the next four bits indicate the address (in our case the row of the matrix)
	for (int i = 0; i < 4; i++)
	{
		//set the clock pin to a low power state
		*m_pCLKPinReg &= *iNotCLKPin;

		//send one bit, either 0 or 1, depending on the value in "iAddress"
		if (iAddress >= TwoToThe[3 - i])
		{
			//if bit "i" in "iAddress" is 1, set the data pin to HIGH...
			*m_pMOSIPinReg |= *iMOSIPin;
			//and subtract "iAddress" by the binary significance value of "i"
			iAddress -= TwoToThe[3 - i];
		}
		else
		{
			//...if bit "i" in "iAddress" is 0, set the data pin to LOW
			*m_pMOSIPinReg &= *iNotMOSIPin;
		}

		//set the clock pin to a high power state to send the data
		*m_pCLKPinReg |= *iCLKPin;
	}

	//the last eight bits indicate the data (in our case the state of each LED in the row)
	for (int i = 0; i < 8; i++)
	{
		//set the clock pin to a low power state
		*m_pCLKPinReg &= *iNotCLKPin;

		//send one bit, either 0 or 1, depending on the value in "m_LedState"
		if (m_LedState[iBegin + i])
		{
			//if bit "i" in "iData" is 1, set the data pin to HIGH...
			*m_pMOSIPinReg |= *iMOSIPin;
		}
		else
		{
			//...if bit "i" in "iData" is 0, set the data pin to LOW
			*m_pMOSIPinReg &= *iNotMOSIPin;
		}

		//set the clock pin to a high power state to send the data
		*m_pCLKPinReg |= *iCLKPin;
	}
}



//public functions
//universal and specific functions for sending commands
//send any command to any matrix
void LedMatrix::SendCommand(int iCommandID, int iData)
{
	//set the CS pin to low, so we can send data to the matrix controller
	*m_pCSPinReg &= NotTwoToThe[m_iCSPinNum];

	/*just send the data we got after limiting them to the range
	(0 - 15 for iAddress (4-bit) and 0 - 255 for iData (8-bit))*/
	SendData(min(max(iCommandID, 0), 15), min(max(iData, 0), 255));

	/*set the CS pin to high, so the data get latched into the registers of the controller
	and the command gets executed*/
	*m_pCSPinReg |= TwoToThe[m_iCSPinNum];
}

//set the intensity for one or more matrices
void LedMatrix::SetIntensity(int iIntensity, int iMatrixNum)
{
	//set the CS pin to low, so we can send data to the matrix controller
	*m_pCSPinReg &= NotTwoToThe[m_iCSPinNum];

	//send one command per matrix
	for (int i = m_iNumMatrices; i > 0; i--)
	{
		//if (i - 1) is equal to the number of the matrix send the command...
		if (iMatrixNum == i - 1)
			SendData(10, iIntensity);
		//if it isn't, just send a no-op command, so the matrix doesn't change
		else
			SendData(0, 0);
	}

	/*set the CS pin to high, so the data get latched into the registers of the controller
	and the command gets executed*/
	*m_pCSPinReg |= TwoToThe[m_iCSPinNum];
}


//set the state of one or more LED
//set every LED in every matrix either to be enabled or disabled
void LedMatrix::ClearDisplay(bool bState)
{
	//repeat it for each LED in each matrix
	for (int i = 0; i < 64 * m_iNumMatrices; i++)
	{
		//set the LED state to the value which was passed to this function
		m_LedState[i] = bState;
	}
}

//set every LED in one matrix either to be enabled or disabled
void LedMatrix::ClearMatrix(int iMatrix, bool bState)
{
	//make some calculations before entering the loops
	int iLedsPerRow = 8 * m_iNumMatrices;
	int iEightTimesMatrixNum = 8 * (m_iNumMatrices - 1 - m_MatrixConfig[iMatrix]);

	//repeat it for each row in one matrix
	for (int i = 0; i < 8; i++)
	{
		//repeat for each LED in one row of one matrix
		for (int j = 0; j < 8; j++)
		{
			//set the LED state to the value which was passed to this function
			m_LedState[j + iEightTimesMatrixNum + i * iLedsPerRow] = bState;
		}
	}
}

//invert every LED in one matrix
void LedMatrix::InvertMatrixStates(int iMatrix) //todo: rename and make a version which changes the whole display
{
	//make some calculations before entering the loops
	int iLedsPerRow = 8 * m_iNumMatrices;
	int iEightTimesMatrixNum = 8 * (m_iNumMatrices - 1 - m_MatrixConfig[iMatrix]);

	//repeat it for each row in one matrix
	for (int i = 0; i < 8; i++)
	{
		//repeat for each LED in one row of one matrix
		for (int j = 0; j < 8; j++)
		{
			int LedIndex = j + iEightTimesMatrixNum + i * iLedsPerRow;
			//set the LED state to the value which was passed to this function
			m_LedState[LedIndex] = !m_LedState[LedIndex];
		}
	}
}

//set one Led to a specific state
void LedMatrix::SetLed(int iCoordX, int iCoordY, bool bState)
{
	//calculate the number of the matrix, the pixel gets displayed on
	int iMatrixX = (int)floorf(iCoordX / 8);
	int iMatrixY = (int)floorf(iCoordY / 8);
	int iMatrixNum = iMatrixY * m_iColumns + iMatrixX;
	int iFinalMatrix = m_MatrixConfig[iMatrixNum];

	//calculate the position on the 8x8 matrix
	int iLocalX = iCoordX % 8;
	int iLocalY = iCoordY % 8;

	//calculate the index we are searching for
	int iLedStateNum = 0;
	/*before making calculations, we need to know whether the matrix,
	where the point is displayed on is turned around by 180° or not*/
	if (m_MatrixDirSwitched[iMatrixNum])
		//if it is, we need to calculate a little bit more
		iLedStateNum = (7 - iLocalX + (8 * (m_iNumMatrices - 1 - iFinalMatrix))) +
			(56 * m_iNumMatrices) - (iLocalY * 8 * m_iNumMatrices);
	else
		//if it isn't, just do the basic calculations
		iLedStateNum = (iLocalX + (8 * (m_iNumMatrices - 1 - iFinalMatrix))) + (iLocalY * 8 * m_iNumMatrices);

	//set the correct index of the LED state list to the state which was passed to this function
	m_LedState[iLedStateNum] = bState;
}

//set one Led to a specific state
void LedMatrix::InvertLed(int iCoordX, int iCoordY)
{
	//calculate the number of the matrix, the pixel gets displayed on
	int iMatrixX = (int)floorf(iCoordX / 8);
	int iMatrixY = (int)floorf(iCoordY / 8);
	int iMatrixNum = iMatrixY * m_iColumns + iMatrixX;
	int iFinalMatrix = m_MatrixConfig[iMatrixNum];

	//calculate the position on the 8x8 matrix
	int iLocalX = iCoordX % 8;
	int iLocalY = iCoordY % 8;

	//calculate the index we are searching for
	int iLedStateNum = 0;
	/*before making calculations, we need to know whether the matrix,
	where the point is displayed on is turned around by 180° or not*/
	if (m_MatrixDirSwitched[iMatrixNum])
		//if it is, we need to calculate a little bit more
		iLedStateNum = (7 - iLocalX + (8 * (m_iNumMatrices - 1 - iFinalMatrix))) +
			(56 * m_iNumMatrices) - (iLocalY * 8 * m_iNumMatrices);
	else
		//if it isn't, just do the basic calculations
		iLedStateNum = (iLocalX + (8 * (m_iNumMatrices - 1 - iFinalMatrix))) + (iLocalY * 8 * m_iNumMatrices);

	//set the correct index of the LED state list to the state which was passed to this function
	m_LedState[iLedStateNum] = !(m_LedState[iLedStateNum]);
}

//set each LED in the Matrix to a specific state
void LedMatrix::SetMatrix(bool* bStates)
{
	//repeat for each row
	for (int i = 0; i < 8; i++)
	{
		//precalculate this value, so it doesn't have to be re-calculated in each loop
		int iLedRowPreCalc = i * 8 * m_iNumMatrices;

		//repeat for each matrix
		for (int j = 0; j < m_iNumMatrices; j++)
		{
			//get the number of the matrix, the pixel gets displayed on
			int iFinalMatrix = m_MatrixConfig[j];

			//in this variable the calculated index gets stored in
			int iLedStateNum = 0;

			//precalculate some values, so they don't have to be re-calculated in each loop
			int bStateNumPreCalc = 8 * (j % m_iColumns) + 8 * m_iColumns * (i + 8 * (int)floorf(j / m_iColumns));
			int iFinalMatrixPreCalc = 8 * (m_iNumMatrices - 1 - iFinalMatrix);
			
			/*before making calculations, we need to know whether the matrix,
			where the point is displayed on is turned around by 180° or not*/
			if (m_MatrixDirSwitched[j])
			{
				for (int k = 0; k < 8; k++)
				{
					//if it is, we need to calculate a little bit more
					iLedStateNum = (7 - k + iFinalMatrixPreCalc) + (56 * m_iNumMatrices - iLedRowPreCalc);
				
					//set the correct index of the LED state list to the state which was passed to this function
					m_LedState[iLedStateNum] = bStates[k + bStateNumPreCalc];
				}
			}
			else
			{
				for (int k = 0; k < 8; k++)
				{
					//if it isn't, just do the basic calculations
					iLedStateNum = (k + iFinalMatrixPreCalc) + (iLedRowPreCalc);

					//set the correct index of the LED state list to the state which was passed to this function
					m_LedState[iLedStateNum] = bStates[k + bStateNumPreCalc];
				}
			}
		}
	}
}



//2D-drawing commands
//draw a line
void LedMatrix::DrawLine(int iStartX, int iStartY, int iEndX, int iEndY)
{
	int Xdiff = iStartX - iEndX;
	int Ydiff = iStartY - iEndY;

	if ((Xdiff == 0) && (Ydiff == 0))
	{
		SetLed(iStartX, iStartY, true);
	}
	else if (abs(Xdiff) > abs(Ydiff))
	{

		float XCoeff = -((float)Xdiff) / (float)abs(Xdiff);
		float YCoeff = -((float)Ydiff) / (float)abs(Xdiff);

		for (int i = 0; i <= abs(Xdiff); i++)
		{
			int CurrX = (int)roundf(XCoeff * (float)i) + iStartX;
			int CurrY = (int)roundf(YCoeff * (float)i) + iStartY;

			if ((CurrX >= 0) && (CurrX <= 31) && (CurrY >= 0) && (CurrY <= 23))
			{
				SetLed(CurrX, CurrY, true);
			}
		}
	}
	else
	{

		float XCoeff = -((float)Xdiff) / (float)abs(Ydiff);
		float YCoeff = -((float)Ydiff) / (float)abs(Ydiff);

		for (int i = 0; i <= abs(Ydiff); i++)
		{
			int CurrX = (int)roundf(XCoeff * (float)i) + iStartX;
			int CurrY = (int)roundf(YCoeff * (float)i) + iStartY;

			if ((CurrX >= 0) && (CurrX <= 31) && (CurrY >= 0) && (CurrY <= 23))
			{
				SetLed(CurrX, CurrY, true);
			}
		}
	}
}

//draw a rectangle
//void LedMatrix::DrawRectangle(int iLeft, int iTop, int iRight, int iBottom, bool bFill);

//draw an ellipse
//void LedMatrix::DrawEllipse(int iCenterX, int iCenterY, int iRadiusX, int iRadiusY, bool bFill);

//draw a polygon
//void LedMatrix::DrawPolygon(int* PointX, int* PointX, int PointCount);



//update the matrix
void LedMatrix::UpdateMatrix()
{
	//get the values ?????????????????? to speed things up
	int iMOSIPin = TwoToThe[m_iMOSIPinNum];
	int iNotMOSIPin = NotTwoToThe[m_iMOSIPinNum];
	int iCLKPin = TwoToThe[m_iCLKPinNum];
	int iNotCLKPin = NotTwoToThe[m_iCLKPinNum];

	int iTotalColumns = 8 * m_iNumMatrices;

	//repeat the process for each row
	for (int i = 0; i < 8; i++)
	{
		//set the CS pin to low, so we can send data to the matrix controller
		*m_pCSPinReg &= NotTwoToThe[m_iCSPinNum];

		//repeat it for each matrix
		for (int j = 0; j < iTotalColumns; j += 8)
		{
			//send the LED states to the matrix controller
			SendLEDStates(&iMOSIPin, &iNotMOSIPin, &iCLKPin, &iNotCLKPin, i + 1, j + iTotalColumns * i);
		}

		/*set the CS pin to high, so the data get latched into the registers of the controller
		and the LEDs get enabled*/
		*m_pCSPinReg |= TwoToThe[m_iCSPinNum];
	}



	/*/repeat the process for each row
	for (int i = 0; i < 8; i++)
	{
		//set the CS pin to low, so we can send data to the matrix controller
		*m_pCSPinReg &= NotTwoToThe[m_iCSPinNum];

		//repeat it for each matrix
		for (int j = 0; j < m_iNumMatrices; j++)
		{
			//send the LED states to the matrix controller
			SendLEDStates(&iMOSIPin, &iNotMOSIPin, &iCLKPin, &iNotCLKPin, i + 1, 8 * (j + m_iNumMatrices * i));
		}

		/*set the CS pin to high, so the data get latched into the registers of the controller
		and the LEDs get enabled/
		*m_pCSPinReg |= TwoToThe[m_iCSPinNum];
	}*/


	//for eventual debugging; will be removed
	/*for (int i = 0; i < m_iNumMatrices; i++)
	{
		Serial.print(i);
		Serial.print(";\t");
		Serial.println(m_MatrixConfig[i]);
	}*/
}