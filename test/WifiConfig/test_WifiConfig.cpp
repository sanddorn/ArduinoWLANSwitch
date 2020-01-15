//
// Created by nilsb on 07.01.20.
//

#include <Arduino.h>
#include <unity.h>

#include "test_WifiConfig.h"
#include <WifiConfigStorage.h>
#include <cstring>

using namespace fakeit;
namespace TestWifiConfig {

    static struct StorageData createWifiStorageMinimal() {
        struct StorageData wifiStorage{};
        memcpy(wifiStorage.configValid, "NB", 3);
        WifiStorage nullstorage{};
        memset(nullstorage.AccessPointName, '\0', ACCESSPOINT_NAME_LENGTH);
        memset(nullstorage.AccessPointPassword, '\0', WIFI_PASSWORD_LENGTH);
        for (int i : {0, 1, 2, 3, 4}) {
            wifiStorage.knownNets[i] = nullstorage;
        }
        return wifiStorage;
    }

    static const char *const SSID_FIRST_KNOWNNETWORK = "SSID";

    static struct StorageData createSingleFilledStorageData() {
        struct StorageData wifiStorage = createWifiStorageMinimal();
        memcpy(wifiStorage.knownNets[0].AccessPointName, SSID_FIRST_KNOWNNETWORK, 4);
        memcpy(wifiStorage.knownNets[0].AccessPointPassword, "PASSWORD", 8);
        wifiStorage.numberOfNets = 1;

        return wifiStorage;
    }

    static const char *const SSID_SECOND_KNOWNNETWORK = "SSID1";

    static struct StorageData createDoubleFilledStorageData() {
        struct StorageData wifiStorage = createSingleFilledStorageData();
        memcpy(wifiStorage.knownNets[1].AccessPointName, SSID_SECOND_KNOWNNETWORK, 4);
        memcpy(wifiStorage.knownNets[1].AccessPointPassword, "PASSWORD", 8);
        wifiStorage.numberOfNets = 2;

        return wifiStorage;
    }

    static WifiStorage createWifiStorage() {
        WifiStorage wifiStorage{};
        memcpy(wifiStorage.AccessPointName, "WIFI_SSID", 4);
        memcpy(wifiStorage.AccessPointPassword, "PASSWORD", 8);
        return wifiStorage;
    }

