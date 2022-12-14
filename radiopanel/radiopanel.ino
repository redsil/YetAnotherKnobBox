#include "panel_led.h"
#include "knob.h"


enum MODE { MODE_NONE, MODE_1, MODE_2, MODE_3, MODE_4, MODE_5, MODE_6, BRIGHTNESS_ADJUST, SET_TIMER };


#include <BleGamepad.h> 

#define LED_PIN    2
#define LED_COUNT 8
#define UPDATE_INTERVAL 40


// #define ENABLE_BUTTONS
#define NUM_AXES 4
#define NUM_BUTTONS 32

#define NUM_MODE_PINS 6
#define NUM_BUTTONS_PER_KNOB 12
#define KNOB1_BUTTON_OFFSET 0
#define KNOB2_BUTTON_OFFSET NUM_BUTTONS_PER_KNOB


#define ENCODER1_DIR_BUTTON_OFFSET 2
#define ENCODER1_VAL_BUTTON_OFFSET ENCODER1_DIR_BUTTON_OFFSET + 2
#define ENCODER2_DIR_BUTTON_OFFSET ENCODER1_VAL_BUTTON_OFFSET + 3
#define ENCODER2_VAL_BUTTON_OFFSET ENCODER2_DIR_BUTTON_OFFSET + 2

#define RELEASE_MASK  ~( (0x7 << ENCODER1_VAL_BUTTON_OFFSET) | (0x7 << ENCODER2_VAL_BUTTON_OFFSET) \
                       | (0x7 << (ENCODER1_VAL_BUTTON_OFFSET + KNOB2_BUTTON_OFFSET) ) | (0x7 << (ENCODER2_VAL_BUTTON_OFFSET + KNOB2_BUTTON_OFFSET) ) ) 


#define MODE_BUTTON_OFFSET NUM_BUTTONS_PER_KNOB * 2

BleGamepad bleGamepad("KnobPanel","Rpierce",90);

DualEncoderKnob knob1(15,16,17,18,13,0);
DualEncoderKnob knob2(19,21,22,23,12,1);

PanelLed strip(LED_PIN,10);

bool debug = false;

int mode_button_pins[NUM_MODE_PINS] = { 33, 25, 26, 32, 27, 14 };
char g_last_mode_pin_state = 0;

int g_button_index[NUM_BUTTONS] = { 
  BUTTON_1,
  BUTTON_2,
  BUTTON_3,
  BUTTON_4,
  BUTTON_5,
  BUTTON_6,
  BUTTON_7,
  BUTTON_8,
  BUTTON_9,
  BUTTON_10,
  BUTTON_11,
  BUTTON_12,
  BUTTON_13,
  BUTTON_14,
  BUTTON_15,
  BUTTON_16,
  BUTTON_17,
  BUTTON_18,
  BUTTON_19,
  BUTTON_20,
  BUTTON_21,
  BUTTON_22,
  BUTTON_23,
  BUTTON_24,
  BUTTON_25,
  BUTTON_26,
  BUTTON_27,
  BUTTON_28,
  BUTTON_29,
  BUTTON_30
};

MODE current_mode = MODE_NONE;
MODE prev_mode = MODE_NONE;
MODE long_mode = MODE_NONE;

int g_timer_value = 0;
int button_hold_count = 0;

// Each time through the loop we alternate make controller button presses
// with releasing buttons.  This gives pulses for each turn of the encoder
boolean release_cycle = true;


// Setup the interrupt handler.  Can't be inside the class
void IRAM_ATTR intr1_() {
  // We need to monitor both pins, rising and falling for all states
  knob1.process1();
} 
void IRAM_ATTR intr2_() {
  // We need to monitor both pins, rising and falling for all states
  knob1.process2();
} 
// Setup the interrupt handler.  Can't be inside the class
void IRAM_ATTR intr3_() {
  // We need to monitor both pins, rising and falling for all states
  knob2.process1();
} 
// Setup the interrupt handler.  Can't be inside the class
void IRAM_ATTR intr4_() {
  // We need to monitor both pins, rising and falling for all states
  knob2.process2();
} 


