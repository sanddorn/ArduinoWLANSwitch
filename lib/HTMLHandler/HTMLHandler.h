//
// Created by Nils Bokermann on 27.11.19.
//

#ifndef ARDUINOWLANSWITCH_HTMLHANDLER_H
#define ARDUINOWLANSWITCH_HTMLHANDLER_H

#include <WString.h>
#include <Arduino.h>

#define MAINPAGE "<!DOCTYPE HTML>\n<html lang='de'>\n<head>\n    <title>Captive Portal</title>\n    <meta charset='UTF-8'>\n    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n    <link rel=\"stylesheet\" href=\"/portal.css\">\n</head>\n<body>\n<h2>Captive Portal</h2>\n<h3>Sytemlinks:</h3>\n<table class=\"systemlinks\">\n    <tr>\n        <th><a href='/wifi'>WIFI Einstellungen</a></th>\n    </tr>\n</table>\n</body>\n</html>"
#define WIFIPAGE "<!DOCTYPE HTML>\n<html lang='de'>\n<head>\n    <meta charset='UTF-8'>\n    <meta name=viewport content='width=device-width, initial-scale=1.0'>\n    <link rel=\"stylesheet\" href=\"/portal.css\">\n</head>\n    <title>Wifi Switch</title>\n</head>\n<body>\n<h2>WiFi Einstellungen</h2>\n<div class=\"currentSettings\">\n    <h4>Current WiFi Settings:</h4>\n   <currentWifiSettings/> \n</div>\n<br>\n<form action='/wifi' method='post'>\n    <div class=\"networks\">\n        <div class=\"heading\">\n            <label class=\"centered\">\n                <input type='radio' value='1' name='WiFiMode'>\n            </label> WiFi Station Mode\n        </div>\n        <div class=\"heading\">Available WiFi Networks:</div>\n        <table>\n            <tr>\n                <th>Number</th>\n                <th>SSID</th>\n                <th>Encryption</th>\n                <th>WiFi Strength</th>\n            </tr>\n          <availableNetworks/>\n        </table>\n        <table>\n            <tr>\n                <td>Connect to WiFi SSID:</td>\n                <td><select name='WiFi_Network'>\n                    <networkOtions/>\n                </select></td>\n            </tr>\n            <tr>\n                <td>WiFi Password:</td>\n                <td><label>\n                    <input type='text' name='STAWLanPW' maxlength='40' size='40'>\n                </label></td>\n            </tr>\n        </table>\n    </div>\n    <div class=\"networks\">\n        <div class=\"heading\">\n            <label>\n                <input type='radio' value='2' name='WiFiMode'>\n            </label>WiFi Access Point Mode\n        </div>\n        <table>\n            <tr>\n                <td>WiFi Access Point Name:</td>\n                <td><input type='text' name='APPointName' maxlength='19' size='30' value='ESP8266_Config_WLAN'>\n                </td>\n            </tr>\n            <tr>\n                <td>WiFi Password:</td>\n                <td><input type='password' name='APPW' maxlength='24' size='30' value='12345678'></td>\n            </tr>\n            <tr>\n                <td>Repeat WiFi Password:</td>\n                <td><input type='password' name='APPWRepeat' maxlength='24' size='30' value='12345678'></td>\n            </tr>\n        </table>\n        <div>\n            <label>\n                <input type='checkbox' name='PasswordReq' checked>\n            </label> Password for Login required.\n        </div>\n        <div class=\"buttonbox\">\n            <button type='submit' name='Settings' value='1' autofocus>Set WiFi\n                Settings\n            </button>\n            <button type='submit' name='Reboot' value='1'>Reboot System</button>\n            <button type='reset' name='action' value='1'>Reset Inputs</button>\n        </div>\n    </div>\n</form>\n<h3>Sytemlinks:</h3>\n<table class=\"systemlinks\">\n    <tr>\n        <th><a href='/wifi'>WIFI Einstellungen</a></th>\n    </tr>\n</table>\n</body>\n</html>"
#define PORTAL_CSS "button {\n    height: 35px;\n    font-size: 16px\n}\n\ntable {\n    border: 2pt ridge;\n    background-color: white;\n    padding: 5pt;\n    width: 100%;\n}\n\ntd {\n    border: 2pt ridge;\n}\n\nbody {\n    background-color: powderblue;\n}\n\n.currentSettings {\n    border: 2pt ridge;\n    horiz-align: center;\n    align-content: flex-start;\n    background-color: darkseagreen;\n    width: 500pt;\n}\n\n.currentMode {\n    horiz-align: center;\n}\n\n.currentInfo {\n    horiz-align: center;\n}\n\n.networks {\n    border: 2pt ridge;\n    horiz-align: center;\n    align-content: flex-start;\n    background-color: white;\n    width: 500pt;\n}\n\n.heading {\n    font-weight: bold;\n}\n.buttonbox {\n    padding: 5px;\n}\n.systemlinks {\n    border: 2pt;\n    border-style: ridge;\n    background-color: white;\n    width: 500pt;\n    padding: 5pt;\n}"
#define AVALABLE_NETWORK_PARTIAL "<tr><td><number/></td>\n    <td><ssid/></td>\n    <td><encryption/></td>\n    <td><strenght/></td>\n</tr>"


class HTMLHandler {
public:
    HTMLHandler();
    ~HTMLHandler() = default;
    static String getMainPage();
    String getWifiPage();
    void wifiSetAPMode(boolean isSoftAP);
    void addAvailableNetwork(const String  &ssid, uint8 encryption, int strength);
    void setSSID(const String &ssid);
    void setBSSID(const String &bssid);
    void setAPName(const String &apname);
private:
    void setCurrentWifiSettings();
    int wifiAPMode;
    String options;
    String availableNetworks;
    String currentSettings;
    String ssid;
    String bssid;
    String apname;
    int noNetwork;
};


#endif //ARDUINOWLANSWITCH_HTMLHANDLER_H
