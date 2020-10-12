//
// Created by nilsb on 07.01.20.
//

#ifndef ESP8266_WLAN_SCHALTER_ABSTRACT_WIFI_STORAGE_H
#define ESP8266_WLAN_SCHALTER_ABSTRACT_WIFI_STORAGE_H
#include <memory>

#define MAX_NUMBER_OF_NETS 5

#define WIFI_PASSWORD_LENGTH 63
#define ACCESSPOINT_NAME_LENGTH 32

static const int storage_size = 6*(WIFI_PASSWORD_LENGTH+ACCESSPOINT_NAME_LENGTH+2)+3;

typedef struct _WifiStorage {
    char AccessPointName[ACCESSPOINT_NAME_LENGTH];
    char AccessPointPassword[WIFI_PASSWORD_LENGTH];
} WifiStorage;

struct StorageData {
    WifiStorage fallback;
    int numberOfNets;
    WifiStorage knownNets[MAX_NUMBER_OF_NETS];
    char configValid[3];
};

class AbstractWifiStorage {
public:
    virtual void begin() = 0;
    virtual bool commit() = 0;
    virtual void end() = 0;

    virtual void get(StorageData &t) = 0;
    virtual void put(const StorageData &t) = 0;
};

#endif //ESP8266_WLAN_SCHALTER_ABSTRACT_WIFI_STORAGE_H
