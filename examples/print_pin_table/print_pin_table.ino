//=============================================================================
// Zephyr print pin table information for the STM32 Zephyr based boards such as
// Giga, Portenta H7, Nicla Vision, Arduino Uno Q
// 
// Prints out in three section
// First section, is comma seperated list of Pin name to Pin number, 
// I extract and use this in excel to be able to map pin names in documents to
// what Arduino Pin, probably could condense this to only those pins found.
// PA_0,
// PA_1,
// PA_2,
// PA_3,
// PA_4,21
// ...
// The Second and third parts are tables to easily find what pin pin maps
// to what pin name, and like wise from pin name to Arduino pin pin_number
// Example from Portenta H7
// *** GPIO PORT/Pin To Arduino Pin numbers mapping ***
// 
// GPIOX: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
// ====== == == == == == == == == == == == == == == == ==
// GPIOA: -- -- -- -- 21 -- 22 --  6 14 13 -- -- -- -- --
// GPIOB: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// GPIOC: -- -- 10  8 -- --  5  4 -- -- -- -- -- -- -- --
// GPIOD: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// GPIOE: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// GPIOF: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
// GPIOG: -- -- -- -- -- -- --  3 -- -- -- -- -- -- -- --
// GPIOH: -- -- -- -- -- -- -- 12 11 -- -- -- -- -- --  0
// GPIOI:  7  9 -- -- -- -- -- -- -- -- -- -- 26 -- -- --
// GPIOJ: -- -- -- -- -- -- -- -- -- -- --  2 -- -- -- --
// GPIOK: --  1 -- -- -- 23 24 25
// 
// *** Arduino Pin To Pin Name mapping table ***
// 
// Pin   0     1     2     3     4     5     6     7     8     9
// === ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
//   0 PH_15 PK_1  PJ_11 PG_7  PC_7  PC_6  PA_8  PI_0  PC_3  PI_1 
//  10 PC_2  PH_8  PH_7  PA_10 PA_9  ----- ----- ----- ----- -----
//  20 ----- PA_4  PA_6  PK_5  PK_6  PK_7  PI_12
//=============================================================================


#include <Arduino_zephyr_toolbox.h>
//#define PRINT_DEBUG_PIN_ENUM

#ifdef ARDUINO_UNO_Q
#define LAST_PIN_NAME (uint8_t)PI_7
#else
#define LAST_PIN_NAME (uint8_t)PK_7
#endif

PinName led_builtin_pn;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}

  for (uint8_t i = 0; i < PinName::PX_COUNT; i++) {
    PinName pn = (PinName)i;
    uint8_t pin_number = mapPinNameToPin(pn);

    Serial.print(pinNameToStr(pn));
    Serial.print(",");
    if (pin_number != 0xff) Serial.print(pin_number);
    Serial.println();
  }

  // Lets print a condensed GPIO Table
  Serial.println("\n*** GPIO PORT/Pin To Arduino Pin numbers mapping ***");
  Serial.println("\nGPIOX: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15");
  Serial.print("====== == == == == == == == == == == == == == == == ==");

  for (uint8_t i = 0; i < PinName::PX_COUNT; i++) {
    PinName pn = (PinName)i;
    uint8_t pin_number = mapPinNameToPin(pn);
    if ((i & 0xf) == 0) {
      Serial.print("\nGPIO");
      Serial.write('A' + (i >> 4));
      Serial.print(":");
    }
    Serial.print(" ");
    if (pin_number == 0xff) Serial.print("--");
    else {
      if (pin_number < 10) Serial.print(" ");
      Serial.print(pin_number);
    }
  }
  Serial.println();

  Serial.println("\n*** Arduino Pin To Pin Name mapping table ***");
  Serial.flush();
  Serial.println("\nPin   0     1     2     3     4     5     6     7     8     9");
  Serial.print    ("=== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====");

  // for the fun of it print out pin to PinName map as well
  for (uint8_t i = 0; i < NUM_OF_DIGITAL_PINS; i++) {
    if ((i % 10) == 0) {
      Serial.print("\n");
      if (i < 10) Serial.print("  ");
      else if (i < 100) Serial.print(" ");
      Serial.print(i);
    }

    PinName pn = mapPinToPinName(i);
    if (pn == PX_INVALID) {
      Serial.print(" -----");
    } else {
      const char *sz = pinNameToStr(pn);
      Serial.print(" ");
      Serial.print(sz);
      if (strlen(sz) < 5) Serial.print(" ");
    }
  }
  Serial.println("\n");

  // Lets try LED_BUILTIN
  led_builtin_pn = mapPinToPinName(LED_BUILTIN);
  Serial.print("LED_BUILTIN(");
  Serial.print(LED_BUILTIN);
  Serial.print("): ");
  Serial.println(pinNameToStr(led_builtin_pn));

  pinMode(led_builtin_pn, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWriteFast(led_builtin_pn, !digitalReadFast(led_builtin_pn));
  delay(500);

}
