/**
 * SmartKar-Cano Main Entry Point
 * 
 * This file contains the Arduino setup() and loop() entry points.
 * All device logic is delegated to the DeviceController.
 */

#include <Arduino.h>
#include "core/DeviceController.h"

// The central device controller
DeviceController device;

void setup() {
    device.setup();
}

void loop() {
    device.loop();
}
