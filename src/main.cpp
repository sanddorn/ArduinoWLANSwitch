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
#define TRIGGER_PORT D5


int lastTriggerState = LOW;
/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *ESPHostname = "ESP";

// DNS server
const byte DNS_PORT = 53;

DNSServer dnsServer;

HTMLHandler htmlHandler;

ValveHandler valveHandler(VALVE_PORT);

EEPromStorage storage;

// Web server
ESP8266WebServer server(80);

/* Soft AP available_network parameters */
static IPAddress apIP(172, 20, 0, 1);
static IPAddress netMsk(255, 255, 255, 0);


bool isSoftAP;

bool isAPAccociated;

void CreateWifiSoftAP();

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

void handleFactoryReset();

void scan_completed(int noNetworks);

typedef struct _wifi_info {
    char *ssid;
    uint8_t encryption;
    int32_t strength;
} WIFI_INFO;

WIFI_INFO *available_networks;


void setup() {
    pinMode(VALVE_PORT, OUTPUT);
    pinMode(TRIGGER_PORT, INPUT);
    digitalWrite(VALVE_PORT, LOW);
    Serial.begin(115200);
    Serial.println("booting");
    isSoftAP = false;
    storage.initStorage();
//    storage.resetStorage();
    WiFi.hostname(ESPHostname); // Set the DHCP hostname assigned to ESP station.
    Serial.setDebugOutput(true); //Debug Output for WLAN on Serial Interface.
    available_networks = nullptr;
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
    WiFi.disconnect();
    /* Function will set currently configured SSID and password of the soft-AP to null values. The parameter  is optional. If set to true it will switch the soft-AP mode off.*/
    WiFi.softAPdisconnect(true);

    Serial.println("AP Mode");
    CreateWifiSoftAP();
    WiFi.scanDelete();
    WiFi.scanNetworksAsync(scan_completed);;
    if (isSoftAP) {
        Serial.println("Wifi Setup Succes");
    } else {
        Serial.println("No SoftAP or STA could be started. Resetting to default");
        storage.resetStorage();
    }
    InitalizeHTTPServer();

}

void InitalizeHTTPServer() {
    /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
    server.on("/", handleRoot);
    server.on("/wifi", handleWifi);
    server.on("/wifiSetup", handleWifiSetup);
    server.on("/reset", handleFactoryReset);
    server.on("/Valve/Open", handleOpenValve);
    server.on("/Valve/Close", handleCloseValve);
    server.on("/Valve", handleValveStatus);
    server.on("/portal.css", handleCss);
    server.onNotFound(handleNotFound);
    server.begin(); // Web server start
}

void CreateWifiSoftAP() {
    WifiStorage softAP = storage.getSoftAPData();
    Serial.print("SoftAP ");
    Serial.printf("SoftAP Settings: '%s' Passwd: '%s'\n", softAP.AccessPointName, softAP.AccessPointPassword);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    bool SoftAccOK = WiFi.softAP(softAP.AccessPointName,
                                 softAP.AccessPointPassword); // Passwortlänge mindestens 8 Zeichen !
    delay(600); // Without delay I've seen the IP address blank
    if (SoftAccOK) {
        Serial.println("SoftAP: OK");
        /* Setup the DNS server redirecting all the domains to the apIP */
        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(DNS_PORT, "*", apIP);
        isSoftAP = true;
        isAPAccociated = false;
    } else {
        Serial.println("Softap: err");
    }
    WiFi.scanDelete();
    WiFi.scanNetworksAsync(scan_completed);
}

void set_disconnected(const WiFiEventStationModeDisconnected &wiFiEventStationModeDisconnected) {
    Serial.printf("Disconnected from AP %s for reasonNo %i\n", wiFiEventStationModeDisconnected.ssid.c_str(),
                  wiFiEventStationModeDisconnected.reason);
}

bool connect_to_wifi(WifiStorage *wifiStorage) {
    Serial.printf("connect_to_wifi\n");
    WiFi.softAPdisconnect(false);
    isSoftAP = false;
    Serial.printf("stopped SoftAP, Starting Associate with network: %s:%s\n", wifiStorage->AccessPointName,
                  wifiStorage->AccessPointPassword);
    WiFi.begin(wifiStorage->AccessPointName, wifiStorage->AccessPointPassword);
    WiFi.onStationModeDisconnected(set_disconnected);
    byte i = 0;
    Serial.printf("Checking ConnectionResult\n");
    byte connRes = WL_DISCONNECTED;
    while ((connRes != WL_CONNECTED) && (i < 10)) {
        connRes = WiFi.waitForConnectResult(60000);
        i++;
        Serial.printf("Connecting to %s, Wait-Phase %i\n", wifiStorage->AccessPointName, i);
    }
    return connRes == WL_CONNECTED;
}

