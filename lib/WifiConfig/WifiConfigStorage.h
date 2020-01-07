//
// Created by Nils Bokermann on 19.11.19.
//

#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H


#include <string>
#include <ArduinoLog.h>
#include "AbstractWifiStorage.h"

class WifiConfigStorage {
public:
    WifiConfigStorage(Logging * logger, AbstractWifiStorage * storage);

    ~WifiConfigStorage();

    int getNumberOfKnownNetworks();

    void addWifiNetwork(WifiStorage newNetwork);

    WifiStorage *retrieveNetwork(const char *ssid);

    WifiStorage getSoftAPData();

    char *getApSSID(int number);

    void removeWifiNetwork(const char * ssid);

    void initStorage();

    void resetStorage();

private:
    bool storageIsValid;
    bool storageIsDirty;
    struct StorageData actualData;
    Logging *log;
    AbstractWifiStorage * storage;

    void saveToEEPROM();
};

struct WiFiEEPromData {
    bool AccessPointMode;
    char APSTAName[ACCESSPOINT_NAME_LENGTH]; // STATION /AP Point Name TO cONNECT, if definded
    char WiFiPwd[WIFI_PASSWORD_LENGTH]; // WiFiPAssword, if definded
    char ConfigValid[3]; //If Config is Vaild, Tag "TK" is required"
};


#endif //EEPROM_STORAGE_H
