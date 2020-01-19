//
// Created by nilsb on 02.12.19.
//

#ifndef ARDUINOWLANSWITCH_VALVEHANDLER_H
#define ARDUINOWLANSWITCH_VALVEHANDLER_H

#include <string>
#define VALVE_OPEN 0
#define VALVE_CLOSED 1
using namespace std;
class ValveHandler {
public:
    ValveHandler(uint8_t valvePin);
    void openValve();
    void closeValve();
    unsigned char getStatus();
private:
    uint8_t valvePin;
    void switchValve();
    unsigned char valveState = VALVE_CLOSED;


};


#endif //ARDUINOWLANSWITCH_VALVEHANDLER_H
