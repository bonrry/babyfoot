#ifndef _BABYFOOT_H_
#define _BABYFOOT_H_

//------------------------------------------------------------------------------
//        COMMON DECLARATIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// FOR DEBUG: are goals sensors active HIGH or active LOW ?
//------------------------------------------------------------------------------
//#define  GOAL_LEVEL               HIGH   // With a push button, it active when you press -> HIGH level
#define  GOAL_LEVEL               LOW   // On the IR barrier, its active when the ball cuts the light -> LOW level 


#define  LED                      13   // Onboard debug led is on pin 13
#define  GOAL_DETECTOR_RED        2    // Red goal sensor is on pin 2
#define  GOAL_DETECTOR_BLUE       3    // Blue goal sensor is on pin 3

//------------------------------------------------------------------------------
//                 Commands for Arduino-Android communication
//
//                   !!! KEEP IN SYNC WITH ANDROID APP !!!
//------------------------------------------------------------------------------
// Commands format: <LENGHT><COMMAND><data...>
// <LENGHT>  : 1 char representing the lenght of the data (<LENGHT> char not included)
// <COMMAND> : 1 char representing the command name (alls commands are described below)
// <data>    : <LENGHT> - 1 chars representing the data to transmit

// GOAL: there is a goal. Len=2, format: 2g<TEAM>. <TEAM> is 'r' if the red scored, 'b' if the blue scored.
// Example: "2gr" -> Red goal. "2gb" -> Blue goal
#define  CMD_GOAL       'g'

// SCORE_REQ: Request the current score. Len=1, format: 1s.
#define  CMD_SCORE_REQ  's'

// SCORE_RESP: Response to current score request. Len=3, format: 3S<SCORE_RED><SCORE_BLUE>. <SCORE_RED> is the score of the red team...
// Example: "3S48" -> Red have 4, blue have 8.
#define  CMD_SCORE_RESP 'S'

// CMD_NEW_GAME: Start a new game (reset score). Len=1, format: 1n.
#define  CMD_NEW_GAME   'n'

// LED: For debug... Start/stop the onboard debug led. Len=2, format: 2l<STATE>. <STATE> is 0 to stop, 1 to start.
// Example: "2l0" -> Turn off debug led. "2l1" -> Turn ON debug led
#define  CMD_LED        'l'

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//        REAL BOARD SPECIFICS
//------------------------------------------------------------------------------
#if defined(ARDUINO)

    #define out(...) 

#else
//------------------------------------------------------------------------------
//        SIMULATOR SPECIFICS
//------------------------------------------------------------------------------

	#include <stdio.h>
	#include <fcntl.h> 
	#include <unistd.h>

	#define HIGH 1
	#define LOW  0

	#define delay(...) usleep(__VA_ARGS__ * 1000)

	// No need of pinMode
    #define pinMode(...)
    
	// No need of digitalWrite
    #define digitalWrite(...)

	int set_stdin_nonblocking() {
		const int fd = fileno(stdin);
		const int fcflags = fcntl(fd,F_GETFL);
		if (fcflags<0) { /* handle error */}
		if (fcntl(fd,F_SETFL,fcflags | O_NONBLOCK) <0) { /* handle error */} // set non-blocking
		return 0;	
	}

	/* Emulate goal sensors using the keyboard.
	 * press b<ENTER> to simulate a ball in the blue goal
	 * press r<ENTER> to simulate a ball in the red goal
	 */
    int digitalRead(int pin) {
    	char ch;
        if (pin == GOAL_DETECTOR_RED || pin == GOAL_DETECTOR_BLUE) {
        	ch = getc(stdin);
        	if (ch != EOF) {
        		if (ch == 'r' && pin == GOAL_DETECTOR_RED) {
        			return 0;	
        		}
        		if (ch == 'b' && pin == GOAL_DETECTOR_BLUE) {
        			return 0;
        		}
        		// This char will be consummed soon...
        		if (ch == 'r' || ch == 'b') {
	        		ungetc(ch, stdin);
        		}
        	}
        }
        return 1; // No ball detected
    }
    
    #define out(...) fprintf(stderr, __VA_ARGS__)

#endif  // ARDUINO


#endif  /* _BABYFOOT_H_ */
