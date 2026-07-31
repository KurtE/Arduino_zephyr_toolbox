/*
 *    digitalWriteFast_zephyr.h - A quick and dirty digitalWriteFast
 *    and digitalToggleFast for STM32 based sezphyr boards. There are better
 *    versions out there, but this good enough for my testing
 * 
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 * 
 *    This permission notice shall be included in all copies or 
 *    substantial portions of the Software.
 * 
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
 */
#include <Arduino.h>
#include "digitalWriteFast_zephyr.h"
#include <zephyr/drivers/gpio.h>

#include "wiring_private.h"
using namespace zephyr::arduino;


// Generate a list of the stm32 GPIO port zephyr device objects.
#define ADD_GPIO_NODE(node_id) DEVICE_DT_GET(node_id),

const struct device *zephyr_gpio_devices[] = {
#if defined(STM32H7) || defined(STM32U5)
    DT_FOREACH_STATUS_OKAY(st_stm32_gpio, ADD_GPIO_NODE)
#elif defined(ARDUINO_PORTENTA_C33)
    DT_FOREACH_STATUS_OKAY(renesas_ra_gpio_ioport, ADD_GPIO_NODE)
#endif
};

const uint8_t COUNT_ZEPHYR_GPIO_DEVICES = (sizeof(zephyr_gpio_devices) / sizeof(zephyr_gpio_devices[0]));


uint8_t mapPinNameToPin(PinName pin_name) {
  const struct device *port_device = zephyr_gpio_devices[pin_name >> 4];
  uint8_t pin_on_port = pin_name & 0xf;

  for (uint8_t pin_num = 0; pin_num < NUM_OF_DIGITAL_PINS; pin_num++) {
    if ((arduino_pins[pin_num].port == port_device) && 
        (arduino_pins[pin_num].pin == pin_on_port)) {

        return pin_num;
    }
  }

  return 0xff;  // pin name not in Arduino Pin list
}


PinName mapPinToPinName(uint8_t pin) {

  // now find this port in our Port list;
  for (uint8_t i = 0; i < COUNT_ZEPHYR_GPIO_DEVICES; i++) {
    if (arduino_pins[pin].port == zephyr_gpio_devices[i]) {
      return (PinName)((i << 4) | arduino_pins[pin].pin);
    }
  }
  return (PinName)PX_INVALID;
}

const struct device *mapPinToZephyrGPIODevice(uint8_t pin, uint8_t *port_pin) {
  if (pin >= NUM_OF_DIGITAL_PINS)
  if (port_pin) return nullptr;

  if(port_pin) *port_pin = arduino_pins[pin].pin;

  return arduino_pins[pin].port;
}


extern const struct device *mapPinNameToZephyrGPIODevice(PinName pin_name, uint8_t *port_pin) {
  if (pin_name >= PX_COUNT) return nullptr;

  if (port_pin) *port_pin = pin_name & 0xf;

  return zephyr_gpio_devices[pin_name >> 4];
}



void pinMode(PinName pin_name, PinMode mode, bool bypass_pin_match) {
  if (pin_name >= PX_COUNT) return;

  const struct device *port_device = zephyr_gpio_devices[pin_name >> 4];
  uint8_t pin_on_port = pin_name & 0xf;

  // if we are not told to bypass the pin matching
  if (!bypass_pin_match) {
    for (uint8_t pin_num = 0; pin_num < NUM_OF_DIGITAL_PINS; pin_num++) {
      if ((arduino_pins[pin_num].port == port_device) && 
          (arduino_pins[pin_num].pin == pin_on_port)) {

          pinMode(pin_num, mode);
          return;
      }
    }
  }

  if (mode == INPUT) { // input mode
    gpio_pin_configure(port_device, pin_on_port, 
                       GPIO_INPUT | GPIO_ACTIVE_HIGH);
  } else if (mode == INPUT_PULLUP) { // input with internal pull-up
    gpio_pin_configure(port_device, pin_on_port, 
                       GPIO_INPUT | GPIO_PULL_UP | GPIO_ACTIVE_HIGH);
  } else if (mode == INPUT_PULLDOWN) { // input with internal pull-down
    gpio_pin_configure(port_device, pin_on_port, 
                       GPIO_INPUT | GPIO_PULL_DOWN | GPIO_ACTIVE_HIGH);
  } else if (mode == OUTPUT) { // output mode
    gpio_pin_configure(port_device, pin_on_port, 
                       GPIO_OUTPUT_LOW | GPIO_ACTIVE_HIGH);
  }
}

