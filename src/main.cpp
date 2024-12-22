#include <Arduino.h>

#include "driver/rtc_io.h"

#include "PowerManager.h"
#include "Network/Modem.h"
#include "Network/Link/ConnectionManager.h"

PowerManager powerManager;
Modem modemManager(&powerManager);
ConnectionManager connectionManager(&modemManager);

enum {
    CANO_INITIALIZING,
    CANO_INITIALIZED,
    CANO_AWAKE,
    CANO_GOING_TO_SLEEP,
    CANO_ASLEEP,
    CANO_WAKING_UP,
} canoState;

void setup() {
    canoState = CANO_INITIALIZING;

    Serial.begin(115200);

    /*// Wait for USB Serial
    while (!Serial);

    delay(3000);

    Serial.println();*/

    // Initialize the power manager
    powerManager.setup();

    // Initialize the modem
    modemManager.setup();

    // Initialize the connection manager
    connectionManager.setup();

    canoState = CANO_INITIALIZED;

    switch(esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_TIMER: // if the ESP32 is wake up by timer
            Serial.println("Wakeup caused by timer");
            break;
        case ESP_SLEEP_WAKEUP_GPIO: // if the ESP32 is wake up by GPIO
            Serial.println("Wakeup caused by GPIO");
            break;
        case ESP_SLEEP_WAKEUP_EXT1: // if the ESP32 is wake up by ext1
            Serial.println("Wakeup caused by ext1");
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            break;
        default:
            Serial.println("Unknown wakeup cause");
            break;
    }

    sleep(1);
}

unsigned long wake_time = 0;
unsigned long last_msg = 0;

bool light = false;

int unsavedCount=0;

bool mqttConnected = true;

void loop() {
    if (canoState == CANO_INITIALIZED) {
        // Lets get things setup after a boot

        // Enable the modem
        modemManager.enableModem();

        // TODO: Initialize the CAN bus, and deduce the cars current state
        // TODO: Initialize the MQTT connection, and send the current state to the server
        
        wake_time = millis();
        canoState = CANO_AWAKE;
    } else if (canoState == CANO_AWAKE) {

        // Handle interrupts
        modemManager.handleModemInterrupt();

        powerManager.getPMU()->setChargingLedMode(XPOWERS_CHG_LED_ON);

        modemManager.loop();
        connectionManager.loop();

        // If it has been 5 minutes since we woke up, we should go to sleep
        if (millis_since(wake_time) > MINUTES(1)) {
            canoState = CANO_GOING_TO_SLEEP;
        }

    } else if (canoState == CANO_GOING_TO_SLEEP) {
        // We are going to sleep, prepare for it.
        Serial.println("Going to sleep!");

        // TODO: Tell the server we are going to sleep, disconnect from MQTT
        // TODO: Prepare CAN bus interrupts
        // TODO: Then we should go into sleep ourselves.
        connectionManager.prepareForSleep();

        modemManager.disableInterrupt();
        powerManager.enableDeepSleep();

        powerManager.getPMU()->setChargingLedMode(XPOWERS_CHG_LED_OFF);

        Serial.flush();

        canoState = CANO_ASLEEP;
    } else if (canoState == CANO_ASLEEP) {
        esp_deep_sleep_start();
    }
}