void scan_completed(int noNetworks) {
    Serial.printf("scan_completed: %i\n", noNetworks);
    WIFI_INFO *net = available_networks;
    while (net != nullptr && net->ssid != nullptr) {
        delete[] net->ssid;
        net++;
    }
    delete[]available_networks;
    available_networks = new WIFI_INFO[noNetworks + 1];
    available_networks[noNetworks].ssid = nullptr;

    for (int i = 0; i < noNetworks; i++) {
        available_networks[i].ssid = new char[strlen(WiFi.SSID(i).c_str()) + 1];
        strcpy(available_networks[i].ssid, WiFi.SSID(i).c_str());
        available_networks[i].encryption = WiFi.encryptionType(i);
        available_networks[i].strength = WiFi.RSSI(i);
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
    Serial.println("Request redirected to captive portal");
    Serial.printf("server.client(): %s", server.client().localIP().toString().c_str());
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

    for (int i = 0; i < storage.getNumberOfKnownNetworks(); i++) {
        htmlHandler.addRegisteredNetwork(storage.getApSSID(i));
    }
    WIFI_INFO *network = available_networks;
    while (network != nullptr && network->ssid != nullptr) {
        htmlHandler.addAvailableNetwork(network->ssid, network->encryption, network->encryption);
        network++;
    }
    htmlHandler.setSoftAPCredentials(storage.getSoftAPData().AccessPointName);
    webPage = htmlHandler.getWifiPage().c_str();
    htmlHandler.resetWifiPage();
    server.setContentLength(webPage.length());
    server.send(200, MEDIATYPE_TEXT_HTML, webPage);

}


void handleCss() {
    String cssString = htmlHandler.getCss().c_str();
    server.setContentLength(cssString.length());
    server.send(200, "text/css", cssString);
}


void handleWifiSetup() {
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
    if (server.hasArg("WiFi_Add_Network") && server.arg("WiFi_Add_Network").length() > 0) {

        WifiStorage addNEtwork{};
        strncpy(addNEtwork.AccessPointName, server.arg("WiFi_Add_Network").c_str(), ACCESSPOINT_NAME_LENGTH);
        if (server.hasArg("STAWLanPW")) {
            strncpy(addNEtwork.AccessPointPassword, server.arg("STAWLanPW").c_str(), WIFI_PASSWORD_LENGTH);
        }
        Serial.printf("Add available_network: '%s':'%s\n", addNEtwork.AccessPointName, addNEtwork.AccessPointPassword);
        storage.addWifiNetwork(addNEtwork);
    }

    if (server.hasArg("SoftAPSSID") &&
        strncmp(server.arg("SoftAPSSID").c_str(), storage.getSoftAPData().AccessPointName, ACCESSPOINT_NAME_LENGTH) ==
        0) {
        // Set AP Name
    }
    // TODO: Password for softap
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");

    const string &webpage = htmlHandler.getWifiSaveDonePage();

    server.setContentLength(webpage.length());
    server.send(200, MEDIATYPE_TEXT_HTML, webpage.c_str());
    WiFi.scanDelete();
    WiFi.scanNetworksAsync(scan_completed);
}

void handleFactoryReset() {
    storage.resetStorage();

    server.sendHeader("Location", String("http://") + server.client().localIP().toString(), true);

    server.send(307, MEDIATYPE_TEXT_PLAIN, "Redirect");
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
    if (isSoftAP) {
        dnsServer.processNextRequest(); //DNS
    }
    //HTTP
    server.handleClient();
    ArduinoOTA.handle();
    checkTrigger();
    WIFI_INFO *network = available_networks;
    while (!isAPAccociated && network != nullptr && network->ssid != nullptr) {
        Serial.printf("Checking for known networks\n");
        Serial.printf("Checking for ssid: %s\n", network->ssid);
        WifiStorage *wifi = storage.retrieveNetwork(network->ssid);

        if (wifi != nullptr) {
            if (connect_to_wifi(wifi)) {
                isAPAccociated = true;
                isSoftAP = false;
                Serial.printf("Connection to AP succeedd: %s\n", wifi->AccessPointName);
            }
        }
        network++;
    }
    if (!isAPAccociated) {
        Serial.printf("No AP Connection could be used\n");
        WiFi.scanDelete();
        WiFi.scanNetworksAsync(scan_completed);
    }
    if (!isAPAccociated && !isSoftAP) {
        CreateWifiSoftAP();
    }

    yield();
}


