//
// Created by Nils Bokermann on 21.01.20.
//

#include "TimedCallbackHandler.h"

static void internalCallBackfunction() {
    TimedCallbackHandler::getInstance()->callbackHit();
}

std::shared_ptr<TimedCallbackHandler> TimedCallbackHandler::callbackHandler;

void TimedCallbackHandler::startCallbackTimer(long millis, void (*function)(void *), void *arg) {
    logging.trace("startCallbackTimer begin");
    if (this->payload != nullptr) {
        // Error
    }
    this->payload = arg;
    this->callbackFunction = function;
    ticker.once(millis/1000.0f, function, arg);
    logging.trace("startCallbackTimer end");

}

void TimedCallbackHandler::callbackHit() {
    callbackFunction(payload);
    payload = nullptr;
    callbackFunction = nullptr;
}

std::shared_ptr<TimedCallbackHandler> TimedCallbackHandler::getInstance() {
    if (TimedCallbackHandler::callbackHandler == nullptr) {
        TimedCallbackHandler::callbackHandler = std::shared_ptr<TimedCallbackHandler>(new TimedCallbackHandler());
    }
    return TimedCallbackHandler::callbackHandler;
}


