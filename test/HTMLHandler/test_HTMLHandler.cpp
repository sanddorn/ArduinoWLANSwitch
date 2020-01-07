//
// Created by Nils Bokermann on 29.11.19.
//


#include <string>
#include "test_HTMLHandler.h"

using namespace fakeit;
using namespace std;

class MockStorage : public StorageBlob {
public:
    MockStorage(std::string storage) : store(storage) {};

    virtual ~MockStorage() = default;

    virtual size_t size() const { return store.length(); };

    virtual void close() {};

    virtual bool isFile() const { return true; };

    virtual std::string readString() { return store; };

private:
    std::string store;
};

namespace TestHtmlHandler {
    HTMLHandler *subjectUnderTest;
    Logging logging;
    Mock <FilePersistence> persistenceMock;

    void setUp() {
        FilePersistence &persistence = persistenceMock.get();
        When(Method(persistenceMock, begin)).Return(true);
        subjectUnderTest = new HTMLHandler(&persistence, &logging);
    }

    void test_getMainPage() {
        persistenceMock.ClearInvocationHistory();
        string original = "Mainpage_Mock";
        std::shared_ptr <MockStorage> storage(new MockStorage(original));
        Mock <MockStorage> fileMock(*storage);
        Spy(Method(fileMock, isFile));
        Spy(Method(fileMock, size));
        Spy(Method(fileMock, readString));
        When(Method(persistenceMock, open)).Return(storage);
        string mainpage;
        try {
            mainpage = subjectUnderTest->getMainPage();
            TEST_ASSERT_TRUE(original.compare(mainpage) == 0);
            Verify(Method(persistenceMock, open).Using("/MainPage.html", "r")).Once();
            Verify(Method(fileMock, isFile)).Exactly(2);
            Verify(Method(fileMock, size)).Once();
            Verify(Method(fileMock, readString)).Once();
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
    }

    void test_getCSS() {
        persistenceMock.ClearInvocationHistory();
        string original = "CSS";
        std::shared_ptr <MockStorage> storage(new MockStorage(original));
        Mock <MockStorage> fileMock(*storage);
        Spy(Method(fileMock, isFile));
        Spy(Method(fileMock, size));
        Spy(Method(fileMock, readString));
        When(Method(persistenceMock, open)).Return(storage);
        string mainpage;
        try {
            mainpage = subjectUnderTest->getCss();
            Verify(Method(persistenceMock, open).Using("/portal.css", "r")).Once();
            Verify(Method(fileMock, isFile)).Exactly(2);
            Verify(Method(fileMock, size)).Once();
            Verify(Method(fileMock, readString)).Once();
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_TRUE(original.compare(mainpage) == 0);
    }

    void test_getWifiSaveDonePage() {
        persistenceMock.ClearInvocationHistory();
        string original = "WifiSaveDone";
        std::shared_ptr <MockStorage> storage(new MockStorage(original));
        Mock <MockStorage> fileMock(*storage);
        Spy(Method(fileMock, isFile));
        Spy(Method(fileMock, size));
        Spy(Method(fileMock, readString));
        When(Method(persistenceMock, open)).Return(storage);
        string mainpage;
        try {
            mainpage = subjectUnderTest->getWifiSaveDonePage();
            Verify(Method(persistenceMock, open).Using("/WifiSetupDone.html", "r")).Once();
            Verify(Method(fileMock, isFile)).Exactly(2);
            Verify(Method(fileMock, size)).Once();
            Verify(Method(fileMock, readString)).Once();
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_TRUE(original.compare(mainpage) == 0);
    }

    void test_addAvailableNetwork() {
        persistenceMock.ClearInvocationHistory();
        string wifiString = "<availableNetworks/>";
        string partial = "Number: '<number/>' SSID: '<ssid/>' encryption: '<encryption/>' strength: '<strength/>'";
        std::shared_ptr <MockStorage> partialFile(new MockStorage(partial));
        std::shared_ptr <MockStorage> wifiFile(new MockStorage(wifiString));
        When(Method(persistenceMock, open).Using("/AvailableNetwork.partial", "r")).Return(partialFile);
        When(Method(persistenceMock, open).Using("/WifiPage.html", "r")).Return(wifiFile);
        string mainpage;
        try {
            subjectUnderTest->addAvailableNetwork("ssid", 4, 42);
            mainpage = subjectUnderTest->getWifiPage();
            subjectUnderTest->resetWifiPage();
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_TRUE(mainpage.compare("Number: '1' SSID: 'ssid' encryption: 'WPA2' strength: '42'") == 0);
    }

    void test_addAvailableNetworkWithTwoNetworks() {
        persistenceMock.ClearInvocationHistory();
        string wifiString = "<availableNetworks/>";
        string partial = "Number: '<number/>' SSID: '<ssid/>' encryption: '<encryption/>' strength: '<strength/>'";
        std::shared_ptr <MockStorage> partialFile(new MockStorage(partial));
        std::shared_ptr <MockStorage> wifiFile(new MockStorage(wifiString));
        When(Method(persistenceMock, open).Using("/AvailableNetwork.partial", "r")).AlwaysReturn(partialFile);
        When(Method(persistenceMock, open).Using("/WifiPage.html", "r")).Return(wifiFile);
        string mainpage;
        try {
            subjectUnderTest->addAvailableNetwork("ssid", 4, 42);
            subjectUnderTest->addAvailableNetwork("ssid2", 4, 43);
            mainpage = subjectUnderTest->getWifiPage();
            subjectUnderTest->resetWifiPage();
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_TRUE(mainpage.compare(
                "Number: '1' SSID: 'ssid' encryption: 'WPA2' strength: '42'Number: '2' SSID: 'ssid2' encryption: 'WPA2' strength: '43'") ==
                         0);
    }

    void test_addAvailableNetworkOptions() {
        persistenceMock.ClearInvocationHistory();
        string wifiString = "<networkOptions/>";
        string partial = "Number: '<number/>' SSID: '<ssid/>' encryption: '<encryption/>' strength: '<strength/>'";
        std::shared_ptr <MockStorage> partialFile(new MockStorage(partial));
        std::shared_ptr <MockStorage> wifiFile(new MockStorage(wifiString));
        When(Method(persistenceMock, open).Using("/AvailableNetwork.partial", "r")).AlwaysReturn(partialFile);
        When(Method(persistenceMock, open).Using("/WifiPage.html", "r")).Return(wifiFile);
        string mainpage;
        try {
            subjectUnderTest->addAvailableNetwork("ssid", 4, 42);
            subjectUnderTest->addAvailableNetwork("ssid2", 4, 43);
            mainpage = subjectUnderTest->getWifiPage();
            subjectUnderTest->resetWifiPage();
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_TRUE(mainpage.compare(
                "<option value=''>No WiFiNetwork</option><option value='ssid'>ssid</option><option value='ssid2'>ssid2</option>") ==
                         0);
    }

    void test_addConfiguredNetworks() {
        persistenceMock.ClearInvocationHistory();
        string wifiString = "<configuredNetworks/>";
        string partial = "SSID: '<ssid/>' ";
        std::shared_ptr <MockStorage> partialFile(new MockStorage(partial));
        std::shared_ptr <MockStorage> wifiFile(new MockStorage(wifiString));
        When(Method(persistenceMock, open).Using("/RegisteredNetwork.partial", "r")).AlwaysReturn(partialFile);
        When(Method(persistenceMock, open).Using("/WifiPage.html", "r")).Return(wifiFile);
        string mainpage;
        try {
            subjectUnderTest->addRegisteredNetwork("ssid");
            mainpage = subjectUnderTest->getWifiPage();
            subjectUnderTest->resetWifiPage();
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_TRUE(mainpage.compare("SSID: 'ssid' ") == 0);
    }


    void test_softAPCredentials() {
        persistenceMock.ClearInvocationHistory();
        string wifiString = "SSID: '<softapssid>' Password: '<softappassword>";
        std::shared_ptr <MockStorage> wifiFile(new MockStorage(wifiString));
        When(Method(persistenceMock, open).Using("/WifiPage.html", "r")).Return(wifiFile);
        string mainpage;
        try {
            subjectUnderTest->setSoftAPCredentials("ssid", "password");
            mainpage = subjectUnderTest->getWifiPage();
            subjectUnderTest->resetWifiPage();
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_TRUE(mainpage.compare("SSID: 'ssid' Password: 'password") == 0);
    }

    void test_getSwitchOpen() {
        persistenceMock.ClearInvocationHistory();
        string webPage = "&#x274C; The valve is closed";
        std::shared_ptr <MockStorage> storage(new MockStorage(webPage));
        When(Method(persistenceMock, open)).Return(storage);
        string mainpage;
        try {
            mainpage = subjectUnderTest->getSwitch(true);
            subjectUnderTest->resetWifiPage();
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_TRUE(mainpage.compare("&#x274E; The valve is open") == 0);
    }

    void test_getSwitchClosed() {
        persistenceMock.ClearInvocationHistory();
        string webPage = "&#x274C; The valve is closed";
        std::shared_ptr <MockStorage> storage(new MockStorage(webPage));
        When(Method(persistenceMock, open)).Return(storage);
        string mainpage;
        try {
            mainpage = subjectUnderTest->getSwitch(false);
            subjectUnderTest->resetWifiPage();
        } catch (UnexpectedMethodCallException e) {
            TEST_FAIL_MESSAGE(e.what().c_str());
        }
        TEST_ASSERT_TRUE(mainpage.compare("&#x274C; The valve is closed") == 0);
    }

    void run_tests() {
        RUN_TEST(test_getMainPage);
        RUN_TEST(test_getCSS);
        RUN_TEST(test_getWifiSaveDonePage);
        RUN_TEST(test_addAvailableNetwork);
        RUN_TEST(test_addAvailableNetworkWithTwoNetworks);
        RUN_TEST(test_addAvailableNetworkOptions);
        RUN_TEST(test_addConfiguredNetworks);
        RUN_TEST(test_softAPCredentials);
        RUN_TEST(test_getSwitchOpen);
        RUN_TEST(test_getSwitchClosed);
    }
}
