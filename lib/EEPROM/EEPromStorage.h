//
// Created by Nils Bokermann on 19.11.19.
//

#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H

#define WIFI_PASSWORD_LENGTH 25
#define ACCESSPOINT_NAME_LENGTH 20

#include <Arduino.h>
#include <EEPROM.h>
#include <string>

#define MAX_NUMBER_OF_NETS 5
typedef struct _WifiStorage {
    char AccessPointName[ACCESSPOINT_NAME_LENGTH];
    char AccessPointPassword[WIFI_PASSWORD_LENGTH];
} WifiStorage;


static const int eeprom_size = 512;

class EEPromStorage {
public:
    EEPromStorage();

    ~EEPromStorage() {
        EEPROM.end();
    };

    int getNumberOfKnownNetworks();

    void addWifiNetwork(WifiStorage newNetwork);

    WifiStorage *retrieveNetwork(const char *ssid);

    WifiStorage getSoftAPData();

    char *getApSSID(int number);

    void removeWifiNetwork(const char * ssid);

private:
    struct StorageData {
        WifiStorage fallback;
        int numberOfNets;
        WifiStorage knownNets[MAX_NUMBER_OF_NETS];
        char configValid[3];
    };
    bool storageIsValid;
    bool storageIsDirty;
    struct StorageData *actualData;
};

struct WiFiEEPromData {
    bool AccessPointMode;
    char APSTAName[ACCESSPOINT_NAME_LENGTH]; // STATION /AP Point Name TO cONNECT, if definded
    char WiFiPwd[WIFI_PASSWORD_LENGTH]; // WiFiPAssword, if definded
    char ConfigValid[3]; //If Config is Vaild, Tag "TK" is required"
};


#endif //EEPROM_STORAGE_H
