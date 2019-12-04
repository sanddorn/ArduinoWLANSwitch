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


bool softap_up;

bool CreateWifiSoftAP();

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
    softap_up = false;
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
    WifiStorage softAP = storage.getSoftAPData();
    Serial.print("SoftAP ");
    Serial.printf("SoftAP Settings: '%s' Passwd: '%s'\n", softAP.AccessPointName, softAP.AccessPointPassword);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    SoftAccOK = WiFi.softAP(softAP.AccessPointName, softAP.AccessPointPassword); // Passwortlänge mindestens 8 Zeichen !
    delay(600); // Without delay I've seen the IP address blank
    if (SoftAccOK) {
        Serial.println("SoftAP: OK");
        /* Setup the DNS server redirecting all the domains to the apIP */
        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(DNS_PORT, "*", apIP);
        softap_up = true;
    } else {
        Serial.println("Softap: err");
    }
    return SoftAccOK;
}

bool connect_to_wifi(WifiStorage *wifiStorage) {
    WiFi.begin(wifiStorage->AccessPointName, wifiStorage->AccessPointPassword);
    byte i = 0;
    byte connRes = WiFi.waitForConnectResult();
    while ((connRes == 0) && (i < 10)) {
        WiFi.waitForConnectResult(60000);
        i++;
        Serial.printf("Connecting to %s, Wait-Phase %i\n", wifiStorage->AccessPointName, i);
    }
    return connRes == WL_CONNECTED;
}

void scan_completed(int noNetworks) {
    for (int i = 0; i < noNetworks; i++) {
        WifiStorage *wifi = storage.retrieveNetwork(WiFi.SSID(i).c_str());
        if (wifi != nullptr) {
            if (connect_to_wifi(wifi)) {
                return;
            }
        }
    }
    if (!softap_up) {
        CreateWifiSoftAP();
    }
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

    htmlHandler.setSSID(WiFi.SSID().c_str());
    htmlHandler.setAPName(MyWiFiConfig.APSTAName);
    if (!MyWiFiConfig.AccessPointMode) {
        htmlHandler.setBSSID(WiFi.BSSIDstr().c_str());
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

Settings

     */

    // remove unwanted networks from known network list
    vector<String> networksToDelete;
    for (int i = 0; i < storage.getNumberOfKnownNetworks(); i++) {
        String networkName = storage.getApSSID(i);
        networkName += +"_delete";
        if (server.hasArg(networkName) && server.arg(networkName).equals("on")) {
            networksToDelete.emplace_back(storage.getApSSID(i));
        }
    }
    for (auto iterator = networksToDelete.begin(); iterator < networksToDelete.end(); iterator++) {
        storage.removeWifiNetwork((*iterator).c_str());
    }
    // Add new Network, if any
    if (server.hasArg("WiFi_Add_Network")) {
        WifiStorage addNEtwork;
        strncpy(addNEtwork.AccessPointName, server.arg("WiFi_Add_Network").c_str(), ACCESSPOINT_NAME_LENGTH);
        if (server.hasArg("STAWLanPW")) {
            strncpy(addNEtwork.AccessPointPassword, server.arg("STAWLanPW").c_str(), WIFI_PASSWORD_LENGTH);
        }
    }
    // TODO: Settings of local endpoint...
    /*
     * APPointName	ESP8266_Config_WLAN
     * APPW	12345678
     * APPWRepeat	12345678
     */
    // ------- Old impl
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
    //HTTP
    server.handleClient();
    ArduinoOTA.handle();
    checkTrigger();
    yield();
}


