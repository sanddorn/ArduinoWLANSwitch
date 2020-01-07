//
// Created by Nils Bokermann on 27.11.19.
//

#ifndef ARDUINOWLANSWITCH_HTMLHANDLER_H
#define ARDUINOWLANSWITCH_HTMLHANDLER_H

#include <string>
#include <ArduinoLog.h>
#include "AbstractPersistence.h"

#define MEDIATYPE_TEXT_HTML "text/html"
#define MEDIATYPE_TEXT_PLAIN "text/plain"

using namespace std;

class HTMLHandler {
public:
    HTMLHandler(FilePersistence *persit, Logging *log);

    ~HTMLHandler();

    string getMainPage();

    string getWifiSaveDonePage();

    string getCss();

    string getWifiPage();

    void resetWifiPage();

    string getSwitch(bool on);

    void addAvailableNetwork(const string &ssid, unsigned char encryption, int strength);

    void addRegisteredNetwork(const string &ssid);

    void setSoftAPCredentials(const string &ssid, const string &password);

protected:
    static void replaceString(string &original, const string &toReplace, const string &replacement);

private:
    const string internalError = "Internal Error. Cannot deliver page.";
    string options;
    string availableNetworks;
    string softAP_SSID;
    string softAP_password;
    int noNetwork;
    bool spiffsStarted;
    string registeredNetwork;

    string getStaticPage(const char *path) const;

    FilePersistence *persistence;
    Logging *_log;
};


#endif //ARDUINOWLANSWITCH_HTMLHANDLER_H
