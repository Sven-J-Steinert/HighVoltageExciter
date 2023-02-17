#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define LEDC_SLOW_CLK_RTC8M  2
#define LEDC_DEFAULT_CLK     0
#define LEDC_CHANNEL_0     0
#define LED_PIN            11

long cycle_periods = 30000;

float calc_frequency(){
  float f = (1/(25*pow(10,-9)*(cycle_periods)));
  return f;
}

float frequency = calc_frequency();
float frequency_old = 0;

int switchBitResolution(void){
  /**
   * Switch resolution depending on max frequencies
   * 12 bit    9.765625 kHz
   * 11 bit   19.53125 kHz
   * 10 bit   39.0625 kHz
   * 9 bit    78.125 kHz
   * 8 bit   156.25 kHz
   * 7 bit   312.5 kHz
   * 6 bit   625 kHz
   * 5 bit     1.250 MHz
   * 4 bit     2.5 MHz
   * 3 bit     5 MHz
   * 2 bit    10 MHz
   * 1 bit    20 MHz
   */
  int bit_res = 0;

  if      (frequency > 10000000){ bit_res = 1; }
  else if (frequency > 5000000) { bit_res = 2; }
  else if (frequency > 2500000) { bit_res = 3; }
  else if (frequency > 1250000) { bit_res = 4; }
  else if (frequency >  625000) { bit_res = 5; }
  else if (frequency >  312500) { bit_res = 6; }
  else if (frequency >  156250) { bit_res = 7; }
  else if (frequency >   78125) { bit_res = 8; }
  else if (frequency >   39062.5) { bit_res = 9; }
  else if (frequency >   19531.25) { bit_res = 10; }
  else if (frequency >    9765.625) { bit_res = 11; }
  else if (frequency >    4882.8125) { bit_res = 12; }
  else if (frequency >    2441.40625) { bit_res = 13; }
  else if (frequency >    1220.703125) { bit_res = 14; } // highest bitrate for LEDC_USE_APB_CLK
  
  else if (frequency >     610.3515625) { bit_res = 0; }
  else if (frequency >       0) { bit_res = 16; }

  return bit_res;
}

int LEDC_TIMER_BIT = switchBitResolution();
float duty_percent = 50;
long duty_periods = int((duty_percent/100)*(pow(2,LEDC_TIMER_BIT)-1));
long duty_periods_old = 0;


// OLED
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t TIMER_BIT) {
  // calculate duty, 4095 from 2 ^ 12 - 1
  uint32_t valueMax = pow(2,TIMER_BIT)-1;
  int val = pow(2,TIMER_BIT) -1;
  uint32_t duty = (val / valueMax) * min(value, valueMax);

  ledcWrite(channel, duty);
}

void drawFrame(void)
{
  u8g2.setFontMode(1);	// Transparent
  u8g2.setFontDirection(0);
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(18,48,"CYCLE");
  u8g2.drawStr(85,48,"DUTY");
  u8g2.drawFrame(0, 0, 127, 38); // Top Box
  u8g2.drawHLine(0, 44, 16);
  u8g2.drawHLine(48, 44, 35);
  u8g2.drawHLine(110, 44, 17);
  //u8g2.drawHLine(0, 63, 127); // Bottom
  //u8g2.drawFrame(0, 37, 127, 27); // Bottom Box
  u8g2.drawVLine(0, 44, 20); // Left
  u8g2.drawVLine(64, 44, 20); // Bottom Box Divider
  u8g2.drawVLine(127, 44, 20); // Right
}

void drawValue(u8g2_uint_t x, u8g2_uint_t y, uint num)
{
  char str[9];
  itoa(num, str, 10 );
  int x_len = u8g2.getStrWidth(str);
  u8g2.drawStr(x-x_len,y,str);
}

void drawValueFloat(u8g2_uint_t x, u8g2_uint_t y, float num)
{
  char str[8];
  
  if (num >= 1000000){
    dtostrf(num/1000000, 6, 3, str);
    int x_len = u8g2.getStrWidth(str);
    u8g2.drawStr(x-x_len,y,str);
    u8g2.drawStr(x+7,y,"MHz");
  }
  else if (num < 1000){
    dtostrf(num, 6, 1, str);
    int x_len = u8g2.getStrWidth(str);
    u8g2.drawStr(x-x_len,y,str);
    u8g2.drawStr(x+7,y,"Hz");
  }
  else {
    dtostrf(num/1000, 6, 3, str);
    int x_len = u8g2.getStrWidth(str);
    u8g2.drawStr(x-x_len,y,str);
    u8g2.drawStr(x+7,y,"kHz");
  }

}

void refreshDisplayValues(void)
{
  

  //u8g2.setFont(u8g2_font_crox2h_tf);
  //u8g2.drawStr(1,62,"10274");

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_10x20_tf);
  drawValueFloat(80,24,frequency);

  u8g2.setFont(u8g2_font_6x10_tf);

  drawValue(70,48,LEDC_TIMER_BIT);

  drawValue(56,61,cycle_periods);
  
  drawValue(85,61,duty_percent);
  u8g2.drawStr(88,61,"%");
  drawValue(128,61,duty_periods);

  //u8g2.drawHLine(0, 63, 127); 
  u8g2.updateDisplayArea(1, 1, 14, 2); // frequency
  u8g2.updateDisplayArea(7, 5, 2, 2); // bit
  //u8g2.updateDisplayArea(1, 6, 6, 2); // cycle_periods
  //u8g2.updateDisplayArea(9, 6, 7, 2); // duty_percent
  u8g2.updateDisplayArea(1, 6, 15, 2); // bottom
  //u8g2.updateDisplay();


}

void setup(void) {
  ledcSetup(LEDC_CHANNEL_0, frequency, LEDC_TIMER_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);

  u8g2.begin();
  u8g2.setBusClock(2000000);
  
  // draw borders
  u8g2.clearBuffer();
  drawFrame();
  u8g2.sendBuffer();
}

void loop(void) {
  frequency = calc_frequency();
  LEDC_TIMER_BIT = switchBitResolution();

  if (frequency < 1220.703125){
    const int freq = 5000;
    const int ledChannel = 1;
    const int resolution = 8;
    ledcSetup(ledChannel, freq, resolution);
    ledcAttachPin(12, ledChannel);
    ledcWrite(ledChannel, 125);
  }

  else {
    ledcChangeFrequency(LEDC_CHANNEL_0, frequency,LEDC_TIMER_BIT);
  }
  
  
  if (duty_periods > (pow(2,LEDC_TIMER_BIT))){
    duty_periods = (pow(2,LEDC_TIMER_BIT));
  }
  duty_percent = 100*(duty_periods/(pow(2,LEDC_TIMER_BIT)));
  
  ledcAnalogWrite(LEDC_CHANNEL_0, duty_periods,LEDC_TIMER_BIT);
  
  refreshDisplayValues();

  cycle_periods+=100;
  //duty_periods+= int(0.1*(pow(2,LEDC_TIMER_BIT)));
  //delay(1000);
  //duty_periods=3;
}