//
// Created by nilsb on 07.01.20.
//

#include "test_WifiConfig.h"
#include "../../lib/WifiConfig/AbstractWifiStorage.h"
#include <WifiConfigStorage.h>
#include <string.h>


namespace TestWifiConfig {
    WifiConfigStorage *subjectUnderTest;
    Logging logging;
    Mock <AbstractWifiStorage> storage;
    struct StorageData wifiStorage;

    void setUp() {
        memcpy(wifiStorage.configValid, "NB", 3);
        WifiStorage nullstorage{};
        memset(nullstorage.AccessPointName, '\0', ACCESSPOINT_NAME_LENGTH);
        memset(nullstorage.AccessPointPassword, '\0', WIFI_PASSWORD_LENGTH);
        for (int i : {0, 1, 2, 3, 4}) {
            wifiStorage.knownNets[i] = nullstorage;
        }
        AbstractWifiStorage &persistence = storage.get();
        subjectUnderTest = new WifiConfigStorage(&persistence, &logging);
    }


    void test_initStorage() {
        When(Method(storage, begin)).Return();
        When(Method(storage, get)).AlwaysDo([](auto& t) {
            result = wifiStorage
            return true;
        });
        subjectUnderTest->initStorage();
    }

    void run_tests() {
        RUN_TEST(test_getMainPage);
    }
}