void setup(){
  Serial.begin(115200);

  for (int mode = 0; mode < 6; mode++) {
    pinMode(mode_button_pins[mode],INPUT_PULLUP);  
  }
  
  Serial.println("Starting BLE work!");
  BleGamepadConfiguration gp_config;
  gp_config.setAutoReport(false);
  gp_config.setWhichAxes(true,true,false,true,true,false,false,false); 
//  gp_config.setControllerType(CONTROLLER_TYPE_MULTI_AXIS);
  gp_config.setButtonCount(32);  
//  gp_config.setModelNumber("1");
//  gp_config.setSoftwareRevision("1.1");
//  gp_config.setSerialNumber("00001");
  gp_config.setFirmwareRevision("1.2");
  gp_config.setHardwareRevision("1.0");
  // reset rudder trim, reset aileron trim, reset elevator trim, switch knob3 modes

  bleGamepad.begin(&gp_config);

  
  knob1.init();
  knob2.init();
  
  strip.begin();
  //  strip.rainbow(10);             // Flowing rainbow cycle along the whole strip
  strip.show();

  attachInterrupt(digitalPinToInterrupt(knob1.get_pinA()), intr1_, CHANGE);
  attachInterrupt(digitalPinToInterrupt(knob1.get_pinB()), intr1_, CHANGE);
  attachInterrupt(digitalPinToInterrupt(knob1.get_pina()), intr2_, CHANGE);
  attachInterrupt(digitalPinToInterrupt(knob1.get_pinb()), intr2_, CHANGE);

  attachInterrupt(digitalPinToInterrupt(knob2.get_pinA()), intr3_, CHANGE);
  attachInterrupt(digitalPinToInterrupt(knob2.get_pinB()), intr3_, CHANGE);
  attachInterrupt(digitalPinToInterrupt(knob2.get_pina()), intr4_, CHANGE);
  attachInterrupt(digitalPinToInterrupt(knob2.get_pinb()), intr4_, CHANGE);
  

}

