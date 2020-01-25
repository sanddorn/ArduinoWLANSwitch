//
// Created by nilsb on 02.12.19.
//

#ifndef ARDUINOWLANSWITCH_VALVEHANDLER_H
#define ARDUINOWLANSWITCH_VALVEHANDLER_H

#include <string>
#include <memory>
#include <AbstractCallbackHandler.h>
#include <ArduinoLog.h>

enum class VALVESTATE {
    UNKNOWN,
    OPENING,
    OPEN,
    CLOSING,
    CLOSED
} ;

using namespace std;

class ValveHandler {
public:
    ValveHandler(uint8_t valveOpemPin, uint8_t valveClosePin, std::shared_ptr<AbstractCallBackHandler> callbackHandler, Logging &logging);

    void openValve();

    void closeValve();

    void callBack();

    VALVESTATE getStatus();

    static void callbackMethod(void * instance);

private:
    uint8_t valveOpenPin;
    uint8_t valveClosePin;
    unsigned long lastChange;
    std::shared_ptr<AbstractCallBackHandler> callbackHandler;
    VALVESTATE valveState = VALVESTATE::UNKNOWN;
    Logging &logging;
};


#endif //ARDUINOWLANSWITCH_VALVEHANDLER_H
