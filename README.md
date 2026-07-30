# Overview and Warning: 

This Arduino library is an experiment and a WIP, which may never go anywhere.

The idea in this library is to allow me to have access to all of the GPIO pins
on the Arduino Zephyr STM32 based boards, such as Giga, Portenta H7, UNO Q

I have introduced pin names for all possible GPIO pins, and then
added digital functions such as digitalWriteFast, digitalReadFast,
digitalToggleFast for all of these pins.  For the fun of it, I also have
Fast version for the actual Arduino pins as well.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


This library uses concepts and relies on the ArduinoCore-zephyr as well as 
Zephyr.

Warning: this document is still WIP, and many of the features are likely
to change and be reorganized.

#Current Features:

## PinName:

There is currently an Enum defined for most of the pins on an STM32 based
board.  

```
typedef enum {
    PA_0=0x00, PA_1, PA_2, PA_3, PA_4, PA_5, PA_6, PA_7, PA_8, PA_9, PA_10, PA_11, PA_12, PA_13, PA_14, PA_15,
    PB_0=0x10, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7, PB_8, PB_9, PB_10, PB_11, PB_12, PB_13, PB_14, PB_15,
    PC_0=0x20, PC_1, PC_2, PC_3, PC_4, PC_5, PC_6, PC_7, PC_8, PC_9, PC_10, PC_11, PC_12, PC_13, PC_14, PC_15,
    PD_0=0x30, PD_1, PD_2, PD_3, PD_4, PD_5, PD_6, PD_7, PD_8, PD_9, PD_10, PD_11, PD_12, PD_13, PD_14, PD_15,
    PE_0=0x40, PE_1, PE_2, PE_3, PE_4, PE_5, PE_6, PE_7, PE_8, PE_9, PE_10, PE_11, PE_12, PE_13, PE_14, PE_15,
    PF_0=0x50, PF_1, PF_2, PF_3, PF_4, PF_5, PF_6, PF_7, PF_8, PF_9, PF_10, PF_11, PF_12, PF_13, PF_14, PF_15,
    PG_0=0x60, PG_1, PG_2, PG_3, PG_4, PG_5, PG_6, PG_7, PG_8, PG_9, PG_10, PG_11, PG_12, PG_13, PG_14, PG_15,
    PH_0=0x70, PH_1, PH_2, PH_3, PH_4, PH_5, PH_6, PH_7, PH_8, PH_9, PH_10, PH_11, PH_12, PH_13, PH_14, PH_15,
    PI_0=0x80, PI_1, PI_2, PI_3, PI_4, PI_5, PI_6, PI_7, 
#if !defined(STM32U5)  // UNO_Q does not have the following pins
    PI_8, PI_9, PI_10, PI_11, PI_12, PI_13, PI_14, PI_15,
    PJ_0=0x90, PJ_1, PJ_2, PJ_3, PJ_4, PJ_5, PJ_6, PJ_7, PJ_8, PJ_9, PJ_10, PJ_11, PJ_12, PJ_13, PJ_14, PJ_15,
    PK_0=0xA0, PK_1, PK_2, PK_3, PK_4, PK_5, PK_6, PK_7,
#endif
    PX_COUNT, PX_INVALID=0xff
} PinName;
```

The format of the pin names follows the naming used in many of the Arduino documents such as the Software mapping for the Mid Carrier:
https://docs.arduino.cc/static/8ef771ee25fbb0fd245598df70429cbf/ASX00055-software-pinout-mapping.pdf


Other documents such as the Portenta Full pinout:
https://docs.arduino.cc/resources/pinouts/ABX00042-full-pinout.pdf
Use a different format instead of: PA_0 they use PA0

### Mapping functions

The header file digitalWriteFast_zephyr.h defines some functions that 
allow you to map between the two:

```
extern uint8_t mapPinNameToPin(PinName pin_name);
```
Maps a pin name to the correspoinding pin name.
For example, on Portenta H7: mapPinNameToPin(PJ_11) will return pin 2.

likewise:
```
extern PinName mapPinToPinName(uint8_t pin);
```
Maps a pin number to a pin name, so mapPinToPinName(2) will return PJ_11.  If the pin number passed in is invalid it will return PX_INVALID.

The helper function:
```
extern GPIO_TypeDef const *mapPinNameToPortAndPin(PinName pin_name, uint8_t *port_pin);
```
Maps a pin name to a pointer to the GPIO registers structure, and return which pin in that GPIO register is for that pin.
So for example one could do:
```
uint8_t port_pin;
GPIO_TypeDef const *port = mapPinNameToPortAndPin(PJ_11, &port_pin);
port->BSRR = (1 << port_pin);
```
Will set that pin to the on state.