void loop(){
  boolean update_controller = false;

  if (release_cycle) {
    if (release_buttons(bleGamepad) && bleGamepad.isConnected()) bleGamepad.sendReport() ;
  }
  else {
    // Check for mode change
    int mode_button_changed = get_mode_pin_change();

    // Set a new mode if button released
    if (mode_button_changed < 0) {
      if (current_mode > MODE_6) {
	      if (current_mode == SET_TIMER) {
	        // Start timer on release
	        strip.startTimer();
        }
        
        MODE tmp_mode = current_mode;
        current_mode = prev_mode;
        prev_mode = tmp_mode;
        strip.clear();

      }
      else {
        int mode_offset = abs(mode_button_changed) - 1;
        int button_offset = MODE_BUTTON_OFFSET + mode_offset;

        bleGamepad.press(g_button_index[button_offset]);
        
        update_controller = true;

        prev_mode = current_mode;
        current_mode = static_cast<MODE>(static_cast<int>(MODE_NONE) - mode_button_changed);

      }
      // Show the light for this mode
      strip.show_mode(current_mode - static_cast<int>(MODE_1));

      // disable long press waiting
      button_hold_count = 0;
    }

    // Start long press detection on button press
    else if (mode_button_changed > 0) {
      button_hold_count = 20;
      if (mode_button_changed == 1) {
	      long_mode = BRIGHTNESS_ADJUST;
      }
      if (mode_button_changed == 3) {
	      long_mode = SET_TIMER;
      }
    }

    // Long press detection , change mode after hold count
    if (button_hold_count) {
      button_hold_count--;

      if (button_hold_count <= 0) {
	      prev_mode = current_mode;
	      current_mode = long_mode;
      }
    }

    // reset left/right blinker if it is currently on
    // Gets set during process_encoder if the knob is being moved
    if (strip.leftOn() || strip.rightOn()) {
      strip.clearRight();
      strip.clearLeft();
      strip.show();
    }
    
    boolean encoder_movement = false;
    if (process_encoder(knob1,KNOB1_BUTTON_OFFSET,bleGamepad)) encoder_movement = true;
    if (process_encoder(knob2,KNOB2_BUTTON_OFFSET,bleGamepad)) encoder_movement = true;

    
    update_controller = encoder_movement || update_controller;
  
    if (current_mode >= MODE_1 && current_mode <= MODE_6) {
      if (update_controller) {
	      print_debug(bleGamepad);
      	if (bleGamepad.isConnected()) bleGamepad.sendReport();
      }
      if (bleGamepad.isPressed(g_button_index[KNOB1_BUTTON_OFFSET + ENCODER1_DIR_BUTTON_OFFSET]) || bleGamepad.isPressed(g_button_index[KNOB1_BUTTON_OFFSET + ENCODER2_DIR_BUTTON_OFFSET]) ||
      	  bleGamepad.isPressed(g_button_index[KNOB2_BUTTON_OFFSET + ENCODER1_DIR_BUTTON_OFFSET]) || bleGamepad.isPressed(g_button_index[KNOB2_BUTTON_OFFSET + ENCODER2_DIR_BUTTON_OFFSET])) {
        strip.setLeft();
        strip.show();
      }
      if (bleGamepad.isPressed(g_button_index[KNOB1_BUTTON_OFFSET + ENCODER1_DIR_BUTTON_OFFSET + 1]) || bleGamepad.isPressed(g_button_index[KNOB1_BUTTON_OFFSET + ENCODER2_DIR_BUTTON_OFFSET + 1]) ||
	        bleGamepad.isPressed(g_button_index[KNOB2_BUTTON_OFFSET + ENCODER1_DIR_BUTTON_OFFSET + 1]) || bleGamepad.isPressed(g_button_index[KNOB2_BUTTON_OFFSET + ENCODER2_DIR_BUTTON_OFFSET + 1])) {
        strip.setRight();
        strip.show();
      }
    }
    else if (current_mode == BRIGHTNESS_ADJUST) {
      strip.rainbow(100 << 8);
      if (bleGamepad.isPressed(g_button_index[KNOB2_BUTTON_OFFSET + ENCODER1_DIR_BUTTON_OFFSET])) {
      	if (strip.getBrightness() > 10) strip.setBrightness(strip.getBrightness() - 10);
      }
      if (bleGamepad.isPressed(g_button_index[KNOB2_BUTTON_OFFSET + ENCODER1_DIR_BUTTON_OFFSET + 1])) {
	      if (strip.getBrightness() < 245) strip.setBrightness(strip.getBrightness() + 10);
      }
      strip.show();
    }
    else if (current_mode == SET_TIMER) {
      if (bleGamepad.isPressed(g_button_index[KNOB2_BUTTON_OFFSET + ENCODER1_DIR_BUTTON_OFFSET])) {
	      if (g_timer_value >= 5) g_timer_value -= 5;
      }
      if (bleGamepad.isPressed(g_button_index[KNOB2_BUTTON_OFFSET + ENCODER1_DIR_BUTTON_OFFSET + 1])) {
	      if (g_timer_value < (strip.getMaxTimerValue() - 5)) g_timer_value += 5;
      }
      strip.setTimer(g_timer_value);
      strip.showTimer();
    }  
  }

  // don't run the timer while we are setting it
  if (current_mode != SET_TIMER) {
    strip.runTimer();
  }

  // Flip betweewn pressing and releasing buttons
  release_cycle = !release_cycle;
  delay(UPDATE_INTERVAL);
}

void print_debug(BleGamepad &gp) {
  if (!debug) return;
  Serial.printf("DEBUG Gamepad ");
  for (int i = 0; i < 32; i++) {
    if (gp.isPressed(g_button_index[i])) {
      Serial.printf("%d",1);
    }
    else {
      Serial.printf("%d",0);
    }      
  }  
  Serial.println();
}


