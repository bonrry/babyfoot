*****************************************************************************
** Babyfoot arduino code and simulator running into a Linux/MacOS X process**
*****************************************************************************

** ARDUINO **

This sketch is intented to run on an ARDUINO ADK MEGA board.
The arduino code is located in its babyfoot/ "sketch" directory.
You must install the 2 arduino librairies provided in libraries/ to use this project (AndroidAccessory and USB_Host_Shield).
Open babyfoot/babyfoot.ino with arduino IDE and flash it on the board as usually.

** SIMULATOR **

The babyfoot simulator runs under Linux/MacOS as an application program.
It uses libusb to emulate an USB HOST ADK babyfoot.
Use 'make all' to build it, then use runSimulator.sh to use it. Plug an Android device 
to the computer via USB and start testing your app.