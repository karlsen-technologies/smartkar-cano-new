#include "ConnectionManager.h"
#include <ArduinoJson.h>

ConnectionManager* ConnectionManager::_instance = nullptr;

ConnectionManager::ConnectionManager(Modem* modemManager) {
    this->modemManager = modemManager;
    this->_instance = this;
    this->client = new TinyGsmClient(*this->modemManager->getModem());
}

ConnectionManager::~ConnectionManager() {
}

bool ConnectionManager::setup() {

    if (this->modemManager->state == ModemState::MODEM_HOTSTART) {
        this->modemManager->getModem()->sendAT("+CACLOSE=0");
        this->modemManager->getModem()->waitResponse();
    }

    Serial.println("[CM] Setup OK!");

    return true;
}

bool ConnectionManager::prepareForSleep() {

    if (this->state == LinkState::LINK_CONNECTED) {
        String auth = "{\"type\":\"state\",\"data\":{\"type\":\"device\",\"state\":\"asleep\"}}";
        this->client->println(auth);
        this->client->stop();
        this->state = LinkState::LINK_DISCONNECTED;
    }

    return true;
}

void ConnectionManager::handleTCPInterrupt() {
    Serial.println("[CM] Received TCP interrupt!");

    int urc = this->modemManager->getModem()->waitResponse(15L, "STATE:", "DATAIND:");

    if (urc == 1) {

        int state = this->modemManager->getModem()->waitResponse(15L, "0,1", "0,0");

        Serial.println("[CM] Received update about connection state!");

        if (state == 1) {
            Serial.println("[CM] Modem was connected to server!");
            this->state = LinkState::LINK_CONNECTED;
        } else {
            Serial.println("[CM] Modem lost connection from server!");
            this->state = LinkState::LINK_DISCONNECTED;
        }

    } else if (urc == 2) {
        this->modemManager->getModem()->waitResponse(15L, AT_NL);
        Serial.println("[CM] Received data from server!");

        // TODO: Read multiple lines of data
        this->modemManager->getModem()->maintain();
        while(this->client->available()) {

            String dataStr = this->client->readStringUntil('\n');
            Serial.println("[CM] Received data: " + dataStr);

            // Allocate the JSON document
            JsonDocument data;

            // Deserialize the JSON document
            DeserializationError error = deserializeJson(data, dataStr);

            // Test if parsing succeeds
            if (error) {
                Serial.print(F("[CM] Parsing data failed: "));
                Serial.println(error.f_str());
                return;
            }

            // Check if the JSON data contains the key we are looking for
            if (data.containsKey("type")) {
                String type = data["type"];

                if (type == "authentication") {

                    if (data.containsKey("data")) {
                        JsonObject auth = data["data"];

                        if (auth.containsKey("action")) {
                            String status = auth["action"];

                            if (status == "accepted") {
                                Serial.println("[CM] Authenticated with server!");
                                this->state = LinkState::LINK_CONNECTED;
                            } else {
                                Serial.println("[CM] Rejected by server!");
                                this->state = LinkState::LINK_REJECTED;
                            }
                        } else {
                            Serial.println("[CM] Authentication does not contain action!");
                        }
                    } else {
                        Serial.println("[CM] Authentication does not contain data!");
                    }
                } else if (type == "command") {
                    Serial.println("[CM] Received command from server!");

                    if (data.containsKey("data")) {
                        JsonObject command = data["data"];

                        if (command.containsKey("action")) {
                            String type = command["action"];

                            if (type == "test") {
                                Serial.println("[CM] Received test command from server!");

                                int id = command["id"];

                                String response = "{\"type\":\"response\",\"data\":{\"id\":" + String(id) + ",\"action\":\"test\",\"status\":\"ok\"}}";
                                this->client->println(response);

                            } else {
                                Serial.println("[CM] Unknown command type!");
                            }
                        } else {
                            Serial.println("[CM] Command does not contain type!");
                        }
                    } else {
                        Serial.println("[CM] Command does not contain data!");
                    }
                } else {
                    Serial.println("[CM] Unknown message type!");
                }
            } else {
                Serial.println("[CM] Unknown message type!");
            }
        }

    } else {
        Serial.println("[CM] Unknown TCP interrupt!");
    }

}

void ConnectionManager::loop() {

    if (this->modemManager->state == ModemState::MODEM_CONNECTED) {
        // If we are not connected, try to connect
        if (this->state == LinkState::LINK_DISCONNECTED) {
            Serial.println("[CM] Connecting to server...");
            if (this->client->connect("gallant.kartech.no", 4589)) {
                Serial.println("[CM] Connected to server!");
                this->state = LinkState::LINK_AUTHENTICATING;

                // Send authentication message
                String auth = "{\"type\":\"authentication\",\"data\":{\"action\":\"authenticate\",\"ccid\":\"" + this->modemManager->getSimCCID() + "\"}}";
                this->client->println(auth);
            } else {
                Serial.println("[CM] Failed to connect to server!");
            }
        }
    }

    this->last_loop = millis();
}