boolean process_encoder(DualEncoderKnob &knob, int gamepad_button_offset, BleGamepad &gp) {
  // Returns true if buttons are being pressed
  // First button is the encoder button itself
  // second and third buttons are left turn and right turn
  // fourth through sixth are a 3 bit encoding of how many encoder ticks
  // happened since the last processing.  0 means changed by 1,
  // 7 means changed by 8. If buttons two and three are zero, the last
  // three buttons are ignored
  boolean is_change = false;

  // Short and long presses of button trigger two differetn gamepad buttons upon release
  if (!knob.buttonPressed()) {
    if (knob.timeSincePressed() > 0) {
      if (knob.timeSincePressed() > 1000) {
        // long press
        gp.press(g_button_index[gamepad_button_offset+1]);
      }
      else {
        // short press
        gp.press(g_button_index[gamepad_button_offset]);
      }
      is_change = true;
      knob.resetPressed();
    }
  }
  // button pressed
  else {
    if (knob.timeSincePressed() == 0) {
      knob.setPressed();
    }
  }
	    

  int knob_offset[2];
  knob_offset[0] = gamepad_button_offset + ENCODER1_DIR_BUTTON_OFFSET;
  knob_offset[1] = gamepad_button_offset + ENCODER2_DIR_BUTTON_OFFSET;
  
  int axis_value[4] = { 0,0,0,0 };
  for (int encoder_id = 0; encoder_id < 2; encoder_id++) {
    int offset = knob_offset[encoder_id];
      
    int value = knob.getChange(encoder_id);
    int magnitude = abs(value);

    if (knob.get_id() == 0) {
      if (encoder_id == 0) {
        gp.setX(value);
      }
      else {
        gp.setY(value);
      }
    }
    else {
      if (encoder_id == 0) {
        gp.setRX(value);
      }
      else {
        gp.setRY(value);
      }
    }

    if (value < 0) {
//      Serial.printf("DEBUG %d: count = %d change = %d ",encoder_id,knob.getCount(encoder_id),value);
//      Serial.println();
      gp.press(g_button_index[offset]);

      is_change = true;
    }
    offset++;
    if (value > 0) {
//      Serial.printf("DEBUG %d: count = %d change = %d ",encoder_id,knob.getCount(encoder_id),value);
//      Serial.println();
      gp.press(g_button_index[offset]);
      is_change = true;
    }
    offset++;

    if (magnitude > 8) magnitude = 8;

    // Set binary encode value in 3 buttons
    for (int index = 0; index <3; index++) {
      if (magnitude && ((magnitude-1) & (0x1 << index)) ) {
      	gp.press(g_button_index[offset]);
      } else {
	      gp.release(g_button_index[offset]);
      }
      offset++;
    }
  }
  return(is_change);
}


// Detects if a mode pin has changed state
// Retunrs the the mode number whose button was pressed/released
// Negative value is a release
// positibve value is a press
// 0 is no change
int get_mode_pin_change() {
  int state = 0;
  int change = 0;
  for (int mode=0; mode < NUM_MODE_PINS; mode++) {
    // create a onehot byte showing which mode pins are pressed
    if (digitalRead(mode_button_pins[mode]) == LOW) {
      state |= (0x1 << mode);
    }
  }

  // If the mode pins pressed this cycle are different than last
  if (state != g_last_mode_pin_state) {

    // one hot vector showing which mode pins changed
    char state_change = state ^ g_last_mode_pin_state;

    // Get the number of the mode pin that changed
    int count = 1;
    while (state_change) {
      if (state_change & 0x1) {
      	change = count;
      }
      count++;
      state_change = state_change >> 1;
    }

    // set mode pin number to negative if it was released instead of pressed
    if ((state & (0x1 << (change - 1))) == 0) {
      change *= -1;
    }
    
    // Update the last state seen
    g_last_mode_pin_state = state;

  }
  
  return(change);
    
}

boolean release_buttons(BleGamepad &gp) {
  boolean found_pressed_button = false;
  for (int button = 0; button < NUM_BUTTONS; button++) {
    // Only release buttons that aren't the encoder value buttons
//    Serial.printf("%04x %04x",RELEASE_MASK,(1 << button));
//    Serial.println();
    if ( (RELEASE_MASK & (1 << button) ) && gp.isPressed(g_button_index[button])) { 
      gp.release(g_button_index[button]);
      found_pressed_button = true;
    }
  }
  if (found_pressed_button) { // if buttons pressed then axes should be reset as well
    gp.setX(0);
    gp.setY(0);
    gp.setZ(0);
    gp.setRX(0);
    print_debug(gp);
  }
  return(found_pressed_button);
}