//
// Created by Nils Bokermann on 19.01.20.
//

#ifndef ESP8266_WLAN_SCHALTER_ABSTRACTCALLBACKHANDLER_H
#define ESP8266_WLAN_SCHALTER_ABSTRACTCALLBACKHANDLER_H

class AbstractCallBackHandler {
public:
    virtual ~AbstractCallBackHandler() = default;

    virtual void startCallbackTimer(long millis, void (*callbackFunction)(void *), void *payload) = 0;
};

#endif //ESP8266_WLAN_SCHALTER_ABSTRACTCALLBACKHANDLER_H
