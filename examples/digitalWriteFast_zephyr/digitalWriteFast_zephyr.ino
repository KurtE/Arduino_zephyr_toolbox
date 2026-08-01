//=============================================================================
// Zephyr Quick and dirty test to compare the speed of digitalWrite versus
// the versions in this library, which in some cases bypass zephyr
//=============================================================================

#include <Arduino_zephyr_toolbox.h>
#ifdef ARDUINO_UNO_Q
#define PIN 2
#define PIN_MARKER 3
#define PIN_NAME PB_3
#else
#define PIN 2
#define PIN_MARKER 3
#if defined (ARDUINO_PORTENTA_H7_M7)
#define PIN_NAME PJ_11
#elif defined(ARDUINO_GIGA)
#define PIN_NAME PA_3
#elif defined(ARDUINO_PORTENTA_C33)
#define PIN_NAME P1_11

#endif

#endif

PinName pin_name;


void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000)
    ;

  Serial.println("\n\nTest");
  Serial.flush();

  pin_name = mapPinToPinName(PIN);
  Serial.print("Pin: ");
  Serial.print(PIN);
  Serial.print(" -> ");
  Serial.println(pinNameToStr(pin_name));

  pinMode(PIN, OUTPUT);

  //pinMode(pin_name, OUTPUT);
  pinMode(PIN_MARKER, OUTPUT);
  delay(250);
}


void do_digitalWrite() {

  digitalWrite(PIN_MARKER, HIGH);
  uint32_t start_time = micros();
  for (int i = 0; i < 1000; i++) {
    digitalWrite(PIN, HIGH);
    digitalWrite(PIN, LOW);
  }
  uint32_t delta_time = micros() - start_time;
  digitalWrite(PIN_MARKER, LOW);
  Serial.print("digitalWrite: ");
  Serial.println(delta_time, DEC);
}

void do_digitalWriteFast() {
  digitalWrite(PIN_MARKER, HIGH);
  uint32_t start_time = micros();
  for (int i = 0; i < 1000; i++) {
    digitalWriteFast(PIN, HIGH);
    digitalWriteFast(PIN, LOW);
  }
  uint32_t delta_time = micros() - start_time;
  digitalWrite(PIN_MARKER, LOW);
  Serial.print("digitalWriteFast: ");
  Serial.println(delta_time, DEC);
}

void do_digitalWriteFastName() {
  digitalWrite(PIN_MARKER, HIGH);
  uint32_t start_time = micros();
  for (int i = 0; i < 1000; i++) {
    digitalWriteFast(pin_name, HIGH);
    digitalWriteFast(pin_name, LOW);
  }
  uint32_t delta_time = micros() - start_time;
  digitalWrite(PIN_MARKER, LOW);
  Serial.print("digitalWriteFast(name-var): ");
  Serial.println(delta_time, DEC);
}

void do_digitalWriteFastName_const() {
#ifdef PIN_NAME
  digitalWrite(PIN_MARKER, HIGH);
  uint32_t start_time = micros();
  for (int i = 0; i < 1000; i++) {
    digitalWriteFast(PIN_NAME, HIGH);
    digitalWriteFast(PIN_NAME, LOW);
  }
  uint32_t delta_time = micros() - start_time;
  digitalWrite(PIN_MARKER, LOW);
  Serial.print("digitalWriteFast(name defined): ");
  Serial.println(delta_time, DEC);
#endif  
}


void  do_zephyr_gpio_writes() {
  digitalWrite(PIN_MARKER, HIGH);
  uint8_t pin_on_port;
  const struct device *dev = mapPinNameToZephyrGPIODevice(pin_name, &pin_on_port);
  uint32_t start_time = micros();
  for (int i = 0; i < 1000; i++) {
    gpio_pin_set(dev, pin_on_port, 1);
    gpio_pin_set(dev, pin_on_port, 0);
  }
  uint32_t delta_time = micros() - start_time;
  digitalWrite(PIN_MARKER, LOW);
  Serial.print("digitalWriteFast(Port): ");
  Serial.println(delta_time, DEC);
}


void do_digitalToggleFast() {
  digitalWrite(PIN_MARKER, HIGH);
  uint32_t start_time = micros();
  for (int i = 0; i < 1000; i++) {
    digitalToggleFast(PIN);
    digitalToggleFast(PIN);
  }
  uint32_t delta_time = micros() - start_time;
  digitalWrite(PIN_MARKER, LOW);
  Serial.print("digitalToggleFast: ");
  Serial.println(delta_time, DEC);
}

void do_digitalToggleFastName() {
  digitalWrite(PIN_MARKER, HIGH);
  uint32_t start_time = micros();
  for (int i = 0; i < 1000; i++) {
    digitalToggleFast(pin_name);
    digitalToggleFast(pin_name);
  }
  uint32_t delta_time = micros() - start_time;
  digitalWrite(PIN_MARKER, LOW);
  Serial.print("digitalToggleFast(name): ");
  Serial.println(delta_time, DEC);
}


void loop() {
  if (Serial.available()) {
    while (Serial.available()) Serial.read();
    Serial.println("*** Paused ***");
    while (!Serial.available()) {}
    while (Serial.available()) Serial.read();
  }
  for (uint8_t i = 0; i < 5; i++) {
    digitalWrite(PIN, HIGH);
    delay(5);
    digitalWrite(PIN, LOW);
    delay(5);
  }
  do_digitalWrite();
  Serial.flush();
  do_digitalWriteFast();
  Serial.flush();
  do_digitalWriteFastName_const();
  Serial.flush();
  do_digitalWriteFastName();
  Serial.flush();
  do_digitalToggleFast();
  Serial.flush();
  do_digitalToggleFastName();
  Serial.flush();
  do_zephyr_gpio_writes();
  Serial.println();
  delay(1000);
}
