#include <LedMatrix.h>

/*
matrices:
 __     __     __     __     _______
|3 |<--|2 |<--|1 |<--|0 |<--|Arduino|
|__|   |__|   |__|   |__|   |_______|

*/
int mc[] = {
  3, 2, 1, 0
};

bool md[] = {
  false, false, false, false
};

const int iNumMatrixRows = 4;
const int iNumMatrixColumns = 1;

LedMatrix lm = LedMatrix(12, //Data pin
                         11, //CLK pin
                         10, //CS pin
                         0,  //intiensity
                         mc, //matrix configuration
                         md, //matrix directions (are the matrices rotated by 180°)
                         iNumMatrixRows, //the number of rows
                         iNumMatrixColumns); //the number of columns


void setup()
{
  //clears the display and set all Leds to be turned off
  lm.ClearDisplay();
}

void loop()
{
  //loops through all "pixels"
  for (unsigned int y = 0; y < iNumMatrixColumns * 8; y++)
  {
    for (unsigned int x = 0; x < iNumMatrixRows * 8; x++)
    {
      //enables a Led for a small amount of time and disables it afterwards
      lm.SetLed(x, y, true); //this just changes the state of the Led
      lm.UpdateMatrix(); //this is needed to send the changes to the matrices
      delay(100);
      lm.SetLed(x, y, false);
      lm.UpdateMatrix();
    }
  }
}
