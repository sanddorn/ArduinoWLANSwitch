//
// Created by nilsb on 02.12.19.
//

#include "ValveHandler.h"
#include <Arduino.h>

void ValveHandler::openValve() {
    valveState = VALVE_OPEN;
    digitalWrite(valvePin, HIGH);
}

void ValveHandler::closeValve() {
    valveState = VALVE_CLOSED;
    digitalWrite(valvePin, LOW);
}

unsigned char ValveHandler::getStatus() {
    return valveState;
}

ValveHandler::ValveHandler(uint8_t valuePin) {
    valvePin = valuePin;
}

void ValveHandler::switchValve() {
    digitalWrite(valvePin, valveState == VALVE_OPEN ? HIGH : LOW);
}


