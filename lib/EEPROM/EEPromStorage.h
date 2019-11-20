//
// Created by Nils Bokermann on 19.11.19.
//

#ifndef ESP8266_WLAN_SCHALTER_EEPROMSTORAGE_H
#define ESP8266_WLAN_SCHALTER_EEPROMSTORAGE_H

#define WIFI_PASSWORD_LENGTH 25
#define ACCESSPOINT_NAME_LENGTH 20

#include <Arduino.h>
#include <EEPROM.h>

class EEPromStorage {

};


extern "C" {


struct WiFiEEPromData {
    bool APSTA;
    bool PwDReq;
    bool CapPortal;
    char APSTAName[ACCESSPOINT_NAME_LENGTH]; // STATION /AP Point Name TO cONNECT, if definded
    char WiFiPwd[WIFI_PASSWORD_LENGTH]; // WiFiPAssword, if definded
    char ConfigValid[3]; //If Config is Vaild, Tag "TK" is required"
};
}

#endif //ESP8266_WLAN_SCHALTER_EEPROMSTORAGE_H
