#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include "../lib/EEPROM/EEPromStorage.h"
#include "WebFiles.h"


#define BLINK_LED D5
/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *ESPHostname = "ESP";

// DNS server
const byte DNS_PORT = 53;
static const char *const ap_name = "ESP8266_Config_WLAN";
DNSServer dnsServer;

//Conmmon Paramenters
bool SoftAccOK = false;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(172, 20, 0, 1);
IPAddress netMsk(255, 255, 255, 0);

WiFiEEPromData MyWiFiConfig;

boolean reconnect;

boolean loadCredentials();

boolean CreateWifiSoftAP();

byte ConnectWifiAP();

void SetDefaultWiFiConfig();

boolean saveCredentials();

void InitalizeHTTPServer();

void handleRoot();

void handleWifi();

void handleCss();

void handleNotFound();

void configureWifi();

void doReboot();

void startWifi();

void setup() {
    pinMode(BLINK_LED, OUTPUT); // Initialize the BUILTIN_LED1 pin as an output
    Serial.begin(9600);
    Serial.println("booting");
    WiFi.hostname(ESPHostname); // Set the DHCP hostname assigned to ESP station.
    Serial.setDebugOutput(true); //Debug Output for WLAN on Serial Interface.
    startWifi();
}

void startWifi() {
    boolean ConnectSuccess = false;
    byte len;
    WiFi.disconnect();
    /* Function will set currently configured SSID and password of the soft-AP to null values. The parameter  is optional. If set to true it will switch the soft-AP mode off.*/
    WiFi.softAPdisconnect(true);
    if (loadCredentials()) // Load WLAN credentials for WiFi Settings
    {
        // Valid Credentials found.
        if (MyWiFiConfig.AccessPointMode)  // AP Mode
        {
            Serial.println("AP Mode");
            ConnectSuccess = CreateWifiSoftAP();
        } else {
            Serial.println("STA Mode");
            len = ConnectWifiAP();
            ConnectSuccess = (len == 3);
        }
    } else { //Set default Config - Create AP
        Serial.println("DefaultWiFi Cnf");
        SetDefaultWiFiConfig();
        ConnectSuccess = CreateWifiSoftAP();
        saveCredentials();
    }
    if (ConnectSuccess) {
        Serial.println("Wifi Setup Succes");
    } else {
        Serial.println("No SoftAP or STA could be started. Resetting to default");
        SetDefaultWiFiConfig();
        saveCredentials();
        CreateWifiSoftAP();
    }
    InitalizeHTTPServer();
}

void InitalizeHTTPServer() {
    /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
    server.on("/", handleRoot);
    server.on("/wifi", handleWifi);
    server.on("/portal.css", handleCss);
    server.onNotFound(handleNotFound);
    server.begin(); // Web server start
}

boolean CreateWifiSoftAP() {
    Serial.print("SoftAP ");
    Serial.printf("AP Settings: '%s' Passwd: '%s'\n", MyWiFiConfig.APSTAName, MyWiFiConfig.WiFiPwd);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    SoftAccOK = WiFi.softAP(MyWiFiConfig.APSTAName, MyWiFiConfig.WiFiPwd); // Passwortlänge mindestens 8 Zeichen !
    delay(600); // Without delay I've seen the IP address blank
    if (SoftAccOK) {
        Serial.println("OK");
        Serial.println(MyWiFiConfig.APSTAName);
        Serial.println(MyWiFiConfig.WiFiPwd);
        /* Setup the DNS server redirecting all the domains to the apIP */
        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(DNS_PORT, "*", apIP);
    } else {
        Serial.println("err");
        Serial.println(MyWiFiConfig.APSTAName);
        Serial.println(MyWiFiConfig.WiFiPwd);
    }
    return SoftAccOK;
}


