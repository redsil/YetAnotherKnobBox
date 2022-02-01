#include "panel_led.h"
#include "knob.h"


enum MODE { MODE_NONE, MODE_1, MODE_2, MODE_3, MODE_4, MODE_5, MODE_6, BRIGHTNESS_ADJUST, SET_TIMER };


#include <BleGamepad.h> 

#define LED_PIN    2
#define LED_COUNT 8


// #define ENABLE_BUTTONS
#define NUM_AXES 0
#define NUM_BUTTONS 32

#define NUM_MODE_PINS 6
#define NUM_BUTTONS_PER_KNOB 11
#define MODE_BUTTON_OFFSET NUM_BUTTONS_PER_KNOB * 2

BleGamepad bleGamepad("KnobPanel","Rpierce",90);

DualEncoderKnob knob1(15,16,17,18,13);
DualEncoderKnob knob2(19,21,22,23,12);

PanelLed strip(LED_PIN,10);

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
  BUTTON_26,
  BUTTON_27,
  BUTTON_28,
  BUTTON_29,
  BUTTON_31
};

// which buttons should be released during the release cycle
boolean g_release_mask[NUM_BUTTONS];

MODE current_mode = MODE_NONE;
MODE prev_mode = MODE_NONE;
MODE long_mode = MODE_NONE;

int g_timer_value = 0;


void setup(){
  Serial.begin(115200);

  for (int mode = 0; mode < 6; mode++) {
    pinMode(mode_button_pins[mode],INPUT_PULLUP);  
  }
  
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

  
  knob1.init();
  knob2.init();
  
  strip.begin();
  //  strip.rainbow(10);             // Flowing rainbow cycle along the whole strip
  strip.show();

}

int button_hold_count = 0;


// Each time through the loop we alternate make controller button presses
// with releasing buttons.  This gives pulses for each turn of the encoder
boolean release_cycle = true;

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
	g_release_mask[button_offset] = true;
	
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
    
    boolean encoder_movement = true;
    if (process_encoder(knob1,0,bleGamepad)) encoder_movement = true;
    if (process_encoder(knob2,NUM_BUTTONS_PER_KNOB,bleGamepad)) encoder_movement = true;

    
    update_controller = encoder_movement || update_controller;
  
    if (current_mode >= MODE_1 && current_mode <= MODE_6) {
      if (update_controller) {
	print_debug(bleGamepad);
	if (bleGamepad.isConnected()) bleGamepad.sendReport();
      }
      if (bleGamepad.isPressed(g_button_index[1]) || bleGamepad.isPressed(g_button_index[6]) ||
	  bleGamepad.isPressed(g_button_index[12]) || bleGamepad.isPressed(g_button_index[17]) ) {
	strip.setLeft();
	strip.show();
      }
      if (bleGamepad.isPressed(g_button_index[2]) || bleGamepad.isPressed(g_button_index[7]) ||
	  bleGamepad.isPressed(g_button_index[13]) || bleGamepad.isPressed(g_button_index[18])) {
	strip.setRight();
	strip.show();
      }
    }
    else if (current_mode == BRIGHTNESS_ADJUST) {
      strip.rainbow(100 << 8);
      if (bleGamepad.isPressed(g_button_index[12])) {
	if (strip.getBrightness() > 10) strip.setBrightness(strip.getBrightness() - 10);
      }
      if (bleGamepad.isPressed(g_button_index[13])) {
	if (strip.getBrightness() < 245) strip.setBrightness(strip.getBrightness() + 10);
      }
      strip.show();
    }
    else if (current_mode == SET_TIMER) {
      if (bleGamepad.isPressed(g_button_index[12])) {
	if (g_timer_value >= 5) g_timer_value -= 5;
      }
      if (bleGamepad.isPressed(g_button_index[13])) {
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
  delay(50);
}

void print_debug(BleGamepad &gp) {
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

  if (knob.buttonPressed()) {
    if (!gp.isPressed(g_button_index[gamepad_button_offset])) {
      gp.press(g_button_index[gamepad_button_offset]);
      g_release_mask[gamepad_button_offset] = false;
      is_change = true;
    }
  }
  else {
    if (gp.isPressed(g_button_index[gamepad_button_offset])) {
      g_release_mask[gamepad_button_offset] = true;
      is_change = true;
    }
  }    

  int offset = gamepad_button_offset+1;
  for (int knob_id = 0; knob_id < 2; knob_id++) {
    
    int value = knob.getChange(knob_id);
    int magnitude = abs(value);
    if (value != 0) {
      knob.setLastCount(knob_id);
    }
    
    if (value < 0) {
      Serial.printf("DEBUG %d: count = %d change = %d ",knob_id,knob.getCount(knob_id),value);
      Serial.println();
      gp.press(g_button_index[offset]);
      g_release_mask[offset] = true;
      is_change = true;
    }
    offset++;
    if (value > 0) {
      Serial.printf("DEBUG %d: count = %d change = %d ",knob_id,knob.getCount(knob_id),value);
      Serial.println();
      gp.press(g_button_index[offset]);
      g_release_mask[offset] = true;
      is_change = true;
    }
    offset++;

    if (magnitude > 8) magnitude = 8;
      // Big Endian
    if (magnitude && ((magnitude-1) & 0x4)) gp.press(g_button_index[offset]);
    g_release_mask[offset] = true;
    offset++;
    if (magnitude && ((magnitude-1) & 0x2)) gp.press(g_button_index[offset]);
    g_release_mask[offset] = true;
    offset++;
    if (magnitude && ((magnitude-1) & 0x1)) gp.press(g_button_index[offset]);
    g_release_mask[offset] = true;
    offset++;
    
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
    if (gp.isPressed(g_button_index[button]) && g_release_mask[button]) {
      gp.release(g_button_index[button]);
      found_pressed_button = true;
    }
  }
  return(found_pressed_button);
}