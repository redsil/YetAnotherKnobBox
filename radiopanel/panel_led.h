#include <Adafruit_NeoPixel.h>

class PanelLed {

 private:
  const int NUM_PIXELS = 8;
  Adafruit_NeoPixel m_strip;
  boolean right_on = false;
  boolean left_on = false;
  unsigned long m_timer_end = 0;
  const char FIRST_TIMER_PIXEL = 2;
  const char LAST_TIMER_PIXEL = 5;
  int m_timer_setting = 0;
  const int MAX_TIMER_VALUE = 209;
  
 public:
  PanelLed(int data_pin, int brightness) {
    //    m_strip = new Adafruit_NeoPixel(NUM_PIXELS, data_pin, NEO_GRB + NEO_KHZ800);
    m_strip.updateLength(NUM_PIXELS);
    m_strip.updateType(NEO_GRB + NEO_KHZ800);
    m_strip.setPin(data_pin);

    m_strip.setBrightness(brightness); // Set BRIGHTNESS to about 1/5 (max = 255)
    m_strip.clear();
  }


  void begin() {
    m_strip.begin();
  }
  
  void setBrightness(int brightness) {
    m_strip.setBrightness(brightness); // Set BRIGHTNESS to about 1/5 (max = 255)
  }

  int getBrightness() {
    return(m_strip.getBrightness()); 
  }


  // Inner 4 leds used to indicate countdown
  // 3 right most leds are 5 seconds each.  Will change color each second and finish on red
  // left most led indicates which group of 15 seconds we are on
  //    blue - 45-60
  //    green - 30-45
  //    red   - 15-30
  //    off   - 0 -15
  

  void setTimer(int seconds) {
    m_timer_setting = seconds + 1; 
    if (m_timer_setting > MAX_TIMER_VALUE) m_timer_setting = MAX_TIMER_VALUE;
    if (m_timer_setting < 0) m_timer_setting = 0;
  }

  // Display representation of current timer setting
  void showTimer() {
    // First two leds are half minutes
    // remaining 6 are 5 seconds each..  max time 2:00
    char halfminutes = m_timer_setting/30;
    char hexaseconds = (m_timer_setting - (halfminutes * 30))/5;

    m_strip.clear();

    int color = m_strip.gamma32(m_strip.ColorHSV(160 << 8)); // blue
    
    if (halfminutes > 0) m_strip.setPixelColor(0, color); // Set pixel 'c' to value 'color'
    if (halfminutes > 1) m_strip.setPixelColor(1, color); // Set pixel 'c' to value 'color'
    if (halfminutes > 2) m_strip.setPixelColor(2, color); // Set pixel 'c' to value 'color'

    color = m_strip.gamma32(m_strip.ColorHSV(0)); // red
    if (halfminutes > 3) m_strip.setPixelColor(0, color); // Set pixel 'c' to value 'color'
    if (halfminutes > 4) m_strip.setPixelColor(1, color); // Set pixel 'c' to value 'color'
    if (halfminutes > 5) m_strip.setPixelColor(2, color); // Set pixel 'c' to value 'color'

    color = m_strip.gamma32(m_strip.ColorHSV(90 << 8)); // green
    if (hexaseconds > 0) m_strip.setPixelColor(3, color); // Set pixel 'c' to value 'color'
    if (hexaseconds > 1) m_strip.setPixelColor(4, color); // Set pixel 'c' to value 'color'
    if (hexaseconds > 2) m_strip.setPixelColor(5, color); // Set pixel 'c' to value 'color'
    if (hexaseconds > 3) m_strip.setPixelColor(6, color); // Set pixel 'c' to value 'color'
    if (hexaseconds > 4) m_strip.setPixelColor(7, color); // Set pixel 'c' to value 'color'

    show();
  }

  void startTimer() {
    // TODO - detect overflow?
    m_timer_end = millis() + 1000 * m_timer_setting;

    // blank counter pixels
    for (int i = FIRST_TIMER_PIXEL; i <= LAST_TIMER_PIXEL; i++ ) {
      m_strip.setPixelColor(i, 0,0,0); // Blank timer pixels
    }
  }