byte ConnectWifiAP() {
    // Serial.println("Initalizing Wifi Client.");
    byte connRes = 0;
    byte i = 0;
    WiFi.begin(MyWiFiConfig.APSTAName, MyWiFiConfig.WiFiPwd);
    connRes = WiFi.waitForConnectResult();
    delay(500);
    while ((connRes == 0) and (i != 10))  //if connRes == 0  "IDLE_STATUS - change Statius"
    {
        connRes = WiFi.waitForConnectResult();
        delay(1000);
        i++;
        Serial.println(".");
        // statement(s)
    }
    while ((connRes == 1) and (i != 10))  //if connRes == 1  NO_SSID_AVAILin - SSID cannot be reached
    {
        connRes = WiFi.waitForConnectResult();
        delay(1000);
        i++;
        Serial.println(".");
        // statement(s)
    }
    if (connRes == 3) {
        Serial.print("STA ");
        WiFi.setAutoReconnect(
                true); // Set whether module will attempt to reconnect to an access point in case it is disconnected.
        // Setup MDNS responder
        if (!MDNS.begin(ESPHostname)) {
            Serial.println("Err: MDNS");
        } else {
            MDNS.addService("http", "tcp", 80);
        }
        server.stop();
        server.begin();
    }
    if (connRes == 4) {
        Serial.println("STA Pwd Err");
        Serial.println(MyWiFiConfig.APSTAName);
        Serial.println(MyWiFiConfig.WiFiPwd);
        WiFi.disconnect();
    }
    // if (connRes == 6 ) { Serial.println("DISCONNECTED - Not in station mode"); }
    // WiFi.printDiag(Serial);
    return connRes;
}

boolean loadCredentials() {
    boolean retValue;
    EEPROM.begin(512);
    EEPROM.get(0, MyWiFiConfig);
    EEPROM.end();
    retValue = String("TK").equals(String(MyWiFiConfig.ConfigValid)) != 0;
    return retValue;
}


/** Store WLAN credentials to EEPROM */

boolean saveCredentials() {
    if (MyWiFiConfig.AccessPointMode == true) {
        if (sizeof(String(MyWiFiConfig.WiFiPwd)) < 8) {
            return false;  // Invalid Config
        }
        if (sizeof(String(MyWiFiConfig.APSTAName)) < 1) {
            return false;  // Invalid Config
        }
    }
    // End Check logical Errors
    EEPROM.begin(512);
    for (int i = 0; i < (int) sizeof(MyWiFiConfig); i++) {
        EEPROM.write(i, 0);
    }
    strncpy(MyWiFiConfig.ConfigValid, "TK", sizeof(MyWiFiConfig.ConfigValid));
    EEPROM.put(0, MyWiFiConfig);
    EEPROM.commit();
    EEPROM.end();
    return true;
}

void SetDefaultWiFiConfig() {
    byte len;
    MyWiFiConfig.AccessPointMode = true;
    strncpy(MyWiFiConfig.APSTAName, ap_name, sizeof(MyWiFiConfig.APSTAName));
    len = strlen(MyWiFiConfig.APSTAName);
    MyWiFiConfig.APSTAName[len + 1] = '\0';
    strncpy(MyWiFiConfig.WiFiPwd, "12345678", sizeof(MyWiFiConfig.WiFiPwd)); // no password
    len = strlen(MyWiFiConfig.WiFiPwd);
    MyWiFiConfig.WiFiPwd[len + 1] = '\0';
    strncpy(MyWiFiConfig.ConfigValid, "TK", sizeof(MyWiFiConfig.ConfigValid));
    len = strlen(MyWiFiConfig.ConfigValid);
    MyWiFiConfig.ConfigValid[len + 1] = '\0';
    Serial.println("RstWiFiCrd");
}

/** Is this an IP? */
boolean isIp(const String &str) {
    for (unsigned int i = 0; i < str.length(); i++) {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9')) {
            return false;
        }
    }
    return true;
}

String GetEncryptionType(byte thisType) {
    String Output = "";
    // read the encryption type and print out the name:
    switch (thisType) {
        case ENC_TYPE_WEP:
            Output = "WEP";
            return Output;
            break;
        case ENC_TYPE_TKIP:
            Output = "WPA";
            return Output;
            break;
        case ENC_TYPE_CCMP:
            Output = "WPA2";
            return Output;
            break;
        case ENC_TYPE_NONE:
            Output = "None";
            return Output;
            break;
        case ENC_TYPE_AUTO:
            Output = "Auto";
            return Output;
            break;
        default:
            Output = "Unknown";
            return Output;
    }
}

void handleRoot() {
    //  Main Page:
    // HTML Header
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    // HTML Content
    server.setContentLength(strlen(MAINPAGE));
    server.send(200, "text/html", MAINPAGE);
}


