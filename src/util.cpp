#include "util.h"
#include <Arduino.h>

unsigned long millis_since(unsigned long start) {
    return (unsigned long)(millis() - start);
}