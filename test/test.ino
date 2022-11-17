#include <BleGamepad.h> 

// #define ENABLE_BUTTONS
#define NUM_AXES 0
#define NUM_BUTTONS 32



BleGamepad bleGamepad("GPTester","Nobody",90);


void setup(){
  Serial.begin(115200);
  
  Serial.println("Starting BLE work!");

  bleGamepad.setAutoReport(false);
  bleGamepad.begin(32, 0,false,
    false,
    false,
    false,
    false,
    false,
    false,
    false);  

  

  
}

void loop(){
  delay(50);
}