void handleNotFound() {
    // Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + server.client().localIP().toString(), true);
    server.send(302, "text/plain",
                "Redirect");
}

/** Wifi config page handler */
void handleWifi() {
    //  Page: /wifi
    String webPage = "";
    String replacement = "";
    // Check for Site Parameters
    // Reboot System
    if (server.hasArg("Reboot")) {
        webPage = "Rebooting System in 5 Seconds..";
        server.send(200, "text/plain", webPage);
        delay(5000);
        server.client().stop();
        doReboot();
        return;
    }
    if (server.hasArg("WiFiMode")) {
        configureWifi();
        return;
    }
    // HTML Header
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    // HTML Content
    webPage = WIFIPAGE;

    if (server.client().localIP() == apIP) {
        replacement = "<div class='currentMode'>Mode : Soft Access Point (AP)</div>\n";
        replacement += "<div class='currentInfo'>SSID : " + String(MyWiFiConfig.APSTAName) + "</div>\n";

    } else {
        replacement = "<div class='currentMode'>Mode : Station (STA)</div>\n";
        replacement += "<div class='currentInfo'>SSID  :  " + String(MyWiFiConfig.APSTAName) + "</div>\n";
        replacement += "<div class='currentInfo'>BSSID :  " + WiFi.BSSIDstr() + "</div>\n";
    }
    webPage.replace("<currentWifiSettings/>", replacement);


    if (MyWiFiConfig.AccessPointMode == 1) {
        webPage.replace("<apmode/>", "1");
    } else {
        webPage.replace("<apmode/>", "0");
    }

    WiFi.scanDelete();
    replacement = "";
    int scannedNetworks = WiFi.scanNetworks(); //WiFi.scanNetworks(async, show_hidden)
    if (scannedNetworks > 0) {
        for (int j = 0; j < scannedNetworks; j++) {
            char tmp[160];
            sprintf(tmp, AVALABLE_NETWORK_PARTIAL, j, WiFi.SSID(j).c_str(),
                    GetEncryptionType(WiFi.encryptionType(j)).c_str(), WiFi.RSSI(j));
            replacement += tmp;
        }
    } else {
        char tmp[160];
        sprintf(tmp, AVALABLE_NETWORK_PARTIAL, 1, "No WLAN found", " --- ", 0);
        replacement += tmp;
    }
    webPage.replace("<availableNetworks/>", replacement);

    replacement = "";
    if (scannedNetworks > 0) {
        for (int j = 0; j < scannedNetworks; j++) {
            replacement += "<option value='" + WiFi.SSID(j) + "'>" + WiFi.SSID(j) + "</option>";
            // <networkOtions/>
        }
    } else {
        replacement += "<option value='No_WiFi_Network'>No WiFiNetwork found </option>";
    }
    webPage.replace("<networkOtions/>", replacement);
    if (MyWiFiConfig.AccessPointMode == true) {
        webPage.replace("ESP8266_Config_WLAN", MyWiFiConfig.APSTAName);
    } else {
        webPage.replace("ESP8266_Config_WLAN", "");
    }
    server.setContentLength(webPage.length());
    server.send(200, "text/html", webPage);
}


void handleCss() {
    server.setContentLength(strlen(PORTAL_CSS));
    server.send(200, "text/css", PORTAL_CSS);
}

void doReboot() {
    WiFi.disconnect();
    delay(1000);
    pinMode(BLINK_LED, OUTPUT);
    digitalWrite(BLINK_LED, LOW);
}