    WifiConfigStorage *subjectUnderTest;
    Logging logging;
    Mock<AbstractWifiStorage> storage;

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
        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        Verify(Method(storage, begin)).Once();
        Verify(Method(storage, get)).Once();
        VerifyNoOtherInvocations(storage);
    }

    void test_initStorageInvalidStorage() {
        storage.ClearInvocationHistory();
        struct StorageData wifiStorage = createWifiStorageMinimal();
        memcpy(wifiStorage.configValid, "NV", 3);
        When(Method(storage, begin)).Return();
        When(Method(storage, get)).AlwaysDo([wifiStorage](StorageData &result) {
            result = wifiStorage;
        });
        When(Method(storage, put)).Return();
        When(Method(storage, commit)).Return();
        try {
            subjectUnderTest->initStorage();
        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        Verify(Method(storage, begin)).Once();
        Verify(Method(storage, get)).Once();
        Verify(Method(storage, put)).Once();
        Verify(Method(storage, commit)).Once();
        VerifyNoOtherInvocations(storage);
    }


    void test_getSoftAPData() {
        storage.ClearInvocationHistory();
        struct StorageData wifiStorage = createWifiStorageMinimal();
        When(Method(storage, begin)).Return();
        When(Method(storage, get)).AlwaysDo([wifiStorage](StorageData &result) {
            result = wifiStorage;
        });
        WifiStorage resultStorage;
        try {
            subjectUnderTest->initStorage();
            resultStorage = subjectUnderTest->getSoftAPData();
        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_EQUAL_STRING(wifiStorage.fallback.AccessPointPassword, resultStorage.AccessPointPassword);
        TEST_ASSERT_EQUAL_STRING(wifiStorage.fallback.AccessPointName, resultStorage.AccessPointName);
    }

    void test_getApSSID() {
        storage.ClearInvocationHistory();
        struct StorageData wifiStorage = createSingleFilledStorageData();
        When(Method(storage, begin)).Return();
        When(Method(storage, get)).AlwaysDo([wifiStorage](StorageData &result) {
            result = wifiStorage;
        });
        char *ssid = nullptr;
        try {
            subjectUnderTest->initStorage();
            ssid = subjectUnderTest->getApSSID(0);
        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_EQUAL_STRING(wifiStorage.knownNets[0].AccessPointName, ssid);
    }


    void test_getNumberOfKnownNetworks() {
        storage.ClearInvocationHistory();
        struct StorageData wifiStorage = createDoubleFilledStorageData();
        When(Method(storage, begin)).Return();
        When(Method(storage, get)).AlwaysDo([wifiStorage](StorageData &result) {
            result = wifiStorage;
        });
        int noNetworks = 0;
        try {
            subjectUnderTest->initStorage();
            noNetworks = subjectUnderTest->getNumberOfKnownNetworks();
        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_EQUAL_INT(2, noNetworks);
    }

    void test_addWifiNetwork() {
        storage.ClearInvocationHistory();
        struct StorageData wifiStorage = createDoubleFilledStorageData();
        When(Method(storage, begin)).Return();
        When(Method(storage, get)).AlwaysDo([wifiStorage](StorageData &result) {
            result = wifiStorage;
        });
        Fake(Method(storage, put));
        When(Method(storage, commit)).Return(true);
        int noNetworks = 0;
        try {
            subjectUnderTest->initStorage();
            subjectUnderTest->addWifiNetwork(createWifiStorage());
            noNetworks = subjectUnderTest->getNumberOfKnownNetworks();
            Verify(Method(storage, put), Method(storage, commit)).Once();

        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_EQUAL_INT(3, noNetworks);
    }

    void test_addWifiNetwork_CommitToEEPROMFails() {
        storage.ClearInvocationHistory();
        struct StorageData wifiStorage = createDoubleFilledStorageData();
        When(Method(storage, begin)).Return();
        When(Method(storage, get)).AlwaysDo([wifiStorage](StorageData &result) {
            result = wifiStorage;
        });
        Fake(Method(storage, put));
        When(Method(storage, commit)).Return(false);
        int noNetworks = 0;
        try {
            subjectUnderTest->initStorage();
            subjectUnderTest->addWifiNetwork(createWifiStorage());
            noNetworks = subjectUnderTest->getNumberOfKnownNetworks();
            Verify(Method(storage, put), Method(storage, commit)).Once();

        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_EQUAL_INT(-1, noNetworks);
    }

    void test_removeWifiNetwork() {
        storage.ClearInvocationHistory();
        struct StorageData wifiStorage = createDoubleFilledStorageData();
        When(Method(storage, begin)).Return();
        When(Method(storage, get)).AlwaysDo([wifiStorage](StorageData &result) {
            result = wifiStorage;
        });
        Fake(Method(storage, put));
        When(Method(storage, commit)).Return(true);
        int noNetworks = 0;
        try {
            subjectUnderTest->initStorage();
            subjectUnderTest->removeWifiNetwork(SSID_FIRST_KNOWNNETWORK);
            noNetworks = subjectUnderTest->getNumberOfKnownNetworks();
            Verify(Method(storage, put), Method(storage, commit)).Once();

        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_EQUAL_INT(1, noNetworks);
    }

    void test_removeWifiNetwork_CommitToEEPROMFails() {
        storage.ClearInvocationHistory();
        struct StorageData wifiStorage = createDoubleFilledStorageData();
        When(Method(storage, begin)).Return();
        When(Method(storage, get)).AlwaysDo([wifiStorage](StorageData &result) {
            result = wifiStorage;
        });
        Fake(Method(storage, put));
        When(Method(storage, commit)).Return(false);
        int noNetworks = 0;
        try {
            subjectUnderTest->initStorage();
            subjectUnderTest->removeWifiNetwork(SSID_FIRST_KNOWNNETWORK);
            noNetworks = subjectUnderTest->getNumberOfKnownNetworks();
            Verify(Method(storage, put), Method(storage, commit)).Once();

        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_EQUAL_INT(-1, noNetworks);
    }

    void test_removeWifiNetwork_UnkownNetwork() {
        storage.ClearInvocationHistory();
        struct StorageData wifiStorage = createDoubleFilledStorageData();
        When(Method(storage, begin)).Return();
        When(Method(storage, get)).AlwaysDo([wifiStorage](StorageData &result) {
            result = wifiStorage;
        });
        int noNetworks = 0;
        try {
            subjectUnderTest->initStorage();
            subjectUnderTest->removeWifiNetwork("IRGENDWASFALSCHES");
            noNetworks = subjectUnderTest->getNumberOfKnownNetworks();

        } catch (UnexpectedMethodCallException &e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_EQUAL_INT(2, noNetworks);
    }

    void run_tests() {
        RUN_TEST(test_initStorage);
        RUN_TEST(test_initStorageInvalidStorage);
        RUN_TEST(test_getSoftAPData);
        RUN_TEST(test_getApSSID);
        RUN_TEST(test_getNumberOfKnownNetworks);
        RUN_TEST(test_addWifiNetwork);
        RUN_TEST(test_addWifiNetwork_CommitToEEPROMFails);
        RUN_TEST(test_removeWifiNetwork);
        RUN_TEST(test_removeWifiNetwork_CommitToEEPROMFails);
        RUN_TEST(test_removeWifiNetwork_UnkownNetwork);
    }
}