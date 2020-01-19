//
// Created by nilsb on 02.12.19.
//

#ifndef ARDUINOWLANSWITCH_VALVEHANDLER_H
#define ARDUINOWLANSWITCH_VALVEHANDLER_H

#include <string>

enum VALVESTATE {
    UNKNOWN,
    OPENING,
    OPEN,
    CLOSING,
    CLOSED
} ;

using namespace std;

class ValveHandler {
public:
    ValveHandler(uint8_t valveOpemPin, uint8_t valveClosePin);

    void openValve(unsigned long now);

    void closeValve(unsigned long now);

    VALVESTATE getStatus();

    unsigned long getMillisWhenLastChange();

private:
    uint8_t valveOpenPin;
    uint8_t valveClosePin;
    unsigned long lastChange;
    VALVESTATE valveState = UNKNOWN;


};


#endif //ARDUINOWLANSWITCH_VALVEHANDLER_H
