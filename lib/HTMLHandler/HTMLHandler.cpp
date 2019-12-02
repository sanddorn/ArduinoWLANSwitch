//
// Created by Nils Bokermann on 27.11.19.
//

#include "HTMLHandler.h"
#include <ESP8266WiFi.h>
#include <FS.h>


static String GetEncryptionType(byte thisType) {
    String output = "";
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

String HTMLHandler::getMainPage() {
    if (!spiffsStarted)
        return internalError;
    String mainpage;
    File mainpageFile = SPIFFS.open("/MainPage.html", "r");
    Serial.printf("SPIFFS.open returned File: %i\n", mainpageFile.isFile());
    if (mainpageFile.isFile() && mainpageFile.size() > 0) {
        Serial.printf("Returning Main-file\n");
        mainpage = mainpageFile.readString();
        mainpageFile.close();
        SPIFFS.end();

    } else {
        mainpage = internalError;
    }
    return mainpage;
}


String HTMLHandler::getCss() {
    if (!spiffsStarted)
        return internalError;
    String cssString;
    File cssFile = SPIFFS.open("/portal.css", "r");
    if (cssFile.isFile() && cssFile.size() > 0) {
        cssString = cssFile.readString();
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

void HTMLHandler::setSSID(const String &ssid) {
    this->ssid = ssid;
}

void HTMLHandler::setAPName(const String &apname) {
    this->apname = apname;
}

void HTMLHandler::setBSSID(const String &bssid) {
    this->bssid = bssid;
}

String HTMLHandler::getAPName() {
    return ssid.length() > 0 ? ssid : apname;
}

void HTMLHandler::addAvailableNetwork(const String &ssid, const uint8 encryption, int strength) {
    File partial = SPIFFS.open("/AvailableNetwork.partial", "r");
    String newAvalilableNetwork;
    if (partial.isFile() && partial.size() > 0) {
        newAvalilableNetwork = partial.readString();
        partial.close();
        Serial.printf("Partial-File Read\n");
    } else {
        newAvalilableNetwork = internalError;
    }
    newAvalilableNetwork.replace("<number/>", String(noNetwork));
    newAvalilableNetwork.replace("<ssid/>", ssid);
    newAvalilableNetwork.replace("<encryption/>", GetEncryptionType(encryption));
    newAvalilableNetwork.replace("<strength/>", String(strength));

    availableNetworks += newAvalilableNetwork;

    options += "<option value='" + ssid + "'>" + ssid + "</option>";
}

String HTMLHandler::getWifiPage() {
    if (!spiffsStarted)
        return internalError;
    String wifipage;
    File wifiPage = SPIFFS.open("/WifiPage.html", "r");
    if (wifiPage.isFile() && wifiPage.size() > 0) {
        wifipage = wifiPage.readString();
        wifiPage.close();
        Serial.printf("Wifi-File Read\n");
    } else {
        wifipage = internalError;
    }
    setCurrentWifiSettings();
    if (wifiAPMode) {
        wifipage.replace("ESP8266_Config_WLAN", apname);
    }
    wifipage.replace("<apmode/>", String(wifiAPMode));
    wifipage.replace("<currentWifiSettings/>", currentSettings);
    wifipage.replace("<availableNetworks/>", availableNetworks);
    if (options.length() > 0) {
        wifipage.replace("<networkOtions/>", options);
    } else {
        wifipage.replace("<networkOtions/>", "<option value='No_WiFi_Network'>No WiFiNetwork found </option>");
    }
    return wifipage;
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

String HTMLHandler::getSwitch(bool open) {
    if (!spiffsStarted)
        return internalError;
    String valvepage;
    File valvePageFile = SPIFFS.open("/Valve.html", "r");
    if (valvePageFile.isFile() && valvePageFile.size() > 0) {
        valvepage = valvePageFile.readString();
        valvePageFile.close();
        Serial.printf("Wifi-File Read\n");
    } else {
        valvepage = internalError;
    }
    if (open) {
        valvepage.replace("&#x274C; The valve is closed", "&#x274E; The valve is open");

    }
    return valvepage;
}

