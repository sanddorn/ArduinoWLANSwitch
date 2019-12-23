//
// Created by Nils Bokermann on 19.11.19.
//

#include "EEPromStorage.h"

#define DEBUG(x) Serial.printf(x)
#define DEBUG1(x, y) Serial.printf(x,y)

static const char *const ap_name = "ESP8266_Config_WLAN";

static const char *const default_password = "12345678";

int EEPromStorage::getNumberOfKnownNetworks() {
    return storageIsValid ? actualData.numberOfNets : -1;
}

void EEPromStorage::addWifiNetwork(WifiStorage newNetwork) {
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

void EEPromStorage::saveToEEPROM() {
    DEBUG("saveToEEPROM");
    EEPROM.put(0, actualData);
    if (EEPROM.commit()) {
        storageIsDirty = false;
        Serial.printf("EEPROM successfully updated\n");
    } else {
        Serial.printf("EEPROM could not be updated\n");
        storageIsValid = false;
    }
}

void EEPromStorage::initStorage() {
    EEPROM.begin(eeprom_size);
    EEPROM.get(0, actualData);
    if (memcmp("NB", actualData.configValid, 3) == 0) {
        storageIsValid = true;
        DEBUG("Storage was valid\n");
    } else {
        DEBUG("storate is invalid, generating default.");
        resetStorage();
        saveToEEPROM();
        storageIsValid = true;
    }
}


WifiStorage *EEPromStorage::retrieveNetwork(const char *const ssid) {
    DEBUG1("Retrieving info for ssid: %s\n", ssid);
    if (!storageIsValid) {
        DEBUG("Storage was invalid, initalizing\n");
        initStorage();
    }
    for (int i = 0; i < actualData.numberOfNets; i++) {
        DEBUG1("checking against %s\n", actualData.knownNets[i].AccessPointName);
        if (memcmp(actualData.knownNets[i].AccessPointName, ssid, MAX_NUMBER_OF_NETS)==0) {
            DEBUG("Network was found");
            return &actualData.knownNets[i];
        }
    }
    return nullptr;
}

EEPromStorage::EEPromStorage() : actualData{},  storageIsValid(false), storageIsDirty(false) {
    memcpy(actualData.configValid,"NV",3);
    WifiStorage nullstorage{};
    memset(nullstorage.AccessPointName, '\0', ACCESSPOINT_NAME_LENGTH);
    memset(nullstorage.AccessPointPassword, '\0', WIFI_PASSWORD_LENGTH);
    for (int i : {0,1,2,3,4}) {
        actualData.knownNets[i] = nullstorage;
    }
}

char * EEPromStorage::getApSSID(int i) {
    if (storageIsValid && i < actualData.numberOfNets) {
        return actualData.knownNets[i].AccessPointName;
    }
    return nullptr;
}

WifiStorage EEPromStorage::getSoftAPData() {
    if(!storageIsValid) {
        initStorage();
    }
    return actualData.fallback;
}

void EEPromStorage::removeWifiNetwork(const char *const ssid) {
    for (int i = 0; i < actualData.numberOfNets; i++) {
        if (strncmp(actualData.knownNets[i].AccessPointName, ssid, MAX_NUMBER_OF_NETS) == 0) {
            // Move all following members down
            int j = i + 1;
            while (j < actualData.numberOfNets) {
                actualData.knownNets[j - 1] = actualData.knownNets[j];
            }
            actualData.numberOfNets--;
            return;
        }
    }
}

void EEPromStorage::resetStorage() {
    actualData.numberOfNets = 0;
    strncpy(actualData.configValid, "NB", 3);
    actualData.numberOfNets = 0;
    actualData.numberOfNets=0;
    strncpy(actualData.fallback.AccessPointName, ap_name, ACCESSPOINT_NAME_LENGTH);
    strncpy(actualData.fallback.AccessPointPassword, default_password, WIFI_PASSWORD_LENGTH);

}

