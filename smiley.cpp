#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

long last_measure = 0;

// OLED
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display


void smiley(void) {

  u8g2.clearBuffer();

  u8g2.drawVLine(59, 20, 10); //left eye 

  if (millis()>=last_measure+2000){
    last_measure = millis();
    u8g2.drawHLine(69, 25, 10); //right eye blink
  }
  else {
    u8g2.drawVLine(69, 20, 10); //right eye
  }
  
  u8g2.drawHLine(44, 45, 40); //lower lip
  u8g2.drawVLine(44, 40, 5);  //left cheek
  u8g2.drawVLine(84, 40, 5); //right cheek

  u8g2.sendBuffer();

}

void setup(void) {

  u8g2.begin();
  u8g2.setBusClock(2000000);

  // draw borders

}

void loop(void) {
  smiley();
}