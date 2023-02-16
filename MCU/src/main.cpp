#include <Arduino.h>

//#include <stdio.h>
//#include "driver/ledc.h"
//#include "esp_err.h"

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 12 bit precission for LEDC timer
//#define LEDC_TIMER_BIT  8

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     50000

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN            11

int LEDC_TIMER_BIT = 3;
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

int brightness = int(0.5*(pow(2,LEDC_TIMER_BIT)-1));    // how bright the LED is
int fadeAmount = 1;    // how many points to fade the LED by

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = pow(2,LEDC_TIMER_BIT)-1) {
  // calculate duty, 4095 from 2 ^ 12 - 1
  int val = pow(2,LEDC_TIMER_BIT) -1;
  uint32_t duty = (val / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

long i = 1000000;

void setup() {
  // Setup timer and attach timer to a led pin
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);
}

void loop() {
  ledcSetup(LEDC_CHANNEL_0, i, LEDC_TIMER_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);
  // set the brightness on LEDC channel 0
  ledcAnalogWrite(LEDC_CHANNEL_0, brightness);

  // change the brightness for next time through the loop:
  //brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  //if (brightness <= 0 || brightness >= pow(2,LEDC_TIMER_BIT)-1) {
  //  fadeAmount = -fadeAmount;
  //}
  // wait for 30 milliseconds to see the dimming effect
  delay(1000);
  i = i + 10000;
}
