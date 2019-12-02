//
// Created by Nils Bokermann on 19.11.19.
//

#include "EEPromStorage.h"

int EEPromStorage::getNumberOfKnownNetworks() {
    return storageIsValid ? actualData->numberOfNets : -1;
}

void EEPromStorage::addWifiNetwork(WifiStorage newNetwork) {
    if (!storageIsValid) {
        actualData = new StorageData;
        strncpy(actualData->configValid, "NB", 3);
        actualData->numberOfNets=0;
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
}

WifiStorage *  EEPromStorage::retrieveNetwork(const char * const ssid) {
    return nullptr;
}