//
// Created by Nils Bokermann on 19.11.19.
//

#include "WifiConfigStorage.h"

#include <Arduino.h>


static const char *const ap_name = "ESP8266_Config_WLAN";

static const char *const default_password = "12345678";

int WifiConfigStorage::getNumberOfKnownNetworks() {
    return storageIsValid ? actualData.numberOfNets : -1;
}

void WifiConfigStorage::addWifiNetwork(WifiStorage newNetwork) {
    if (!storageIsValid) {
        initStorage();
    }
    if (actualData.numberOfNets < MAX_NUMBER_OF_NETS) {
        memcpy(actualData.knownNets[actualData.numberOfNets].AccessPointName, newNetwork.AccessPointName,
               ACCESSPOINT_NAME_LENGTH);
        memcpy(actualData.knownNets[actualData.numberOfNets].AccessPointPassword, newNetwork.AccessPointPassword,
               WIFI_PASSWORD_LENGTH);
        actualData.numberOfNets++;
        storageIsDirty = true;
    }
    saveToEEPROM();
}

void WifiConfigStorage::saveToEEPROM() {
    log->trace("saveToEEPROM");
    storage->put(actualData);
    if (storage->commit()) {
        storageIsDirty = false;
        log->trace("EEPROM successfully updated");
    } else {
        log->trace("EEPROM could not be updated");
        storageIsValid = false;
    }
}

void WifiConfigStorage::initStorage() {
    storage->begin();
    storage->get(actualData);
    if (memcmp("NB", actualData.configValid, 3) == 0) {
        storageIsValid = true;
        log->trace("Storage was valid\n");
    } else {
        log->trace("storate is invalid, generating default.");
        resetStorage();
        saveToEEPROM();
        storageIsValid = true;
    }
}


WifiStorage *WifiConfigStorage::retrieveNetwork(const char *const ssid) {
    log->trace("Retrieving info for ssid: %s\n", ssid);
    if (!storageIsValid) {
        log->trace("Storage was invalid, initalizing\n");
        initStorage();
    }
    for (int i = 0; i < actualData.numberOfNets; i++) {
        log->trace("checking against %s\n", actualData.knownNets[i].AccessPointName);
        if (memcmp(actualData.knownNets[i].AccessPointName, ssid, MAX_NUMBER_OF_NETS) == 0) {
            log->trace("Network was found\n");
            return &actualData.knownNets[i];
        }
    }
    return nullptr;
}

WifiConfigStorage::WifiConfigStorage(Logging * logger, AbstractWifiStorage * abstractStorage) : storageIsValid(false), storageIsDirty(false), actualData{}, log(logger), storage(abstractStorage){
    memcpy(actualData.configValid, "NV", 3);
    WifiStorage nullstorage{};
    memset(nullstorage.AccessPointName, '\0', ACCESSPOINT_NAME_LENGTH);
    memset(nullstorage.AccessPointPassword, '\0', WIFI_PASSWORD_LENGTH);
    for (int i : {0, 1, 2, 3, 4}) {
        actualData.knownNets[i] = nullstorage;
    }
    actualData.numberOfNets=0;
}

WifiConfigStorage::~WifiConfigStorage() {
    storage->end();
}

char *WifiConfigStorage::getApSSID(int i) {
    if (storageIsValid && i < actualData.numberOfNets) {
        return actualData.knownNets[i].AccessPointName;
    }
    return nullptr;
}

WifiStorage WifiConfigStorage::getSoftAPData() {
    if (!storageIsValid) {
        initStorage();
    }
    return actualData.fallback;
}

void WifiConfigStorage::removeWifiNetwork(const char *const ssid) {
    for (int i = 0; i < actualData.numberOfNets; i++) {
        if (strncmp(actualData.knownNets[i].AccessPointName, ssid, MAX_NUMBER_OF_NETS) == 0) {
            // Move all following members down
            log->trace("Removing entry for ssid: %s", actualData.knownNets[i].AccessPointName);
            int j = i + 1;
            while (j < actualData.numberOfNets) {
                actualData.knownNets[j - 1] = actualData.knownNets[j];
            }
            actualData.numberOfNets--;
            return;
        }
    }
}

void WifiConfigStorage::resetStorage() {
    actualData.numberOfNets = 0;
    strncpy(actualData.configValid, "NB", 3);
    actualData.numberOfNets = 0;
    actualData.numberOfNets = 0;
    strncpy(actualData.fallback.AccessPointName, ap_name, ACCESSPOINT_NAME_LENGTH);
    strncpy(actualData.fallback.AccessPointPassword, default_password, WIFI_PASSWORD_LENGTH);
}

