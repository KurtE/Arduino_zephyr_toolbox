//-----------------------------------------------------------------------------
// Copy of the Arduino Zephyr analog code - to map from Pin Number to zephyr
// object and channel
//-----------------------------------------------------------------------------
#include "wiring_analog_extended.h"
#define PWM_PIN 2
void setup() {
  Serial.begin(115200);
  pinMode(3, OUTPUT);
  // Start of the PWM...
  analogWriteResolution(10);
  XanalogWrite(PWM_PIN, 64);
  delay(100);

  // Quick and dirty to set the period and pulse in nano seconds
  //size_t idx = pwm_pin_index(PWM_PIN);
  //pwm_set(arduino_pwm[idx].dev, arduino_pwm[idx].channel, 1000, 500, PWM_POLARITY_NORMAL);
  XanalogWriteFrequency(PWM_PIN, 1000000);
}

uint16_t duty = 0;
void loop() {
  delay(10);
  digitalWrite(3, !digitalRead(3));
  XanalogWrite(PWM_PIN, duty);
  duty += 64;
  if (duty == 1024) duty = 1023;
  else if (duty > 1024) duty = 0;
}
