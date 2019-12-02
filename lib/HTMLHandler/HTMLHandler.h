//
// Created by Nils Bokermann on 27.11.19.
//

#ifndef ARDUINOWLANSWITCH_HTMLHANDLER_H
#define ARDUINOWLANSWITCH_HTMLHANDLER_H

#include <WString.h>

#define MEDIATYPE_TEXT_HTML "text/html"
#define MEDIATYPE_TEXT_PLAIN "text/plain"

class HTMLHandler {
public:
    HTMLHandler();
    ~HTMLHandler();
    String getMainPage();
    String getCss();
    String getWifiPage();
    void resetWifiPage();
    String getSwitch(bool on);
    void setWiFiAPMode(bool isSoftAP);
    void addAvailableNetwork(const String  &ssid, uint8 encryption, int strength);
    void setSSID(const String &ssid);
    void setBSSID(const String &bssid);
    void setAPName(const String &apname);
private:
    const String internalError = "Internal Error. Cannot deliver page.";
    void setCurrentWifiSettings();
    String getAPName();
    int wifiAPMode;
    String options;
    String availableNetworks;
    String currentSettings;
    String ssid;
    String bssid;
    String apname;
    int noNetwork;
    bool spiffsStarted;
};


#endif //ARDUINOWLANSWITCH_HTMLHANDLER_H
