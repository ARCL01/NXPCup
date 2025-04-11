#ifndef _MBED_ARDUINO_COMPAT_H
#define _MBED_ARDUINO_COMPAT_H

#include "mbed.h"

// Arduino kompatibilní funkce pro mbed platformu
// Tyto deklarace musí být PŘED vložením hlavičkových souborů Pixy2

// Časovací funkce
inline void delayMicroseconds(uint32_t us) {
    wait_us(us);
}

#endif // _MBED_ARDUINO_COMPAT_H