#if defined(ARDUINO)
    #include <Max3421e.h>
    #include <Usb.h>
#endif // ARDUINO

#include <AndroidAccessory.h>

#include "babyfoot.h"

AndroidAccessory acc("Bonrry",
                     "Babyfoot",
                     "Open babyfoot",
                     "0.1",
                     "https://github.com/bonrry/babyfoot",
                     "00000000000042");

//------------------------------------------------------------------------------

char checkGoals();
void usbSendScore();
void usbSendGoal(char goal);
void usbSendScoreResp();

int score_blue;
int score_red;
char msg[20];

inline void sendMessageToandroid(char msg[]) {
  acc.write(msg, msg[0] + 1); 
}

//------------------------------------------------------------------------------
extern "C"{
    
// Called one time only, every time the arduino boots
void setup() {
  pinMode(LED, OUTPUT);
  pinMode(GOAL_DETECTOR_RED, INPUT);
  pinMode(GOAL_DETECTOR_BLUE, INPUT);
  digitalWrite(GOAL_DETECTOR_RED, HIGH);
  digitalWrite(GOAL_DETECTOR_BLUE, HIGH);
  digitalWrite(LED, LOW);
  
  score_blue = 0;
  score_red  = 0;
  
  // Init AndroidAccessory (starts USB host)
  acc.powerOn();

#if !defined(ARDUINO)
  set_stdin_nonblocking();
#endif

}

// Main program loop
void loop() {

  char goal = 0;
  goal = checkGoals();

  if (acc.isConnected()) {
    // An Android device is connected
    
    // Send goal status if needed
    if (goal) {
      usbSendGoal(goal);
    }
    
    // Read USB for incoming commands (if any)
    int len = acc.read(msg, sizeof(msg), 1);
    if (len > 0) {
      
      // assumes only one command per packet
      if (msg[0] >= 0x1) {
        if (msg[1] == CMD_LED) {
          digitalWrite(LED, msg[2] ? HIGH : LOW);
        }
        if (msg[1] == CMD_SCORE_REQ) {
          out("[CMD]Score req received\n");
          usbSendScoreResp();
        }
        if (msg[1] == CMD_NEW_GAME) {
          out("[CMD]New game received\n");
          score_blue = 0;
          score_red  = 0;
        }
      }
    }
    msg[0] = 0;
  } else {
    // No Android device connected

    // reset outputs to default values on disconnect
  }
  
  if (goal) {
    digitalWrite(LED, HIGH);
    delay(1000); // delay in between goals
    digitalWrite(LED, LOW);
  }
}
//------------------------------------------------------------------------------
void quit() {
#if !defined(ARDUINO)
    acc.close();
#endif
}
} // extern "C"

char checkGoals() {
  int red = digitalRead(GOAL_DETECTOR_RED);
  int blue = digitalRead(GOAL_DETECTOR_BLUE);
  if (red == GOAL_LEVEL) {
    score_blue++;
    return 'r';
  }
  if (blue == GOAL_LEVEL) {
    score_red++;
    return 'b';
  }
  return 0;
}

void usbSendScoreResp() {
  msg[0] = 0x3;
  msg[1] = CMD_SCORE_RESP;
  msg[2] = score_red;
  msg[3] = score_blue;
  out("[CMD]Send score: RED %d - BLUE %d\n", score_red, score_blue);
  sendMessageToandroid(msg); 
}
 
void usbSendGoal(char goal) {
  msg[0] = 0x2;
  msg[1] = CMD_GOAL;
  msg[2] = goal;
  out("[CMD]GOAL for the %s team\n", (goal == 'r')? "BLUE": "RED");
  sendMessageToandroid(msg);
}
