class EncoderKnob {
  
 private:

  int8_t m_pin_button;
  int8_t m_pin_A;
  int8_t m_pin_B;

  int m_last_count = 0;
  int m_count = 0;

  unsigned long m_pressed_time = 0;
  
  int8_t TRANS[16] = {0, -1, 1, 14, 1, 0, 14, -1, -1, 14, 0, 1, 14, 1, -1, 0};
  uint8_t lrmem;
  int lrsum;
  volatile bool lock = false;

 public:
    // Uses either the ESP32encoder library or a custom one.  ESP32encoder library is not debounced and only good for optical encoders
  EncoderKnob(int pin_A, int pin_B, int pin_button) {

    m_pin_button = pin_button;
    m_pin_A = pin_A;
    m_pin_B = pin_B;

    lrmem = 3;
    lrsum = 0;
  }

  void init() {
    // Set pin modes
    if (m_pin_button > 0) {
      pinMode(m_pin_button,INPUT_PULLUP);
    }
    pinMode(m_pin_A,INPUT_PULLUP);
    pinMode(m_pin_B,INPUT_PULLUP);
    Serial.println("Setup completed");
  }

  int8_t get_pinButton() {
    return(m_pin_button);
  }

  int8_t get_pinA() {
    return(m_pin_A);
  }

  int8_t get_pinB() {
    return(m_pin_B);
  }

  //  Call this method from inside the interrupt handler that is triggered by either A or B pins
  void process()
  { 
    if (!lock) {
      lock = true;
      // Read BOTH pin states to deterimine validity of rotation (ie not just switch bounce)
      int8_t r = digitalRead(m_pin_A);
      int8_t l = digitalRead(m_pin_B);

      // Move previous value 2 bits to the left and add in our new values
      lrmem = ((lrmem & 0x03) << 2) + 2 * l + r;

      // Convert the bit pattern to a movement indicator (14 = impossible, ie switch bounce)
      lrsum += TRANS[lrmem];

      /* encoder in the neutral state - clockwise rotation*/
      // use 2/-2 for a 2 pulse per detent encoder, 4/-4 for a 4 pulse per detent encoder (half vs full quad?)
      if (lrsum == 2)
      {
          lrsum = 0;
          m_count += 1;
      }

      /* encoder in the neutral state - anti-clockwise rotation*/
      else if (lrsum == -2)
      {
          lrsum = 0;
          m_count -= 1;
      }
      else {
        /* encoder not in the neutral (detent) state */
        if (lrsum % 4 == 0)
        {
          // An impossible rotation has been detected - ignore the movement
          lrsum = 0;
        } 
      }
      lock = false;
    }
  }

  // Specify which knob (0 for large one, 1 for small one)
  int getCount() {
    return(m_count);
  }
  
  bool buttonPressed() {
    if (m_pin_button) {
      return( digitalRead(m_pin_button) == LOW ); 
    }
    else {
      return(false);
    }

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
  
  void setLastCount() {
    m_last_count = m_count;
  }
  
  int getValue() {
    return(getCount());
  }

  int getChange() {
    int diff = getCount() - m_last_count;
    setLastCount();
    return(diff);
  }


};

class DualEncoderKnob {
  

 private:
  EncoderKnob m_encoder1;
  EncoderKnob m_encoder2;

  int m_pin_button;
  
  unsigned long m_pressed_time = 0;
  
  int m_pin_A;
  int m_pin_B;
  int m_pin_a;
  int m_pin_b;
  
 public:
  DualEncoderKnob(int pin_A, int pin_B, int pin_a, int pin_b, int pin_button) : m_encoder1(pin_A,pin_B,0),m_encoder2(pin_a,pin_b,pin_button) {

    m_pin_button = pin_button;
    m_pin_A = pin_A;
    m_pin_B = pin_B;
    m_pin_a = pin_a;
    m_pin_b = pin_b;
    
  }

  int8_t get_pinA() {
    return(m_pin_A);
  }
  int8_t get_pinB() {
    return(m_pin_B);
  }
  int8_t get_pina() {
    return(m_pin_a);
  }
  int8_t get_pinb() {
    return(m_pin_b);
  }
  int8_t get_pin_button() {
    return(m_pin_button);
  }

  void init() {
    m_encoder1.init();
    m_encoder2.init();
  }
  
  void process1() {
    m_encoder1.process();
  }
  void process2() {
    m_encoder2.process();
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
    return( m_encoder2.buttonPressed());
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
  
  int getChange(int encoder) {
    if (encoder == 0) return(m_encoder1.getChange());
    if (encoder == 1) return(m_encoder2.getChange());
  }
}
;
