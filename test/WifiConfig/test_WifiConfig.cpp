//
// Created by nilsb on 07.01.20.
//

#include <Arduino.h>
#include <unity.h>

#include "test_WifiConfig.h"
#include "../../lib/WifiConfig/AbstractWifiStorage.h"
#include <WifiConfigStorage.h>
#include <cstring>

using namespace fakeit;
namespace TestWifiConfig {

    static struct StorageData createWifiStorageMinimal() {
        struct StorageData wifiStorage;
        memcpy(wifiStorage.configValid, "NB", 3);
        WifiStorage nullstorage{};
        memset(nullstorage.AccessPointName, '\0', ACCESSPOINT_NAME_LENGTH);
        memset(nullstorage.AccessPointPassword, '\0', WIFI_PASSWORD_LENGTH);
        for (int i : {0, 1, 2, 3, 4}) {
            wifiStorage.knownNets[i] = nullstorage;
        }
        return wifiStorage;
    }

    WifiConfigStorage *subjectUnderTest;
    Logging logging;
    Mock <AbstractWifiStorage> storage;

    void setUp() {
        AbstractWifiStorage &persistence = storage.get();
        subjectUnderTest = new WifiConfigStorage(&logging, &persistence);
    }


    void test_initStorage() {
        storage.ClearInvocationHistory();
        struct StorageData wifiStorage = createWifiStorageMinimal();
        When(Method(storage, begin)).Return();
        When(Method(storage, get)).AlwaysDo([wifiStorage](StorageData &result) {
            result = wifiStorage;
        });
        try {
            subjectUnderTest->initStorage();
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        Verify(Method(storage, begin)).Once();
        Verify(Method(storage, get)).Once();
        VerifyNoOtherInvocations(storage);
    }

    void test_initStorageInvalidStorage() {
        storage.ClearInvocationHistory();
        struct StorageData wifiStorage = createWifiStorageMinimal();
        memcpy(wifiStorage.configValid, "NV",3);
        When(Method(storage, begin)).Return();
        When(Method(storage, get)).AlwaysDo([wifiStorage](StorageData &result) {
            result = wifiStorage;
        });
        When(Method(storage, put)).Return();
        When(Method(storage, commit)).Return();
        try {
            subjectUnderTest->initStorage();
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        Verify(Method(storage, begin)).Once();
        Verify(Method(storage, get)).Once();
        Verify(Method(storage, put)).Once();
        Verify(Method(storage, commit)).Once();
        VerifyNoOtherInvocations(storage);
    }

    void test_resetStorage() {
        storage.ClearInvocationHistory();
        try {
            subjectUnderTest->resetStorage();
            TEST_ASSERT_EQUAL_STRING( "ESP8266_Config_WLAN", subjectUnderTest->getSoftAPData().AccessPointName);
            TEST_ASSERT_EQUAL_STRING( "12345678", subjectUnderTest->getSoftAPData().AccessPointPassword);
            TEST_ASSERT_EQUAL(0, subjectUnderTest->getNumberOfKnownNetworks());
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }

    }

    void run_tests() {
        RUN_TEST(test_initStorage);
        RUN_TEST(test_initStorageInvalidStorage);
        RUN_TEST(test_resetStorage);
    }
}