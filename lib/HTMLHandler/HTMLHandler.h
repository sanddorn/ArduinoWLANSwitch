//
// Created by Nils Bokermann on 27.11.19.
//

#ifndef ARDUINOWLANSWITCH_HTMLHANDLER_H
#define ARDUINOWLANSWITCH_HTMLHANDLER_H

#include <string>

#define MEDIATYPE_TEXT_HTML "text/html"
#define MEDIATYPE_TEXT_PLAIN "text/plain"
using namespace std;
class HTMLHandler {
public:
    HTMLHandler();
    ~HTMLHandler();
    string getMainPage();
    string getCss();
    string getWifiPage();
    void resetWifiPage();
    string getSwitch(bool on);
    void setWiFiAPMode(bool isSoftAP);
    void addAvailableNetwork(const string  &ssid, uint8_t encryption, int strength);
    void setSSID(const string &ssid);
    void setBSSID(const string &bssid);
    void setAPName(const string &apname);
protected:
    void replaceString(string &original, const string &toReplace, const string &replacement);
private:
    const string internalError = "Internal Error. Cannot deliver page.";
    void setCurrentWifiSettings();
    string getAPName();
    int wifiAPMode;
    string options;
    string availableNetworks;
    string currentSettings;
    string ssid;
    string bssid;
    string apname;
    int noNetwork;
    bool spiffsStarted;
};


#endif //ARDUINOWLANSWITCH_HTMLHANDLER_H
