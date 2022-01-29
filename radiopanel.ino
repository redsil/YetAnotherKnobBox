#include <ESP32Encoder.h>
#include <Adafruit_NeoPixel.h>
 
#include <BleGamepad.h> 
// 1 23 456  78 901

#define LED_PIN    2
#define LED_COUNT 8

// #define ENABLE_BUTTONS
#define NUM_AXES 0
#define NUM_BUTTONS 32

#define KNOB1_I_A_PIN 15
#define KNOB1_I_B_PIN 16
#define KNOB1_O_A_PIN 17
#define KNOB1_O_B_PIN 18
#define KNOB2_I_A_PIN 19
#define KNOB2_I_B_PIN 21
#define KNOB2_O_A_PIN 22
#define KNOB2_O_B_PIN 23

#define BUTTON1_PIN 13 
#define BUTTON2_PIN 12

#define KNOB1_BUTTON_BASE 0
#define KNOB2_BUTTON_BASE 10

#define MODE_1_PIN 33
#define MODE_2_PIN 25
#define MODE_3_PIN 26
#define MODE_4_PIN 32
#define MODE_5_PIN 27
#define MODE_6_PIN 14


BleGamepad bleGamepad;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

ESP32Encoder encoder1_i;
ESP32Encoder encoder1_o;
ESP32Encoder encoder2_i;
ESP32Encoder encoder2_o;

ESP32Encoder encoders[2][2];
int last_count[2][2];
int pins[2][2][2];
int button_base[2];
int button_pins[2];
int mode_button_pins[6] = { MODE_1_PIN, MODE_2_PIN, MODE_3_PIN, MODE_4_PIN, MODE_5_PIN, MODE_6_PIN };

int button_index[] = { 
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
  BUTTON_25

};

int current_mode = -1;
int led_brightness = 10;

void setup(){
  Serial.begin(115200);
  bleGamepad = BleGamepad("FSRadio","Espressif",84);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(led_brightness); // Set BRIGHTNESS to about 1/5 (max = 255)

  encoders[0][0] = encoder1_i;
  encoders[0][1] = encoder1_o;
  encoders[1][0] = encoder2_i;
  encoders[1][1] = encoder2_o;
    
  button_base[0] = KNOB1_BUTTON_BASE;
  button_base[1] = KNOB2_BUTTON_BASE;

  pins[0][0][0] = KNOB1_I_A_PIN;
  pins[0][0][1] = KNOB1_I_B_PIN;
  pins[0][1][0] = KNOB1_O_A_PIN;
  pins[0][1][1] = KNOB1_O_B_PIN;
  pins[1][0][0] = KNOB2_I_A_PIN;
  pins[1][0][1] = KNOB2_I_B_PIN;
  pins[1][1][0] = KNOB2_O_A_PIN;
  pins[1][1][1] = KNOB2_O_B_PIN;
  button_pins[0] = BUTTON1_PIN;
  button_pins[1] = BUTTON2_PIN;

  pinMode(BUTTON1_PIN,INPUT_PULLUP);
  pinMode(BUTTON2_PIN,INPUT_PULLUP);

  for (int mode = 0; mode < 6; mode++) {
    pinMode(mode_button_pins[mode],INPUT_PULLUP);  
  }
  
  for (int knob = 0; knob < 2; knob++) {
    for (int type = 0; type < 2; type++) {
      pinMode(pins[knob][type][0],INPUT_PULLUP);
      pinMode(pins[knob][type][1],INPUT_PULLUP);
      Serial.printf("Setting Knob %d%d pins to %d %d!\n",knob,type,pins[knob][type][0],pins[knob][type][1]);
      encoders[knob][type].useInternalWeakPullResistors=UP;
      encoders[knob][type].attachHalfQuad(pins[knob][type][0],pins[knob][type][1]);
      encoders[knob][type].clearCount();
      last_count[knob][type] = encoders[knob][type].getCount();
    }
  }
   
  Serial.println("Starting BLE work!");
  bleGamepad.setAutoReport(false);

  bleGamepad.begin(NUM_BUTTONS, 0,false,
    false,
    false,
    false,
    false,
    false,
    false,
    false);  

  strip.rainbow(10);             // Flowing rainbow cycle along the whole strip
  strip.show();

}

