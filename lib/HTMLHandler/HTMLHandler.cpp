//
// Created by Nils Bokermann on 27.11.19.
//

#include "HTMLHandler.h"
#include <Arduino.h>

#ifdef UNIT_TEST
#include <arduino/noniso.h>
#endif


/* TODO: Externalize dependency to ESP8266WiFi (GetEncryptionType) */
enum wl_enc_type {  /* Values map to 802.11 encryption suites... */
    ENC_TYPE_WEP = 5,
    ENC_TYPE_TKIP = 2,
    ENC_TYPE_CCMP = 4,
    /* ... except these two, 7 and 8 are reserved in 802.11-2007 */
            ENC_TYPE_NONE = 7,
    ENC_TYPE_AUTO = 8
};

static string GetEncryptionType(unsigned char thisType) {
    string output;
    // read the encryption type and print out the name:
    switch (thisType) {
        case ENC_TYPE_WEP:
            output = "WEP";
            break;
        case ENC_TYPE_TKIP:
            output = "WPA";
            break;
        case ENC_TYPE_CCMP:
            output = "WPA2";
            break;
        case ENC_TYPE_NONE:
            output = "None";
            break;
        case ENC_TYPE_AUTO:
            output = "Auto";
            break;
        default:
            output = "Unknown";
    }
    return output;
}


HTMLHandler::HTMLHandler(Persistence *persistence, Logging *log) : options("<option value=''>No WiFiNetwork</option>"),
                                                                   noNetwork(0),
                                                                   persistence(persistence),
                                                                   _log(log) {
    spiffsStarted = persistence->begin();
    _log->trace("SPIFFS started: '%i'", spiffsStarted);
}

HTMLHandler::~HTMLHandler() {
    persistence->end();
}

string HTMLHandler::getMainPage() {
    const char *path = "/MainPage.html";
    return getStaticPage(path);
}

string HTMLHandler::getWifiSaveDonePage() {
    const char *path = "/WifiSetupDone.html";
    return getStaticPage(path);
}

string HTMLHandler::getStaticPage(const char *path) const {
    if (!spiffsStarted)
        return internalError;
    string mainpage;
    std::shared_ptr<StorageBlob> mainpageFile = persistence->open(path, "r");
    _log->trace("persistence.open returned File: %i", mainpageFile->isFile());
    if (mainpageFile->isFile() && mainpageFile->size() > 0) {
        mainpage = mainpageFile->readString();
        mainpageFile->close();
    } else {
        mainpage = internalError;
    }
    return mainpage;
}

string HTMLHandler::getCss() {
    const char *path = "/portal.css";
    return getStaticPage(path);
}

void HTMLHandler::addRegisteredNetwork(const string &ssid) {
    const char *path = "/RegisteredNetwork.partial";
    string newRegisteredNetword = getStaticPage(path);
    replaceString(newRegisteredNetword, "<ssid/>", ssid);
    registeredNetwork += newRegisteredNetword;

}

void HTMLHandler::addAvailableNetwork(const string &ssid, const unsigned char encryption, int strength) {
    if (ssid.length() < 2) {
        return;
    }

    const char *path = "/AvailableNetwork.partial";
    string newAvalilableNetwork = getStaticPage(path);
    _log->trace("adding Network '%s'", ssid.c_str());
    char tmp[10];
    replaceString(newAvalilableNetwork, "<number/>", itoa(++noNetwork, tmp, 10));
    replaceString(newAvalilableNetwork, "<ssid/>", ssid);
    replaceString(newAvalilableNetwork, "<encryption/>", GetEncryptionType(encryption));
    replaceString(newAvalilableNetwork, "<strength/>", itoa(strength, tmp, 10));

    availableNetworks += newAvalilableNetwork;

    options += "<option value='" + ssid + "'>" + ssid + "</option>";
}

string HTMLHandler::getWifiPage() {
    if (!spiffsStarted)
        return internalError;

    const char *path = "/WifiPage.html";
    string wifiPage = getStaticPage(path);
    replaceString(wifiPage, "<configuredNetworks/>", registeredNetwork);
    replaceString(wifiPage, "<availableNetworks/>", availableNetworks);
    replaceString(wifiPage, "<networkOptions/>", options);
    replaceString(wifiPage, "<softapssid>", softAP_SSID);
    replaceString(wifiPage, "<softappassword>", softAP_password);
    return wifiPage;
}

void HTMLHandler::resetWifiPage() {
    options = "<option value=''>No WiFiNetwork</option>";
    availableNetworks = "";
    registeredNetwork = "";
    noNetwork = 0;

}

string HTMLHandler::getSwitch(bool open) {
    if (!spiffsStarted)
        return internalError;

    const char *path = "/Valve.html";
    string valvepage = getStaticPage(path);
    if (open) {
        replaceString(valvepage, "&#x274C; The valve is closed", "&#x274E; The valve is open");
    }
    return valvepage;
}

void HTMLHandler::replaceString(string &original, const string &toReplace, const string &replacement) {
    size_t start = original.find(toReplace);
    while (start != string::npos) {
        size_t len = toReplace.length();
        original.replace(start, len, replacement);
        start = original.find(toReplace);
    }
}

void HTMLHandler::setSoftAPCredentials(const string &ssid, const string &password) {
    softAP_SSID = ssid;
    softAP_password = password;
}