  // returns true when running
  boolean runTimer() {
    boolean running = (m_timer_end > 0);
    unsigned long cur_time = millis();
    int remaining = 0;
    if (running) { // time is unsigned, so can't subtract to find a negative value
      if (m_timer_end >= cur_time) {
	remaining = (m_timer_end - cur_time)/1000;
      }
      else {
	remaining = -1 * int(cur_time - m_timer_end)/1000;  
      }

      if (remaining == 0) {
	  for (int i = FIRST_TIMER_PIXEL; i <= LAST_TIMER_PIXEL; i++) {
	    m_strip.setPixelColor(i,255,255,255); 
	  }
      }
      else if (remaining > 0) {
	int remaining_minus_one = remaining - 1;
	int quarter = ((remaining)/15) % 4;
	int fiver = 2 - ((remaining)/5) % 3;
	
	// init these yellow on roll over 
	if (fiver == 0) {
	  int color = m_strip.gamma32(m_strip.ColorHSV(32 << 8)); // hue . RGB
	  for (int i = FIRST_TIMER_PIXEL + 1; i <= LAST_TIMER_PIXEL; i++) {
	    m_strip.setPixelColor(i,color); 
	  }
	}
	
	// If last four seconds, set all 4 to red and extinguish one at a time at once for visibility
	if (remaining == 4) {
	  // Turn all 4 to red for the final countdown
	  int color = m_strip.gamma32(m_strip.ColorHSV(9)); // hue . RGB
	  for (int i = FIRST_TIMER_PIXEL; i <= LAST_TIMER_PIXEL; i++) {
	    m_strip.setPixelColor(i,color); 
	  }
	}
	else if (remaining < 4) {
	    // final countdown, turn one off at a time
	  m_strip.setPixelColor(FIRST_TIMER_PIXEL + (3 - ((remaining) % 4)), 0,0,0); // Turn off
	}
	else {
	  //  set quarter LED
	  int hue =  ( (255/4) * (4-quarter) ) << 8; 
	  int color = m_strip.gamma32(m_strip.ColorHSV(hue)); // hue . RGB
	  m_strip.setPixelColor(FIRST_TIMER_PIXEL, color); // Set pixel 'c' to value 'color'
	  
	  
	  // set fiver color
	  int color_index = 4 - (remaining_minus_one % 5); // 
	  
	  hue =  (  (255/5) *  color_index  ) << 8; 
	  color = m_strip.gamma32(m_strip.ColorHSV(hue)); // hue . RGB
	  m_strip.setPixelColor((FIRST_TIMER_PIXEL+fiver+1), color); // Set pixel 'c' to value 'color'
	}
      }
      else if (remaining < 0) {
	m_timer_end = 0;
	for (int i = FIRST_TIMER_PIXEL; i <= LAST_TIMER_PIXEL; i++ ) {
	  m_strip.setPixelColor(i, 0,0,0); // Set pixel 'c' to value 'color'
	}
      }
      show();
    }
    return(running);
  }
  
  void rainbow(int a) {
    m_strip.rainbow(a);
  }

  void setRight() {
    int hue =  32 << 8; // Yellow
    uint32_t color = m_strip.gamma32(m_strip.ColorHSV(hue)); // hue . RGB
    m_strip.setPixelColor(NUM_PIXELS-2, color); // Set pixel 'c' to value 'color'
    right_on = true;
  }
  void setLeft() {
    int hue =  32 << 8; // Yellow
    uint32_t color = m_strip.gamma32(m_strip.ColorHSV(hue)); // hue . RGB
    m_strip.setPixelColor(1, color); // Set pixel 'c' to value 'color'
    left_on = true;
  }
  void clearRight() {
    if (right_on) {
      m_strip.setPixelColor(NUM_PIXELS-2, 0,0,0); // Set pixel 'c' to value 'color'
      right_on = false;
    }
    
  }
  void clearLeft() {
    if (left_on) {
      m_strip.setPixelColor(1, 0,0,0); // Set pixel 'c' to value 'color'
      left_on = false;
    }
  }

  boolean leftOn() {
    return left_on;
  }
  boolean rightOn() {
    return right_on;
  }
  
  void clear() {
    m_strip.clear();
  }

  void show_mode(int mode) {
    int hue = 0; // RED

    // Clear existing mode light
    m_strip.setPixelColor(0, 0,0,0); // Set pixel 'c' to value 'color'
    m_strip.setPixelColor(NUM_PIXELS-1, 0,0,0); // Set pixel 'c' to value 'color'

    if (mode == 0 || mode == 5) {
      hue = 160 << 8;
      //           hue = 0x(65535/3)*2; // GREEN
    }
    else if (mode == 1 || mode == 4) {
      hue = 90 << 8;
      //           hue = 65535/3; // BLUE
    }
    uint32_t color = m_strip.gamma32(m_strip.ColorHSV(hue)); // hue . RGB
    int led_offset = (mode > 2) ? (NUM_PIXELS-1) : 0;
    m_strip.setPixelColor(led_offset, color); // Set pixel 'c' to value 'color'
    show();
    
  }

  int getMaxTimerValue() {
    return MAX_TIMER_VALUE;
  }
  
  void show() {
    m_strip.show();
  }
  
}
;
