#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include "../lib/EEPROM/EEPromStorage.h"
#include "../lib/HTMLHandler/HTMLHandler.h"
#include <ArduinoOTA.h>
#include <FS.h>
#include "../lib/ValveHandler/ValveHandler.h"


#define VALVE_PORT 13
#define TRIGGER_PORT D2


int lastTriggerState = LOW;
/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *ESPHostname = "ESP";

// DNS server
const byte DNS_PORT = 53;

DNSServer dnsServer;

HTMLHandler htmlHandler;

ValveHandler valveHandler(VALVE_PORT);

EEPromStorage storage;

//Conmmon Paramenters
bool SoftAccOK = false;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(172, 20, 0, 1);
IPAddress netMsk(255, 255, 255, 0);

WiFiEEPromData MyWiFiConfig;

bool reconnect;

bool softap_up;

bool CreateWifiSoftAP();

byte ConnectWifiAP();

void InitalizeHTTPServer();

void startWifi();

void handleRoot();

void handleWifi();

void handleCss();

void handleNotFound();

void handleWifiSetup();

void handleOpenValve();

void handleCloseValve();

void handleValveStatus();

void scan_completed(int noNetworks);

void setup() {
    pinMode(VALVE_PORT, OUTPUT);
    pinMode(TRIGGER_PORT, INPUT);
    digitalWrite(VALVE_PORT, LOW);
    Serial.begin(9600);
    Serial.println("booting");
    softap_up=false;
    WiFi.hostname(ESPHostname); // Set the DHCP hostname assigned to ESP station.
    Serial.setDebugOutput(true); //Debug Output for WLAN on Serial Interface.
    startWifi();
    ArduinoOTA.onStart([]() {
        Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.setPassword("Password");
    ArduinoOTA.begin();

    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void startWifi() {
    bool ConnectSuccess;
    WiFi.disconnect();
    /* Function will set currently configured SSID and password of the soft-AP to null values. The parameter  is optional. If set to true it will switch the soft-AP mode off.*/
    WiFi.softAPdisconnect(true);

    if (storage.getNumberOfKnownNetworks() == 0) // Load WLAN credentials for WiFi Settings
    {
        Serial.println("AP Mode");
        ConnectSuccess = CreateWifiSoftAP();
    } else {
        WiFi.scanDelete();
        WiFi.scanNetworksAsync(scan_completed);
        return;
    }
    if (ConnectSuccess) {
        Serial.println("Wifi Setup Succes");
    } else {
        Serial.println("No SoftAP or STA could be started. Resetting to default");
        // TODO: Reset EEPromStorage to defaults
    }
    InitalizeHTTPServer();
}

void InitalizeHTTPServer() {
    /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
    server.on("/", handleRoot);
    server.on("/wifi", handleWifi);
    server.on("/wifiSetup", handleWifiSetup);
    server.on("/Valve/Open", handleOpenValve);
    server.on("/Valve/Close", handleCloseValve);
    server.on("/Valve", handleValveStatus);
    server.on("/portal.css", handleCss);
    server.onNotFound(handleNotFound);
    server.begin(); // Web server start
}

bool CreateWifiSoftAP() {
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
        softap_up = true;
    } else {
        Serial.println("err");
        Serial.println(MyWiFiConfig.APSTAName);
        Serial.println(MyWiFiConfig.WiFiPwd);
    }
    return SoftAccOK;
}

bool connect_to_wifi(WifiStorage * wifiStorage) {
    WiFi.begin(wifiStorage->AccessPointName, wifiStorage->AccessPointPassword);
    byte i =0;
    byte connRes = WiFi.waitForConnectResult();
    while ((connRes == 0) && (i < 10)) {
        WiFi.waitForConnectResult(60000);
        i++;
        Serial.printf("Connecting to %s, Wait-Phase %i\n", wifiStorage->AccessPointName, i);
    }
    return connRes == WL_CONNECTED;
}

void scan_completed(int noNetworks) {
    for (int i = 0; i<noNetworks; i++) {
        WifiStorage * wifi = storage.retrieveNetwork(WiFi.SSID(i).c_str());
        if (wifi != nullptr) {
            if(connect_to_wifi(wifi)) {
                return;
            }
        }
    }
    if (!softap_up) {
        CreateWifiSoftAP();
    }
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
        // Set whether module will attempt to reconnect to an access point in case it is disconnected.
        WiFi.setAutoReconnect(true);
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

void handleRoot() {
    //  Main Page:
    // HTML Header
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    // HTML Content
    const String &webPage = htmlHandler.getMainPage().c_str();
    server.setContentLength(webPage.length());
    server.send(200, MEDIATYPE_TEXT_HTML, webPage);
}


void handleNotFound() {
    // Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + server.client().localIP().toString(), true);
    server.send(302, MEDIATYPE_TEXT_PLAIN,
                "Redirect");
}

/** Wifi config page handler */
void handleWifi() {
    String webPage = "";
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");

    for (int i = 0; i< storage.getNumberOfKnownNetworks(); i++) {
        htmlHandler.addRegisteredNetwork(storage.getApSSID(i));
    }

    WiFi.scanDelete();
    int scannedNetworks = WiFi.scanNetworks(); //WiFi.scanNetworks(async, show_hidden)
    for (int j = 0; j < scannedNetworks; j++) {
        htmlHandler.addAvailableNetwork(WiFi.SSID(j).c_str(), WiFi.encryptionType(j), WiFi.RSSI(j));
    }
    webPage = htmlHandler.getWifiPage().c_str();
    server.setContentLength(webPage.length());
    server.send(200, MEDIATYPE_TEXT_HTML, webPage);
    htmlHandler.resetWifiPage();
}


void handleCss() {
    String cssString = htmlHandler.getCss().c_str();
    server.setContentLength(cssString.length());
    server.send(200, "text/css", cssString);
}


void handleWifiSetup() {
    /*
     *
WiFi_Add_Network	ssid
STAWLanPW	asdf
APPointName	ESP8266_Config_WLAN
APPW	12345678
APPWRepeat	12345678
Settings

     */

    // remove unwanted networks from known network list
    vector<String> networksToDelete;
    for (int i = 0; i< storage.getNumberOfKnownNetworks(); i++) {
        String networkName = storage.getApSSID(i);
        networkName += + "_delete";
        if (server.hasArg(networkName) && server.arg(networkName).equals("on")) {
            networksToDelete.push_back(storage.getApSSID(i));
        }
    }
    for (vector<String>::iterator iterator = networksToDelete.begin(); iterator < networksToDelete.end(); iterator++) {
        storage.removeWifiNetwork(*iterator);
    }

    return;
    // ------- Old impl
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
            server.send(200, MEDIATYPE_TEXT_HTML, temp);
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
            server.send(200, MEDIATYPE_TEXT_PLAIN, temp);
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
            server.send(417, MEDIATYPE_TEXT_PLAIN, temp);
            Serial.println(temp);
            return;
        }
        // End WifiAP
    }
}

void handleOpenValve() {
    String page = htmlHandler.getSwitch(true).c_str();
    server.setContentLength(page.length());
    server.send(200, MEDIATYPE_TEXT_HTML, page);
    valveHandler.openValve();
}

void handleCloseValve() {
    String page = htmlHandler.getSwitch(false).c_str();
    server.setContentLength(page.length());
    server.send(200, MEDIATYPE_TEXT_HTML, page);
    valveHandler.closeValve();
}

void handleValveStatus() {
    String page = htmlHandler.getSwitch(valveHandler.getStatus() == VALVE_OPEN).c_str();
    server.setContentLength(page.length());
    server.send(200, MEDIATYPE_TEXT_HTML, page);
}

void checkTrigger() {
    int triggerSetting = digitalRead(TRIGGER_PORT);
    // Trigger was used
    if (triggerSetting != lastTriggerState) {
        if (triggerSetting == HIGH) {
            valveHandler.openValve();
        } else {
            valveHandler.closeValve();
        }
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
    ArduinoOTA.handle();
    checkTrigger();
    yield();
}


