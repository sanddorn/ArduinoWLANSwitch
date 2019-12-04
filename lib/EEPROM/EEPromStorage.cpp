//
// Created by Nils Bokermann on 19.11.19.
//

#include "EEPromStorage.h"

#define DEBUG(x) Serial.printf(x)
#define DEBUG1(x,y) Serial.printf(x,y)

static const char *const ap_name = "ESP8266_Config_WLAN";


int EEPromStorage::getNumberOfKnownNetworks() {
    return storageIsValid ? actualData->numberOfNets : -1;
}

void EEPromStorage::addWifiNetwork(WifiStorage newNetwork) {
    if (!storageIsValid) {
        actualData = new StorageData;
        strncpy(actualData->configValid, "NB", 3);
        actualData->numberOfNets = 0;
        storageIsValid = true;
    }
    if (actualData->numberOfNets < MAX_NUMBER_OF_NETS) {
        memcpy(actualData->knownNets[actualData->numberOfNets].AccessPointName, newNetwork.AccessPointName,
               ACCESSPOINT_NAME_LENGTH);
        memcpy(actualData->knownNets[actualData->numberOfNets].AccessPointPassword, newNetwork.AccessPointPassword,
               WIFI_PASSWORD_LENGTH);
        actualData->numberOfNets++;
        storageIsDirty = true;
    }
    EEPROM.put(0, *actualData);
    if (EEPROM.commit()) {
        storageIsDirty = false;
    } else {
        storageIsValid = false;
    }
}


WifiStorage *EEPromStorage::retrieveNetwork(const char *const ssid) {
    for (int i = 0; i < MAX_NUMBER_OF_NETS; i++) {
        if (memcmp(actualData->knownNets[i].AccessPointName, ssid, MAX_NUMBER_OF_NETS)==0) {
            return &actualData->knownNets[i];
        }
    }
    return nullptr;
}

EEPromStorage::EEPromStorage() : storageIsValid(false), storageIsDirty(false), actualData(nullptr) {
    DEBUG1("sizeof data: %i\n", sizeof(actualData));
    EEPROM.begin(eeprom_size);
    EEPROM.get(0, *actualData);
    DEBUG("EEPROM Read.\n");
    if (actualData != nullptr && memcmp("NB", actualData->configValid, 3) == 0) {
        storageIsValid = true;
        DEBUG("Storage was valid\n");
    } else {
        actualData->numberOfNets=0;
        strncpy(actualData->fallback.AccessPointName, ap_name, ACCESSPOINT_NAME_LENGTH);
        strncpy(actualData->fallback.AccessPointPassword, "12345678", WIFI_PASSWORD_LENGTH);
    }
}

char * EEPromStorage::getApSSID(int i) {
    if (storageIsValid && i < actualData->numberOfNets) {
        return actualData->knownNets[i].AccessPointName;
    }
    return nullptr;
}

WifiStorage EEPromStorage::getSoftAPData() {
    return actualData->fallback;
}

void EEPromStorage::removeWifiNetwork(String &string) {
// TODO Implement
}
