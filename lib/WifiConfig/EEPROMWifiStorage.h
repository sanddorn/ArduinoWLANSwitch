//
// Created by nilsb on 07.01.20.
//

#ifndef ESP8266_WLAN_SCHALTER_EEPROMWIFISTORAGE_H
#define ESP8266_WLAN_SCHALTER_EEPROMWIFISTORAGE_H
#ifndef UNIT_TEST
#include "AbstractWifiStorage.h"
#include <EEPROM.h>

class EEPROMWifiStorage : public AbstractWifiStorage {
public:
    void begin() override;

    bool commit() override;

    void end() override;

    void get(StorageData &t) override;

    void put(const StorageData &t) override;

};
#endif //UNIT_TEST

#endif //ESP8266_WLAN_SCHALTER_EEPROMWIFISTORAGE_H
