#include <ESP32Encoder.h>

class DualEncoderKnob {
  

 private:
  ESP32Encoder m_encoder1;
  ESP32Encoder m_encoder2;

  int m_pin_button;
  int m_last_count[2] = { 0, 0};

  unsigned long m_pressed_time = 0;
  
  int m_pin_A;
  int m_pin_B;
  int m_pin_a;
  int m_pin_b;
  
 public:
  DualEncoderKnob(int pin_A, int pin_B, int pin_a, int pin_b, int pin_button) {

    m_pin_button = pin_button;
    m_pin_A = pin_A;
    m_pin_B = pin_B;
    m_pin_a = pin_a;
    m_pin_b = pin_b;
    
  }

  void init() {

    pinMode(m_pin_button,INPUT_PULLUP);
    pinMode(m_pin_A,INPUT_PULLUP);
    pinMode(m_pin_B,INPUT_PULLUP);

    m_encoder1.useInternalWeakPullResistors = UP;
    
    m_encoder1.attachHalfQuad(m_pin_A,m_pin_B);
    m_encoder1.setFilter(1023);  // extra debouncing
    m_encoder1.clearCount();

    pinMode(m_pin_a,INPUT_PULLUP);
    pinMode(m_pin_b,INPUT_PULLUP);

    m_encoder2.useInternalWeakPullResistors = UP;
    
    m_encoder2.attachHalfQuad(m_pin_a,m_pin_b);
    m_encoder2.setFilter(1023);  // extra debouncing
    m_encoder2.clearCount();

    m_last_count[0] = getCount(0);
    m_last_count[1] = getCount(1);
    
  }
  

  // Specify which knob (0 for large one, 1 for small one)
  int getCount(int knob) {
    if (knob == 0) {
      return(m_encoder1.getCount());
    }
    else {
      return(m_encoder2.getCount());
    }
  }
  
  boolean buttonPressed() {
    
    return( digitalRead(m_pin_button) == LOW ); 

  }

  // Store previous pressed state
  void setPressed() {
    m_pressed_time = millis();
  }

  void resetPressed() {
    m_pressed_time = 0;
  }

  int timeSincePressed() {
    if (m_pressed_time == 0) {
      return(0);
    }
    else {
      return(millis()-m_pressed_time);
    }
  }
  
  void setLastCount(int encoder) {
    m_last_count[encoder] = getCount(encoder);
  }
  

  int getChange(int encoder) {
    int diff = getCount(encoder) - m_last_count[encoder];
    return(diff);
  }
  
}
;
