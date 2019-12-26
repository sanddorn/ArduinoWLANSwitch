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
    string getWifiSaveDonePage();
    string getCss();
    string getWifiPage();
    void resetWifiPage();
    string getSwitch(bool on);
    void addAvailableNetwork(const string  &ssid, uint8_t encryption, int strength);
    void addRegisteredNetwork(const string &ssid);
    void setSoftAPCredentials(const string &ssid);
protected:
    void replaceString(string &original, const string &toReplace, const string &replacement);
private:
    const string internalError = "Internal Error. Cannot deliver page.";
    string options;
    string availableNetworks;
    string softAP_SSID;
    int noNetwork;
    bool spiffsStarted;
    string registeredNetwork;

    string getStaticPage(const char *path) const;
};


#endif //ARDUINOWLANSWITCH_HTMLHANDLER_H
