/*
    Babyfoot simulator
*/

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

extern void loop();
extern void setup();
extern void quit();

int running = 1;

static void sig_hdlr(int signum) {
	switch (signum) {
    	case SIGINT:
            running = 0;
            break;
    }
}

/*------------------------------------------------------------------------*
 * Simulator main.                                                        *
 *------------------------------------------------------------------------*/
int main(void) {
    signal(SIGINT, &sig_hdlr);

    setup();
    while(running) {loop();}
    quit();
    return 0;
}
