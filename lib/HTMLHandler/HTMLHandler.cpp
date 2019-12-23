//
// Created by Nils Bokermann on 27.11.19.
//

#include "HTMLHandler.h"
#include <ESP8266WiFi.h>
#include <FS.h>


static string GetEncryptionType(byte thisType) {
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

string HTMLHandler::getMainPage() {
    if (!spiffsStarted)
        return internalError;
    string mainpage;
    File mainpageFile = SPIFFS.open("/MainPage.html", "r");
    Serial.printf("SPIFFS.open returned File: %i\n", mainpageFile.isFile());
    if (mainpageFile.isFile() && mainpageFile.size() > 0) {
        Serial.printf("Returning Main-file\n");
        mainpage = mainpageFile.readString().c_str();
        mainpageFile.close();
    } else {
        mainpage = internalError;
    }
    return mainpage;
}


string HTMLHandler::getCss() {
    if (!spiffsStarted)
        return internalError;
    string cssString;
    File cssFile = SPIFFS.open("/portal.css", "r");
    if (cssFile.isFile() && cssFile.size() > 0) {
        cssString = cssFile.readString().c_str();
        cssFile.close();
        Serial.printf("CSS-File Read\n");
    } else {
        cssString = "";
    }
    return cssString;

}

void HTMLHandler::addRegisteredNetwork(const string &ssid) {
    File partial = SPIFFS.open("/RegisteredNetwork.partial", "r");
    string newRegisteredNetword;
    if (partial.isFile() && partial.size() > 0) {
        newRegisteredNetword = partial.readString().c_str();
        partial.close();
        Serial.printf("Partial-File Read\n");
    } else {
        newRegisteredNetword = internalError;
    }
    replaceString(newRegisteredNetword, "<ssid/>", ssid);
    registeredNetwork += newRegisteredNetword;

}

void HTMLHandler::addAvailableNetwork(const string &ssid, const uint8 encryption, int strength) {
    if (ssid.length() < 2) {
        return;
    }
    File partial = SPIFFS.open("/AvailableNetwork.partial", "r");
    string newAvalilableNetwork;
    if (partial.isFile() && partial.size() > 0) {
        newAvalilableNetwork = partial.readString().c_str();
        partial.close();
        Serial.printf("Partial-File Read\n");
    } else {
        newAvalilableNetwork = internalError;
    }
    Serial.printf("adding Network '%s'\n", ssid.c_str());
    char tmp[10];
    replaceString(newAvalilableNetwork, "<number/>", itoa(noNetwork, tmp, 10));
    replaceString(newAvalilableNetwork, "<ssid/>", ssid);
    replaceString(newAvalilableNetwork, "<encryption/>", GetEncryptionType(encryption));
    replaceString(newAvalilableNetwork, "<strength/>", itoa(strength, tmp, 10));

    availableNetworks += newAvalilableNetwork;

    options += "<option value='" + ssid + "'>" + ssid + "</option>";
}

string HTMLHandler::getWifiPage() {
    if (!spiffsStarted)
        return internalError;
    string wifiPage;
    File wifiPageFile = SPIFFS.open("/WifiPage.html", "r");
    if (wifiPageFile.isFile() && wifiPageFile.size() > 0) {
        wifiPage = wifiPageFile.readString().c_str();
        wifiPageFile.close();
        Serial.printf("Wifi-File Read\n");
    } else {
        wifiPage = internalError;
    }
    replaceString(wifiPage, "<configuredNetworks/>", registeredNetwork);
    replaceString(wifiPage, "<availableNetworks/>", availableNetworks);
    if (options.length() > 0) {
        replaceString(wifiPage, "<networkOtions/>", options);
    } else {
        replaceString(wifiPage, "<networkOtions/>", "<option value='No_WiFi_Network'>No WiFiNetwork found </option>");
    }
    return wifiPage;
}

HTMLHandler::HTMLHandler() : noNetwork(0) {
    fs::SPIFFSConfig cfg;
    cfg.setAutoFormat(false);
    SPIFFS.setConfig(cfg);
    spiffsStarted = SPIFFS.begin();
    Serial.printf("SPIFFS started: '%i'\n", spiffsStarted);
}

HTMLHandler::~HTMLHandler() {
    SPIFFS.end();
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
    string valvepage;
    File valvePageFile = SPIFFS.open("/Valve.html", "r");
    if (valvePageFile.isFile() && valvePageFile.size() > 0) {
        valvepage = valvePageFile.readString().c_str();
        valvePageFile.close();
        Serial.printf("Valve-File Read\n");
    } else {
        valvepage = internalError;
    }
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