void configureWifi() {
    String temp = "";
    // STA Station Mode Connect to another WIFI Station

    if ((server.arg("WiFiMode") == "1")) {

        // Connect to existing STATION
        if (server.arg("WiFi_Network").length() > 0) {
            Serial.println("Configuring STA Mode");
            MyWiFiConfig.AccessPointMode = false; // Access Point or Station Mode - false Station Mode
            memset(MyWiFiConfig.APSTAName, '\0', ACCESSPOINT_NAME_LENGTH);

            String networkName = server.arg("WiFi_Network");
            unsigned int networkNamelength = networkName.length();
            for (unsigned int i = 0; i < networkNamelength; i++) {
                MyWiFiConfig.APSTAName[i] = networkName[i];
            }
            memset(MyWiFiConfig.WiFiPwd, '\0', WIFI_PASSWORD_LENGTH);
            String serverPassword = server.arg("STAWLanPW");
            unsigned int passwordLength = serverPassword.length();
            for (unsigned int i = 0; i < passwordLength; i++) {
                if (serverPassword[i] > 32) //Steuerzeichen raus
                {
                    MyWiFiConfig.WiFiPwd[i] = serverPassword[i];
                }
            }
            //    MyWiFiConfig.WiFiPwd[len+1] = '\0';
            temp = "WiFi Connect to AP: -";
            temp += MyWiFiConfig.APSTAName;
            temp += "-<br>WiFi PW: -";
            temp += MyWiFiConfig.WiFiPwd;
            temp += "-<br>";
            temp += "Connecting to STA Mode in 2 Seconds..<br>";
            if (saveCredentials()) // Save AP ConfigCongfig
            {
                temp += "Daten des STA Modes erfolgreich gespeichert. ";
            } else {
                temp += "Daten des STA Modes fehlerhaft.";
            }
            server.send(200, "text/html", temp);
            server.sendContent(temp);
            delay(2000);
            server.client().stop();
            server.stop();
            temp = "";

            delay(500);
            // ConnectWifiAP
            int wifiConnectResult = ConnectWifiAP();

            // 4: WL_CONNECT_FAILED - Password is incorrect 1: WL_NO_SSID_AVAILin - Configured SSID cannot be reached
            if (wifiConnectResult != 3) {
                Serial.printf("Err STA Result: %i", wifiConnectResult);
                server.client().stop();
                delay(100);
                WiFi.setAutoReconnect(false);
                delay(100);
                WiFi.disconnect();
                delay(1000);
                CreateWifiSoftAP();
            } else {
                // Safe Config
                InitalizeHTTPServer();
            }
        }
        return;
    } else {
        unsigned int passwordLength;
        // Configure Access Point
        String apName = server.arg("APPointName");
        unsigned int apNameLength = apName.length();
        String password = server.arg("APPW");
        passwordLength = password.length();

        if ((apNameLength > 1) and (password == server.arg("APPWRepeat")) and (passwordLength > 7)) {
            temp = "";
            Serial.println("Configuring APMode");
            MyWiFiConfig.AccessPointMode = true; // Access Point or Sation Mode - true AP Mode

            // reset AP Name
            memset(MyWiFiConfig.APSTAName, '\0', ACCESSPOINT_NAME_LENGTH);

            for (unsigned int i = 0; i < apNameLength; i++) {
                MyWiFiConfig.APSTAName[i] = apName[i];
            }

            memset(MyWiFiConfig.WiFiPwd, '\0', WIFI_PASSWORD_LENGTH);
            for (unsigned int i = 0; i < passwordLength; i++) {
                MyWiFiConfig.WiFiPwd[i] = password[i];
            }
            if (saveCredentials()) // Save AP ConfigCongfig
            {
                temp = "Daten des AP Modes erfolgreich gespeichert. Reboot notwendig.";
                temp += "AccessPointName: ";
                temp += MyWiFiConfig.APSTAName;
                temp += "\nPassword: ";
                temp += MyWiFiConfig.WiFiPwd;
                temp += "\n";
            } else {
                temp = "Daten des AP Modes fehlerhaft.";
            }
            server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
            server.sendHeader("Pragma", "no-cache");
            server.sendHeader("Expires", "-1");
            server.setContentLength(temp.length());
            server.send(200, "text/plain", temp);
            Serial.println(temp);
            reconnect = true;
            return;
        } else if (server.arg("APPW") != server.arg("APPWRepeat")) {
            temp = "WLAN Passwort nicht gleich. Abgebrochen.";
        } else {
            temp = "WLAN Passwort oder AP Name zu kurz. Abgebrochen.";
        }
        if (temp.length() > 0) {
            // HTML Header
            server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
            server.sendHeader("Pragma", "no-cache");
            server.sendHeader("Expires", "-1");
            server.setContentLength(temp.length());
            server.send(417, "text/plain", temp);
            Serial.println(temp);
            return;
        }
        // End WifiAP
    }
}

void loop() {
    if (SoftAccOK) {
        dnsServer.processNextRequest(); //DNS
    }
    if (reconnect) {
        startWifi();
        reconnect = false;
    }
    //HTTP
    server.handleClient();
    yield();
}
