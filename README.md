# LedMatrix
A fast library for displaying something through the MAX7221 chip (and probably also the MAX7219)

Note: It uses its own implementation of "digitalWrite()", which is specific to the ATmega168/328P   
(it may work on other chips but it was not designed to do so)

Getting started
---------------
To use this library, you need to put the folder containing all of the LedMatrix files into the "libraries" folder located in the "Arduino" root folder.
All the functions and their descriptions are in "LedMatrix.h".

Todos
----
 * make more examples
 * improve DrawLine performance
 * add draw commands for other simple geometric forms (e.g. rectangles, circles)
