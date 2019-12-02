//
// Created by Nils Bokermann on 27.11.19.
//

#include "HTMLHandler.h"
#include <ESP8266WiFi.h>
#include <FS.h>


static string GetEncryptionType(byte thisType) {
    string output = "";
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
        SPIFFS.end();

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

void HTMLHandler::setWiFiAPMode(bool isSoftAP) {
    Serial.printf("setting wifiAPMode: %i\n", isSoftAP);
    wifiAPMode = isSoftAP ? 0 : 1;
    Serial.printf("wifiAPMode is now: %i\n", wifiAPMode);
}

void HTMLHandler::setCurrentWifiSettings() {
    Serial.printf("wifiAPMode is for currentWifiSettings: %i\n", wifiAPMode);
    if (wifiAPMode == 0) {
        currentSettings = "<div class='currentMode'>Mode : Soft Access Point (AP)</div>\n";
    } else {
        currentSettings = "<div class='currentMode'>Mode : Station (STA)</div>\n";
    }
    currentSettings += "<div class='currentInfo'>SSID  :  " + getAPName() + "</div>\n";
    if (bssid.length() > 0) {
        currentSettings += "<div class='currentInfo'>BSSID :  " + bssid + "</div>\n";
    }
}

void HTMLHandler::setSSID(const string &ssid) {
    this->ssid = ssid;
}

void HTMLHandler::setAPName(const string &apname) {
    this->apname = apname;
}

void HTMLHandler::setBSSID(const string &bssid) {
    this->bssid = bssid;
}

string HTMLHandler::getAPName() {
    return ssid.length() > 0 ? ssid : apname;
}

void HTMLHandler::addAvailableNetwork(const string &ssid, const uint8 encryption, int strength) {
    File partial = SPIFFS.open("/AvailableNetwork.partial", "r");
    string newAvalilableNetwork;
    if (partial.isFile() && partial.size() > 0) {
        newAvalilableNetwork = partial.readString().c_str();
        partial.close();
        Serial.printf("Partial-File Read\n");
    } else {
        newAvalilableNetwork = internalError;
    }
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
    setCurrentWifiSettings();
    if (wifiAPMode) {
        replaceString(wifiPage, "ESP8266_Config_WLAN", apname);

    }
    replaceString(wifiPage, "<apmode/>", wifiAPMode == 0 ? "0" : "1");
    replaceString(wifiPage, "<currentWifiSettings/>", currentSettings);
    replaceString(wifiPage, "<availableNetworks/>", availableNetworks);
    if (options.length() > 0) {
        replaceString(wifiPage, "<networkOtions/>", options);
    } else {
        replaceString(wifiPage, "<networkOtions/>", "<option value='No_WiFi_Network'>No WiFiNetwork found </option>");
    }
    return wifiPage;
}

HTMLHandler::HTMLHandler() : wifiAPMode(0), noNetwork(0) {
    spiffsStarted = SPIFFS.begin();
    Serial.printf("SPIFFS started: '%i'\n", spiffsStarted);
}

HTMLHandler::~HTMLHandler() {
    SPIFFS.end();
}

void HTMLHandler::resetWifiPage() {
    options = "";
    availableNetworks = "";
    currentSettings = "";
    ssid = "";
    bssid = "";
    apname = "";
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
        Serial.printf("Wifi-File Read\n");
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
    size_t end = start + toReplace.length();
    original.replace(start, end, replacement);
}

