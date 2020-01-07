//
// Created by nilsb on 07.01.20.
//

#include "EEPROMWifiStorage.h"

void EEPROMWifiStorage::begin() {
    EEPROM.begin(storage_size);

}

bool EEPROMWifiStorage::commit() {
    return EEPROM.commit();
}

void EEPROMWifiStorage::end() {
    EEPROM.end();
}

void EEPROMWifiStorage::get(StorageData &t) {
    EEPROM.get(0, t);
}

void EEPROMWifiStorage::put(const StorageData &t) {
    EEPROM.put(0,t);
}
