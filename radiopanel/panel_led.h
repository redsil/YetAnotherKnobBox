#include <Adafruit_NeoPixel.h>

class PanelLed {

 private:
  const int NUM_PIXELS = 8;
  Adafruit_NeoPixel m_strip;
  boolean right_on = false;
  boolean left_on = false;
  
 public:
  PanelLed(int data_pin, int brightness) {
    //    m_strip = new Adafruit_NeoPixel(NUM_PIXELS, data_pin, NEO_GRB + NEO_KHZ800);
    m_strip.updateLength(NUM_PIXELS);
    m_strip.updateType(NEO_GRB + NEO_KHZ800);
    m_strip.setPin(data_pin);

    m_strip.setBrightness(brightness); // Set BRIGHTNESS to about 1/5 (max = 255)
    rainbow(50);
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


  void rainbow(int a) {
    m_strip.rainbow(a);
  }

  void setRight() {
    int hue =  32 << 8; // Yellow
    uint32_t color = m_strip.gamma32(m_strip.ColorHSV(hue)); // hue . RGB
    m_strip.setPixelColor(4, color); // Set pixel 'c' to value 'color'
    right_on = true;
  }
  void setLeft() {
    int hue =  32 << 8; // Yellow
    uint32_t color = m_strip.gamma32(m_strip.ColorHSV(hue)); // hue . RGB
    m_strip.setPixelColor(3, color); // Set pixel 'c' to value 'color'
    left_on = true;
  }
  void clearRight() {
    if (right_on) {
      m_strip.setPixelColor(4, 0,0,0); // Set pixel 'c' to value 'color'
      right_on = false;
    }
    
  }
  void clearLeft() {
    if (left_on) {
      m_strip.setPixelColor(3, 0,0,0); // Set pixel 'c' to value 'color'
      left_on = false;
    }
  }

  boolean leftOn() {
    return left_on;
  }
  boolean rightOn() {
    return right_on;
  }
  
  void show_mode(int mode) {
    int hue = 0; // RED
    m_strip.clear();
    if (mode == 0 || mode == 5) {
      hue = 160 << 8;
      //           hue = 0x(65535/3)*2; // GREEN
    }
    else if (mode == 1 || mode == 4) {
      hue = 90 << 8;
      //           hue = 65535/3; // BLUE
    }
    uint32_t color = m_strip.gamma32(m_strip.ColorHSV(hue)); // hue . RGB
    int led_offset = (mode > 2) ? 2 : 0;
    m_strip.setPixelColor(mode+led_offset, color); // Set pixel 'c' to value 'color'
    show();
    
  }
  
  void show() {
    m_strip.show();
  }
  
}
;
