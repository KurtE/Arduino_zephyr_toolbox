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
#ifndef _ZEPHYR_DIGITALWRITEFAST_H_
#define _ZEPHYR_DIGITALWRITEFAST_H_
#include <Arduino.h>

#include "PinName_zephyr.h"
#include "wiring_private.h"

// By default the pinMode code, sees if the pin is one that is actually in the pin table
// if so it redirects to use the standard Arduino pinMode.
extern void pinMode(PinName pin_name, PinMode mode, bool bypass_pin_match = false);

extern uint8_t mapPinNameToPin(PinName pin_name);
extern PinName mapPinToPinName(uint8_t pin);

// Map PinName to the underlying Zyphr GPIO Device object and optionally the pin on the port
extern const struct device *mapPinToZephyrGPIODevice(uint8_t pin, uint8_t *port_pin = nullptr);
extern const struct device *mapPinNameToZephyrGPIODevice(PinName pin_name, uint8_t *port_pin = nullptr);

// quick and dirty digitalWriteFast

// Sets the state of an IO pin
// Two versions - this version you takes in a pin number as uint8_t type - uint8_t
//=============================================================================
// STM32 optimized functions (GIGA, Portenta H7, Nicla Vision)
//=============================================================================
#if defined(STM32H7) || defined(STM32U5)
extern GPIO_TypeDef * const stm32_gpio_port_table[];

struct gpio_stm32_config_head {
  /* gpio_driver_config needs to be first */
  struct gpio_driver_config common;
  /* port base address */
  uint32_t *base;
};


inline void digitalWriteFast(uint8_t pin, PinStatus val) {
  const struct gpio_stm32_config_head *cfg = (gpio_stm32_config_head*)zephyr::arduino::arduino_pins[pin].port->config;  
  GPIO_TypeDef *port = (GPIO_TypeDef *)cfg->base;
  //uint16_t mask = mask_table[arduino_pins[pin].pin];
  uint16_t mask = 1 << zephyr::arduino::arduino_pins[pin].pin;

  if (val) port->BSRR = mask;
  else port->BSRR = (uint32_t)(mask << 16);
}

// This version you takes in a pin name (PinName) like LED_RED
inline void digitalWriteFast(PinName pin_name, PinStatus val) {
  uint16_t mask = 1 << (pin_name & 0xf);
  GPIO_TypeDef  * const port = stm32_gpio_port_table[pin_name >> 4];
  if (val) port->BSRR = mask;
  else port->BSRR = (uint32_t)(mask << 16);
}


inline void digitalToggleFast(uint8_t pin) {
  const struct gpio_stm32_config_head *cfg = (gpio_stm32_config_head*)zephyr::arduino::arduino_pins[pin].port->config;  
  GPIO_TypeDef *portX = (GPIO_TypeDef *)cfg->base;
  uint16_t pin_mask = 1 << (zephyr::arduino::arduino_pins[pin].pin);

  if (portX->ODR & pin_mask) portX->BSRR = (uint32_t)(pin_mask << 16);
  else portX->BSRR = pin_mask;
}

// Toggles the state of an IO pin - pin name version
inline void digitalToggleFast(PinName pin_name) {
  uint16_t pin_mask = 1 << (pin_name & 0xf);
  GPIO_TypeDef  * const portX = stm32_gpio_port_table[pin_name >> 4];

  if (portX->ODR & pin_mask) portX->BSRR = (uint32_t)(pin_mask << 16);
  else portX->BSRR = pin_mask;
}


//=============================================================================
// Renesas optiomized functions: currently portenta C33
//=============================================================================
#elif defined(ARDUINO_PORTENTA_C33)
extern R_PORT0_Type *renesas_gpio_port_table[];

struct gpio_ra_config_head {
  struct gpio_driver_config common;
  uint8_t port_num;
  R_PORT0_Type *port;
};

inline void digitalWriteFast(uint8_t pin, PinStatus val) {
  const struct gpio_ra_config_head *config = (gpio_ra_config_head*)zephyr::arduino::arduino_pins[pin].port->config;
  R_PORT0_Type *port = config->port;
  uint16_t mask = 1 << zephyr::arduino::arduino_pins[pin].pin;

  if (val) port->POSR = mask;
  else port->PORR = mask;
}

inline void digitalWriteFast(PinName pin_name, PinStatus val) {
  uint16_t mask = 1 << (pin_name & 0xf);
  R_PORT0_Type *port = renesas_gpio_port_table[pin_name >> 4];
  if (val) port->POSR = mask;
  else port->PORR = mask;
}

inline void digitalToggleFast(uint8_t pin) {
  const struct gpio_ra_config_head *config = (gpio_ra_config_head*)zephyr::arduino::arduino_pins[pin].port->config;
  R_PORT0_Type *port = config->port;
  uint16_t mask = 1 << zephyr::arduino::arduino_pins[pin].pin;

  if (port->PODR & mask) port->PORR = mask;
  else port->POSR = mask;
}

inline void digitalToggleFast(PinName pin_name) {
  uint16_t mask = 1 << (pin_name & 0xf);
  R_PORT0_Type *port = renesas_gpio_port_table[pin_name >> 4];

  if (port->PODR & mask) port->PORR = mask;
  else port->POSR = mask;
}


//=============================================================================
// default less optimized functions.
//=============================================================================
#else

inline void digitalWriteFast(uint8_t pin, PinStatus val) {
    // defaulting to do like Arduino code
    gpio_pin_set_dt(&zephyr::arduino::arduino_pins[pin], val);
}

// This version you takes in a pin name (PinName) like LED_RED
inline void digitalWriteFast(PinName pin_name, PinStatus val) {
  gpio_pin_set(zephyr_gpio_devices[pin_name >> 4], pin_name & 0xf, val);
}


inline void digitalToggleFast(uint8_t pin) {
    // defaulting to do like Arduino code
     gpio_pin_toggle_dt(&zephyr::arduino::arduino_pins[pin]);
}

// Toggles the state of an IO pin - pin name version
inline void digitalToggleFast(PinName pin_name) {
  gpio_pin_toggle(zephyr_gpio_devices[pin_name >> 4], pin_name & 0xf);
}

#endif


//=============================================================================
// For all boards
//=============================================================================
inline void digitalWriteFast(uint8_t pin, int status) {
    digitalWriteFast(pin, (PinStatus)status);
};

inline void digitalWriteFast(PinName pin_name, int status) {
    digitalWriteFast(pin_name, (PinStatus)status);
};



// Reads the state of an IO pin - pin number version
extern PinStatus digitalReadFast(uint8_t pin);
extern PinStatus digitalReadFast(PinName pin_name);

// Map PinName to String
extern const char *pinNameToStr(PinName pin_name);
inline const char *pinNameToStr(int pin_name) {
    return pinNameToStr((PinName)pin_name);
}

//=============================================================================
// STM32 only functions
// Debug printing functions
//=============================================================================

#if defined(STM32H7) || defined(STM32U5)

extern GPIO_TypeDef const *mapPinNameToPortAndPin(PinName pin_name, uint8_t *port_pin=nullptr);

extern void pinNameSetModer(PinName pin_name, uint8_t pin_mode);
extern void PinNameSetAFR(PinName pin_name, uint8_t af);

extern void pinSetModer(uint8_t pin, uint8_t pin_mode);
extern void pinSetAFR(uint8_t pin, uint8_t af);

extern void print_gpio_regs(const char *name, GPIO_TypeDef *port);
extern void print_all_gpio_regs();

#endif
#endif