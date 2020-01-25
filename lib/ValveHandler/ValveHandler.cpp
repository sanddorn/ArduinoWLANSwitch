//
// Created by nilsb on 02.12.19.
//

#include "ValveHandler.h"
#include <Arduino.h>
#include <functional>
#include <utility>

static const long switchTime = 5000;

void ValveHandler::openValve() {
    logging.trace("openValve start");
    valveState = VALVESTATE::OPENING;
    digitalWrite(valveOpenPin, HIGH);
    callbackHandler->startCallbackTimer(switchTime, ValveHandler::callbackMethod, this);
    logging.trace("openValve stop");
}

void ValveHandler::closeValve() {
    valveState = VALVESTATE::CLOSING;
    digitalWrite(valveClosePin, HIGH);
    callbackHandler->startCallbackTimer(switchTime, ValveHandler::callbackMethod, this);
}

VALVESTATE ValveHandler::getStatus() {
    return valveState;
}

void ValveHandler::callBack() {
    logging.trace("callBack start");
    switch (valveState) {
        case VALVESTATE::OPENING:
            digitalWrite(valveOpenPin, LOW);
            valveState = VALVESTATE::OPEN;
            break;
        case VALVESTATE::CLOSING:
            digitalWrite(valveClosePin, LOW);
            valveState = VALVESTATE::CLOSED;
            break;
        default:
            logging.error("Calling callback with unusable State: %s", valveState);
            break;
    }
    logging.trace("callBack stop");
}

void ValveHandler::callbackMethod(void *instance) {
    auto *handler = static_cast<ValveHandler *>(instance);
    handler->callBack();

}

ValveHandler::ValveHandler(uint8_t valueOpenPin, uint8_t valveClosePin,
                           std::shared_ptr<AbstractCallBackHandler> callbackHandler,
                           Logging &log) :
        valveOpenPin(valueOpenPin),
        valveClosePin(valveClosePin),
        callbackHandler(std::move(callbackHandler)),
        logging(log) {}


