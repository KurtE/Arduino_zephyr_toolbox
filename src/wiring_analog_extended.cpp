/*
 * Copyright (c) Arduino s.r.l. and/or its affiliated companies
 * Copyright (c) 2022 TOKITA Hiroshi
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * This is an extended version that 
 */

#include <Arduino.h>
#include <zephyrPinctrl.h>
#include "wiring_private.h"
#define PWM_DT_SPEC(n, p, i) PWM_DT_SPEC_GET_BY_IDX(n, i),
#define PWM_PINS(n, p, i) \
  DIGITAL_PIN_GPIOS_FIND_PIN(DT_REG_ADDR(DT_PHANDLE_BY_IDX(DT_PATH(zephyr_user), p, i)), \
                             DT_PHA_BY_IDX(DT_PATH(zephyr_user), p, i, pin)),


const struct pwm_dt_spec arduino_pwm[] = {
  DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), pwms, PWM_DT_SPEC)
};

/* pwm-pins node provides a mapping digital pin numbers to pwm channels */
const pin_size_t arduino_pwm_pins[] = {
  DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), pwm_pin_gpios, PWM_PINS)
};


#if 0
typedef struct {
  uint32_t period;
  uint32_t duty;
} pwm_period_duty_t;

pwm_period_duty_t pwm_period_duty[sizeof(arduino_pwm) / sizeof(arduino_pwm[0])] = { { 0, 0 } };
#endif
// To enable change the frequency on the fly we can not rely fully on the
// period value in the pwm_dt_spec, and that structure is read-only so we need
// an array to save updated periods into.
uint32_t arduino_pwm_periods[sizeof(arduino_pwm) / sizeof(arduino_pwm[0])] = {0};





size_t pwm_pin_index(pin_size_t pinNumber) {
  for (size_t i = 0; i < ARRAY_SIZE(arduino_pwm_pins); i++) {
    if (arduino_pwm_pins[i] == pinNumber) {
      return i;
    }
  }
  return (size_t)-1;
}

uint32_t map64(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max) {
  return ((uint64_t)(x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

int _analog_write_last_status = 0;



void XanalogWrite(pin_size_t pinNumber, int value) {
  const int maxInput = BIT(analogWriteResolution()) - 1U;
  const int digitalThreshold = maxInput >> 1;
  size_t idx = pwm_pin_index(pinNumber);

  if (idx >= ARRAY_SIZE(arduino_pwm)) {
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, value > digitalThreshold ? HIGH : LOW);
    _analog_write_last_status = 0;
    return;
  }

  if (arduino_pwm_periods[idx] == 0) {
    arduino_pwm_periods[idx] = arduino_pwm[idx].period;
  }

  (void)zephyr::arduino::init_dev_apply_channel_pinctrl(arduino_pwm[idx].dev,
                     zephyr::arduino::state_pin_index_from_spec_index(arduino_pwm, idx));

  if (!pwm_is_ready_dt(&arduino_pwm[idx])) {
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, value > digitalThreshold ? HIGH : LOW);
    _analog_write_last_status = 0;
    return;
  }

  value = CLAMP(value, 0, maxInput);

  const uint32_t pulse = map64(value, 0, maxInput, 0, arduino_pwm_periods[idx]);

  /*
   * A duty ratio determines by the period value defined in dts
   * and the value arguments. So usually the period value sets as 255.
   */
  _analog_write_last_status = pwm_set(arduino_pwm[idx].dev, arduino_pwm[idx].channel,
                    arduino_pwm_periods[idx], pulse, arduino_pwm[idx].flags);
}



void XanalogWriteFrequency(pin_size_t pin, float freq) {
  size_t idx = pwm_pin_index(pin);

  // not a valid PWM PIN.
  if (idx >= ARRAY_SIZE(arduino_pwm)) {
    return;
  }

  arduino_pwm_periods[idx] = (uint32_t)(1000000000.0f / freq);
}

float XanalogWriteFrequency(pin_size_t pin) {
  size_t idx = pwm_pin_index(pin);

  // not a valid PWM PIN.
  if (idx >= ARRAY_SIZE(arduino_pwm)) {
    return -1.0f;
  }

  if (arduino_pwm_periods[idx] == 0) {
    arduino_pwm_periods[idx] = arduino_pwm[idx].period;
  }

  return 1000000000.0f / (float)arduino_pwm_periods[idx];
}

int XanalogWriteLastStatus() {
  return _analog_write_last_status;
}
