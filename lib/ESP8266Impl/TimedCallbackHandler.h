//
// Created by Nils Bokermann on 21.01.20.
//

#ifndef ESP8266_WLAN_SCHALTER_TIMEDCALLBACKHANDLER_H
#define ESP8266_WLAN_SCHALTER_TIMEDCALLBACKHANDLER_H

#include <AbstractCallbackHandler.h>
#include <ArduinoLog.h>
#include <memory>
#include "Ticker.h"


class TimedCallbackHandler : public AbstractCallBackHandler {
public :
    static std::shared_ptr<TimedCallbackHandler> getInstance();
    void startCallbackTimer(long millis, void (*function)(void*), void* arg) override;
    void callbackHit();
    ~TimedCallbackHandler() override = default;
private:
    TimedCallbackHandler() : payload(nullptr), callbackFunction(nullptr) {
        logging.begin(LOG_LEVEL_TRACE, &Serial);
        logging.setPrefix([](Print *p){p->print("TimedCallBackHandler: ");});
        logging.setSuffix([](Print *p){p->print(CR);});
    };

    void * payload;
    void (*callbackFunction)(void*);
    static std::shared_ptr<TimedCallbackHandler> callbackHandler;
    Ticker ticker;
    Logging logging;

};


#endif //ESP8266_WLAN_SCHALTER_TIMEDCALLBACKHANDLER_H