void loop(){
  boolean update_controller = false;  // only send gamepad events if something has changed
  boolean alt_function = false;
  
  boolean mode_pressed[6];    
  for (int mode=0; mode < 6; mode++) {
    mode_pressed[mode] = false;
    if (digitalRead(mode_button_pins[mode]) == LOW) {
      alt_function = true;
      mode_pressed[mode] = true;
      Serial.printf("Mode %d selected\n",mode);
      strip.clear();
      current_mode = mode;
      int hue = 0;
      if (mode == 0 || mode == 5) {
        hue = (65535/3)*2;
      }
      else if (mode == 1 || mode == 4) {
        hue = 65535/3;
      }
      uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
      int led_offset = mode > 2 ? 2 : 0;
      strip.setPixelColor(mode+led_offset, color); // Set pixel 'c' to value 'color'
      strip.show();
    }
  }
  
  for (int knob = 0; knob < 2; knob++) {
    int gp_button = button_index[button_base[knob]];
    int button_value = digitalRead(button_pins[knob]);
    if (bleGamepad.isPressed(gp_button) && button_value == HIGH) {
      bleGamepad.release(gp_button);  
      update_controller = true;
    }
    else if (button_value == LOW && !bleGamepad.isPressed(gp_button)) {
      bleGamepad.press(gp_button);
      update_controller = true;
    }
    if (bleGamepad.isPressed(gp_button))   Serial.printf("Button %d pressed\n",gp_button);        

    
    for (int type = 0; type < 2; type++) {
      int count = encoders[knob][type].getCount();
      int direction = 0;
      int type_button_base = button_base[knob] + type*5;

      direction =  count - last_count[knob][type];
      // limit direction to +- 7 to match the limit of what we can send
      if (direction > 7) direction = 7;
      if (direction < -7) direction = -7;
      last_count[knob][type] = count;
      if (direction != 0) {
        update_controller = true;
        Serial.println("Encoder count " + String((int32_t)knob) + String((int32_t)type) +  " => " + String((int32_t)count));        
        if (abs(direction) & 0x1) {
          bleGamepad.press(button_index[type_button_base + 5]);            
        }
        if (abs(direction) &0x2) {
          bleGamepad.press(button_index[type_button_base + 4]);            
        }
        if (abs(direction) &0x4) {
          bleGamepad.press(button_index[type_button_base + 3]);            
        }
        if (direction > 0) {
          bleGamepad.press(button_index[type_button_base + 2]);            
          strip.setPixelColor(4, 65535/3); // Set pixel 'c' to value 'color'
        }
        else if (direction < 0) {
          bleGamepad.press(button_index[type_button_base + 1]);            
          strip.setPixelColor(3, 65535/3); // Set pixel 'c' to value 'color'
        }
      } 
      if (bleGamepad.isPressed(button_index[type_button_base+1]) && !alt_function)   Serial.printf("Button %d pressed %d%d%d\n",button_index[type_button_base+1],
          bleGamepad.isPressed(button_index[type_button_base+3]),bleGamepad.isPressed(button_index[type_button_base+4]),bleGamepad.isPressed(button_index[type_button_base+5]));        
      if (bleGamepad.isPressed(button_index[type_button_base+2]) && !alt_function)   Serial.printf("Button %d pressed %d%d%d\n",button_index[type_button_base+2],        
          bleGamepad.isPressed(button_index[type_button_base+3]),bleGamepad.isPressed(button_index[type_button_base+4]),bleGamepad.isPressed(button_index[type_button_base+5]));        
    }
  }

  // if button is pressed while knob is turrned, execute alternate function, otherwise send button presses to gamepad
  if (alt_function) {
    // Change brightness
    if  (bleGamepad.isPressed(button_index[button_base[1]] + 2)) {
      if (led_brightness < 245) led_brightness += 10;
      strip.setBrightness(led_brightness); // Set BRIGHTNESS to about 1/5 (max = 255)
      strip.show();                
    } 
    else if (bleGamepad.isPressed(button_index[button_base[1]] + 1)) {
      if (led_brightness > 10) led_brightness -= 10;
      strip.setBrightness(led_brightness); // Set BRIGHTNESS to about 1/5 (max = 255)
      strip.show();                
    }
    delay(50);
    strip.setPixelColor(3, 0); // Set pixel 'c' to value 'color'
    strip.setPixelColor(4, 0); // Set pixel 'c' to value 'color'
  } 

  // Send button presses for knob turns
  if (bleGamepad.isConnected() && update_controller) bleGamepad.sendReport();
  strip.show();
  delay(50);

  // release buttons after one press  
  for (int knob = 0; knob < 2; knob++) {
    for (int type = 0; type < 2; type++) {
      bleGamepad.release(button_index[button_base[knob]] + type*5 + 1);                  
      bleGamepad.release(button_index[button_base[knob]] + type*5 + 2);                   
      bleGamepad.release(button_index[button_base[knob]] + type*5 + 3);                   
      bleGamepad.release(button_index[button_base[knob]] + type*5 + 4);                   
      bleGamepad.release(button_index[button_base[knob]] + type*5 + 5);                   
    }
  }
  if (update_controller) {
    if (bleGamepad.isConnected()) bleGamepad.sendReport();
    strip.setPixelColor(3, 0); // Set pixel 'c' to value 'color'
    strip.setPixelColor(4, 0); // Set pixel 'c' to value 'color'
    strip.show();
    delay(50);
  }
}   