Map pin number or pin name to the underlying Zephyr GPIO Port object and optionally pin number on port:
````
extern const struct device *mapPinToZephyrGPIODevice(uint8_t pin, uint8_t *port_pin = nullptr);
extern const struct device *mapPinNameToZephyrGPIODevice(PinName pin_name, uint8_t *port_pin = nullptr);
```
Which allows you to do things like:
```
const struct device *led_dev; 
uint8_t led_dev_pin;
...
  led_dev = mapPinToZephyrGPIODevice(LED_BUILTIN, &led_dev_pin); 
...
  gpio_pin_toggle(led_dev, led_dev_pin);
```


The function: 
```
extern const char *pinNameToStr(PinName pin_name);
inline const char *pinNameToStr(int pin_name) {
    return pinNameToStr((PinName)pin_name);
}
```
Will convert the pin name value from the enum and return a const char * 
for the actual name.


### Set usage mode of Pins using PinNames

To set up a pin to be in GPIO modes like: INPUT, INPUT_PULLUP, INPUT_PULLDOWN, OUTPUT.
By default, it will check to see if the PinName maps to an existing GPIO pin and
then call the standard Arduino pinMode(pin, PinMode).  There is an optional parameter
that if set to true will bypass this check, and simply map the pin name object, to the
zephyr port device object and pin.
```
extern void pinMode(PinName pin_name, PinMode mode, bool bypass_pin_match = false);
```

Set the MODER register for this pin name, 2 bits per pin(Input, Output, Alternate function, Analog
	)
```
extern void pinSetModer(uint8_t pin, uint8_t pin_mode);
```

Set the Alternate function value for a Pin Name, 4 bits per pin.  The meaning for what the values are changes for each pin.  
```
extern void pinSetAFR(uint8_t pin, uint8_t af);
```

### digital functions

For most of these functions I appended Fast to the end of the normal Arduino GPIO functions.
I also added a pin toggle function as I use them on Teensy boards.

```
extern void digitalWriteFast(PinName pin_name, PinStatus val);
inline void digitalWriteFast(PinName pin_name, int status) {
    digitalWriteFast(pin_name, (PinStatus)status);
};


extern void digitalToggleFast(PinName pin_name);

extern PinStatus digitalReadFast(PinName pin_name);

```

## Fast GPIO Functions on Arduino Pin Numbers

```
extern void digitalWriteFast(uint8_t pin, PinStatus val);
inline void digitalWriteFast(uint8_t pin, int status) {
    digitalWriteFast(pin, (PinStatus)status);
};

extern void digitalToggleFast(uint8_t pin);

extern PinStatus digitalReadFast(uint8_t pin);
```

## Debug Printing functions:

To print debug information for an STM32 GPIO register structure.
```
extern void print_gpio_regs(const char *name, GPIO_TypeDef *port);
```

To Print out these registers for all of the GPIO registers structures on the
STM32 board.
```
extern void print_all_gpio_regs();
```

Example output for Portenta H7 using the example zephyr_pinname_HiLowTest
```
GPIOA 800088A8 : FIIIIIIIFIFIFFFI B0A0ABB0 B0000000 FF75 0 55551101 : UUUUUUUU-U-U---U
GPIOB AA0080A : IIFFFFIIIIFIIIFF 44A990AA 99AAAA00 D3DC C0 50055150 : UU----UUUU-UUU--
GPIOC 32AA0A0A : IAIFFFFFIIFFIIFF BB00BA CCCCC AFCC 0 45555050 : U-UUUUUUUU--UU--
GPIOD AAAA002A : FFFFFFFFIIIIIFFF BB000CCC CC999CCC BCFD 0 50155555 : UU---UUUUUUUUUUU
GPIOE AAAABF0A : FFFFFFFFFAAAIIFF C0000055554055 : UUUUUUUUU---UUUU
GPIOF AAAAAAAA : FFFFFFFFFFFFFFFF 97CCCCCC CCCCC977 BC0 0 55440555 : UUUUU-U---UUUUUU
GPIOG 8A822A2A : FIFFFIIFIFFFIFFF ACC0CCC C7BBB07C 4788 0 50154555 : UU---UUUU-UUUUUU
GPIOH AA7 : IIIIIIIIIIFFFFOA CACC00 0 FFEE 2 55555450 : UUUUUUUUUUU-UU--
GPIOI 3800000 : IIIAFIIIIIIIIIII 0 A000 E7FF 0 54155555 : UUU--UUUUUUUUUUU
GPIOJ 7F000D57 : OAAAIIIIIIAOOOOA 0 0 8FD2 8012 555000 : ----UUUUUU------
GPIOK FFFFDFF3 : AAAAAAAAAOAAAAIA 0 0 42 40 4 : --------------U-
```

Break down of the different values printed above, for example GPIOA:

**ModeR** = 0x800088A8 the FIIIIIIIFIFIFFFI is for the 16 pins
on GPIOA, One char per I=Input, F=Function, O=Output, A=Analog

**AFRL**  (Low 8 GPIO pins) = B0A0ABB0 one nibble per pin so PA_1 is 0xB which looking at
my excel document appears to be an Ethernet function.

**AFRH**  (High 8 GPIO pins) = B0000000 like AFRL

**IDR** = 0XFF75 - Input data register

**ODR** = 0x0 - Output Data register

**PUPDR** (Pull up/Down register) = 55551101 : UUUUUUUU-U-U---U 
First part is the actual hex value, The second part is decoded per pin,
where U is UP, D is Down - is not set.

## Analog Write with Frequency

This adds the ability for the Arduino Zephyr user to be able to set the PWM
frequency for each of the different PWM pins. 

Currently this uses a different header file wiring_analog_extended.h as the
functionally, that is in the Pull request:
https://github.com/arduino/ArduinoCore-zephyr/pull/493


I pre-appended a X to the names of the different functions.

```
void XanalogWriteFrequency(pin_size_t pin, float freq);
float XanalogWriteFrequency(pin_size_t pin);
int XanalogWriteLastStatus();
void XanalogWrite(pin_size_t pinNumber, int value);
```

# Example Sketches

**digitalWriteFast_zephyr**: Compares the speed of digital Writes using different methods: digitalWrite, digitalWriteFast(pin), digitalWriteFast(pin_name)

**print_pin_table**: Prints out the conversion of pin number to pin name, three different ways:  First way is Pin Name, pin which I use extract and use in excel document to map tables extracted from Arduino documents.  Second and third part are condensed tables.
This is for the current Portenta H7
```
*** GPIO PORT/Pin To Arduino Pin numbers mapping ***

GPIOX: 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
====== == == == == == == == == == == == == == == == ==
GPIOA: -- -- -- -- 21 -- 22 --  6 14 13 -- -- -- -- --
GPIOB: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
GPIOC: -- -- 10  8 -- --  5  4 -- -- -- -- -- -- -- --
GPIOD: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
GPIOE: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
GPIOF: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
GPIOG: -- -- -- -- -- -- --  3 -- -- -- -- -- -- -- --
GPIOH: -- -- -- -- -- -- -- 12 11 -- -- -- -- -- --  0
GPIOI:  7  9 -- -- -- -- -- -- -- -- -- -- 26 -- -- --
GPIOJ: -- -- -- -- -- -- -- -- -- -- --  2 -- -- -- --
GPIOK: --  1 -- -- -- 23 24 25

*** Arduino Pin To Pin Name mapping table ***

Pin   0     1     2     3     4     5     6     7     8     9
=== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
  0 PH_15 PK_1  PJ_11 PG_7  PC_7  PC_6  PA_8  PI_0  PC_3  PI_1 
 10 PC_2  PH_8  PH_7  PA_10 PA_9  ----- ----- ----- ----- -----
 20 ----- PA_4  PA_6  PK_5  PK_6  PK_7  PI_12
```

**zephyr_analogWrite_change_frequency**: Example setting the Analog Write frequency one pin.  Which I verified the
speed using Logic Analyzer

**zephyr_pinname_HiLowTest**: This is an example sketch that I use to ring out boards, to see what pin is what.
In this case, it uses the Pin names to manipulate the pins.  It runs in two modes which toggle by entering anything
in the Serial monitor: Starts off by setting all pins to INPUT_PULLUP and then scans all the pins, to see which ones
change values and prints those pins out.  It prints out the Pin Name and if there is an Arduino pin number assigned to
it, it prints that out as well in ().  Note: this sketch has some pin exclude tables in it for some of the boards, 
as for example you don't want to set the pins associated with SDRAM and the like to some random state...

# Reminder

Again this is an experimental library that will probably always be a Work(Play) In Progress.  Some pieces hopefully
will disappear as hopefully some features will make it into the main ArduinoCore-zephyr functionality.

I hope some of this is useful but again Use at your own risk!