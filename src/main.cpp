#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <Ticker.h>
#include <ArduinoLog.h>
#include "WifiConfigStorage.h"
#include "FS_Persistence.h"
#include "HTMLHandler.h"
#include <ArduinoOTA.h>
#include <FS.h>
#include "ValveHandler.h"
#include "EEPROMWifiStorage.h"
#include "TimedCallbackHandler.h"


#define VALVE_OPEN_PORT D5
#define VALVE_CLOSE_PORT D6
#define TRIGGER_OPEN_PORT D2
#define TRIGGER_CLOSE_PORT D3


int lastTriggerState = LOW;
/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *ESPHostname = "ESP";

// DNS server
const byte DNS_PORT = 53;

DNSServer dnsServer;

Logging logging_Storage;
Logging logging_Valve;
Logging mainLog;

HTMLHandler *htmlHandler;
ValveHandler valveHandler(VALVE_OPEN_PORT, VALVE_CLOSE_PORT, TimedCallbackHandler::getInstance(), logging_Valve);

WifiConfigStorage *storage;
EEPROMWifiStorage eepromStorage;
// Web server
ESP8266WebServer server(80);

/* Soft AP available_network parameters */
static IPAddress apIP(172, 20, 0, 1);
static IPAddress netMsk(255, 255, 255, 0);


bool isSoftAP;

bool isAPAccociated;

bool isScanning = false;

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

void doReboot();

void scan_completed(int noNetworks);

typedef struct _wifi_info {
    char *ssid;
    uint8_t encryption;
    int32_t strength;
} WIFI_INFO;

WIFI_INFO *available_networks;


void sendCR(Print *p) {
    p->print(CR);
}

void setup() {
    pinMode(VALVE_OPEN_PORT, OUTPUT);
    pinMode(VALVE_CLOSE_PORT, OUTPUT);
    pinMode(TRIGGER_OPEN_PORT, INPUT_PULLUP);
    pinMode(TRIGGER_CLOSE_PORT, INPUT_PULLUP);
    digitalWrite(VALVE_OPEN_PORT, LOW);
    digitalWrite(VALVE_CLOSE_PORT, LOW);
    Serial.begin(115200);
    Serial.println("booting");
    isSoftAP = false;

    fs::SPIFFSConfig cfg;
    cfg.setAutoFormat(false);
    SPIFFS.setConfig(cfg);
    auto *persistence = new FS_Persistence(&SPIFFS);

    mainLog.begin(LOG_LEVEL_VERBOSE, &Serial);
    mainLog.setSuffix(sendCR);
    mainLog.setPrefix([](Print *p) { p->print("MAIN: "); });

    logging_Storage.begin(LOG_LEVEL_VERBOSE, &Serial);
    logging_Storage.setSuffix(sendCR);
    logging_Storage.setPrefix([](Print *p) { p->print("STORAGE: "); });

    logging_Valve.begin(LOG_LEVEL_TRACE, &Serial);
    logging_Valve.setSuffix(sendCR);
    logging_Valve.setPrefix([](Print *p) { p->print("VALVE: "); });

    htmlHandler = new HTMLHandler(persistence, &logging_Storage);
    storage = new WifiConfigStorage(&logging_Storage, &eepromStorage);
    storage->initStorage();
    //storage->resetStorage();
    WiFi.hostname(ESPHostname); // Set the DHCP hostname assigned to ESP station.
    Serial.setDebugOutput(true); //Debug Output for WLAN on Serial Interface.
    available_networks = nullptr;
    startWifi();
    MDNS.begin(ESPHostname);
    InitalizeHTTPServer();
    ArduinoOTA.onStart([]() {
        mainLog.notice("OTA Start");
    });
    ArduinoOTA.onEnd([]() {
        mainLog.notice("OTA End");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        mainLog.verbose("OTA Progress: %u%%", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        mainLog.verbose("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) mainLog.verbose("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) mainLog.verbose("OTA Begin Failed");
        else if (error == OTA_CONNECT_ERROR) mainLog.verbose("OTA Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) mainLog.verbose("OTA Receive Failed");
        else if (error == OTA_END_ERROR) mainLog.verbose("OTA End Failed");
    });
    ArduinoOTA.setPassword("Password");
    ArduinoOTA.begin();


    mainLog.verbose("Ready. IP: %s", WiFi.localIP().toString().c_str());
}

void startWifi() {
    WiFi.disconnect();
    /* Function will set currently configured SSID and password of the soft-AP to null values. The parameter  is optional. If set to true it will switch the soft-AP mode off.*/
    WiFi.softAPdisconnect(true);

    mainLog.verbose("AP Mode");
    WiFi.begin();
    CreateWifiSoftAP();
    if (isSoftAP) {
        mainLog.verbose("Wifi Setup Succes");
    } else {
        mainLog.verbose("No SoftAP or STA could be started. Resetting to default");
        storage->resetStorage();
    }
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
    server.on("/reboot", doReboot);
    server.onNotFound(handleNotFound);
    server.begin(); // Web server start
}

void CreateWifiSoftAP() {
    WifiStorage softAP = storage->getSoftAPData();
    mainLog.verbose("SoftAP ");
    mainLog.verbose("SoftAP Settings: '%s' Passwd: '%s'\n", softAP.AccessPointName, softAP.AccessPointPassword);
    WiFi.enableAP(true);
    WiFi.enableSTA(false);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    bool SoftAccOK = WiFi.softAP(softAP.AccessPointName,
                                 softAP.AccessPointPassword); // Passwortlänge mindestens 8 Zeichen !
    if (SoftAccOK) {
        mainLog.verbose("SoftAP: OK");
        /* Setup the DNS server redirecting all the domains to the apIP */
        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(DNS_PORT, "*", apIP);
        isSoftAP = true;
        isAPAccociated = false;
        server.begin(80);
    } else {
        mainLog.verbose("Softap: err");
    }
    mainLog.verbose("Wifi Addr: %s\n", WiFi.localIP().toString().c_str());
    WiFi.scanDelete();
    WiFi.scanNetworksAsync(scan_completed);
    isScanning = true;
}

void set_disconnected(const WiFiEventStationModeDisconnected &wiFiEventStationModeDisconnected) {
    mainLog.notice("Disconnected from AP %s for reasonNo %i\n", wiFiEventStationModeDisconnected.ssid.c_str(),
                   wiFiEventStationModeDisconnected.reason);
    CreateWifiSoftAP();
}

void onEvent(WiFiEvent_t event) {
    switch (event) {
        case WIFI_EVENT_STAMODE_CONNECTED:
            WiFi.enableAP(false);
            mainLog.notice("Stopped SoftAP");
            break;
        case WIFI_EVENT_STAMODE_DISCONNECTED:
            mainLog.verbose("STAMODE Disconnected");
            break;
        case WIFI_EVENT_STAMODE_AUTHMODE_CHANGE:
            mainLog.verbose("STAMODE authmode Change");
            break;
        case WIFI_EVENT_STAMODE_GOT_IP:
            mainLog.verbose("STAMODE got ip");
            break;
        case WIFI_EVENT_STAMODE_DHCP_TIMEOUT:
            mainLog.verbose("DHCP Timeout");
            startWifi();
            break;
        case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
            mainLog.verbose("SoftAP STA connected");
            break;
        case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
            mainLog.verbose("SoftAP STA disconected");
            break;
        case WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED:
            mainLog.verbose("SoftAP STA probereq receivec");
            break;
        case WIFI_EVENT_MODE_CHANGE:
            mainLog.verbose("wifi modeChenage");
            break;
        case WIFI_EVENT_SOFTAPMODE_DISTRIBUTE_STA_IP:
            mainLog.verbose("SoftAP STA distribute sta ip");
            break;
        case WIFI_EVENT_ANY:
        default:
            mainLog.verbose("Unspecific event.");
            break;
    }
}

bool connect_to_wifi(WifiStorage *wifiStorage) {
    mainLog.verbose("connect_to_wifi\n");
    mainLog.notice("Starting Associate with network: %s:%s\n", wifiStorage->AccessPointName,
                   wifiStorage->AccessPointPassword);
    WiFi.begin(wifiStorage->AccessPointName, wifiStorage->AccessPointPassword);
    WiFi.setAutoReconnect(false);
    byte i = 0;
    mainLog.verbose("Checking ConnectionResult\n");
    byte connRes = WL_DISCONNECTED;
    while ((connRes != WL_CONNECTED) && (i < 10)) {
        connRes = WiFi.waitForConnectResult(5000);
        i++;
        mainLog.verbose("Connecting to %s, Wait-Phase %i\n", wifiStorage->AccessPointName, i);
    }
    WiFi.onStationModeDisconnected(set_disconnected);
    WiFi.onEvent(onEvent);
    server.begin(80);
    return connRes == WL_CONNECTED;
}

void scan_completed(int noNetworks) {
    isScanning = false;
    mainLog.verbose("scan_completed: %i\n", noNetworks);
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
    const String &webPage = htmlHandler->getMainPage().c_str();
    server.setContentLength(webPage.length());
    server.send(200, MEDIATYPE_TEXT_HTML, webPage);
}


void handleNotFound() {
    mainLog.verbose("Request redirected to captive portal");
    mainLog.verbose("server.client(): %s", server.client().localIP().toString().c_str());
    server.sendHeader("Location", String("http://") + server.client().localIP().toString(), true);
    server.send(302, MEDIATYPE_TEXT_PLAIN,
                "Redirect");
}

/** Wifi config page handler */
void handleWifi() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");

    for (int i = 0; i < storage->getNumberOfKnownNetworks(); i++) {
        htmlHandler->addRegisteredNetwork(storage->getApSSID(i));
    }
    WIFI_INFO *network = available_networks;
    while (network != nullptr && network->ssid != nullptr) {
        htmlHandler->addAvailableNetwork(network->ssid, network->encryption, network->encryption);
        network++;
    }
    htmlHandler->setSoftAPCredentials(storage->getSoftAPData().AccessPointName,
                                      storage->getSoftAPData().AccessPointPassword);
    String webPage = htmlHandler->getWifiPage().c_str();
    htmlHandler->resetWifiPage();
    server.setContentLength(webPage.length());
    server.send(200, MEDIATYPE_TEXT_HTML, webPage);

}


void handleCss() {
    String cssString = htmlHandler->getCss().c_str();
    server.setContentLength(cssString.length());
    server.send(200, "text/css", cssString);
}


void handleWifiSetup() {
    vector<String> networksToDelete;
    for (int i = 0; i < storage->getNumberOfKnownNetworks(); i++) {
        String networkName = storage->getApSSID(i);
        networkName += +"_delete";
        if (server.hasArg(networkName) && server.arg(networkName).equals("on")) {
            networksToDelete.emplace_back(storage->getApSSID(i));
        }
    }
    for (auto iterator = networksToDelete.begin(); iterator < networksToDelete.end(); iterator++) {
        storage->removeWifiNetwork((*iterator).c_str());
    }
    // Add new Network, if any
    if (server.hasArg("WiFi_Add_Network") && server.arg("WiFi_Add_Network").length() > 0) {

        WifiStorage addNEtwork{};
        strncpy(addNEtwork.AccessPointName, server.arg("WiFi_Add_Network").c_str(), ACCESSPOINT_NAME_LENGTH);
        if (server.hasArg("STAWLanPW")) {
            strncpy(addNEtwork.AccessPointPassword, server.arg("STAWLanPW").c_str(), WIFI_PASSWORD_LENGTH);
        }
        mainLog.verbose("Add available_network: '%s':'%s\n", addNEtwork.AccessPointName,
                        addNEtwork.AccessPointPassword);
        storage->addWifiNetwork(addNEtwork);
    }

    if (server.hasArg("SoftAPSSID") &&
        strncmp(server.arg("SoftAPSSID").c_str(), storage->getSoftAPData().AccessPointName, ACCESSPOINT_NAME_LENGTH) ==
        0) {
        // Set AP Name
    }
    // TODO: Password for softap
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");

    const string &webpage = htmlHandler->getWifiSaveDonePage();

    server.setContentLength(webpage.length());
    server.send(200, MEDIATYPE_TEXT_HTML, webpage.c_str());
    if (!isScanning) {
        WiFi.scanDelete();
        WiFi.scanNetworksAsync(scan_completed);
    }
    isScanning = true;
}

void handleFactoryReset() {
    storage->resetStorage();

    server.sendHeader("Location", String("http://") + server.client().localIP().toString(), true);

    server.send(307, MEDIATYPE_TEXT_PLAIN, "Redirect");
}

void doReboot() {
    ESP.restart();
}

void handleOpenValve() {
    mainLog.verbose("handleOpenValve start");
    String page = htmlHandler->getSwitch(true).c_str();
    server.setContentLength(page.length());
    server.send(200, MEDIATYPE_TEXT_HTML, page);
    valveHandler.openValve();
    mainLog.verbose("handleOpenValve stop");
}

void handleCloseValve() {
    String page = htmlHandler->getSwitch(false).c_str();
    server.setContentLength(page.length());
    server.send(200, MEDIATYPE_TEXT_HTML, page);
    valveHandler.closeValve();
}

void handleValveStatus() {
    String page = htmlHandler->getSwitch(valveHandler.getStatus() == VALVESTATE::OPEN).c_str();
    server.setContentLength(page.length());
    server.send(200, MEDIATYPE_TEXT_HTML, page);
}

void checkTrigger() {
    int triggerOpenSetting = digitalRead(TRIGGER_OPEN_PORT);
    int triggerCloseSetting = digitalRead(TRIGGER_CLOSE_PORT);
    if (triggerOpenSetting == triggerCloseSetting) {
        return;
    }
    if (triggerOpenSetting == LOW &&
        (valveHandler.getStatus() != VALVESTATE::OPEN && valveHandler.getStatus() != VALVESTATE::OPENING)) {
        mainLog.verbose("Trigger Opening Valve: %d\n", valveHandler.getStatus());
        valveHandler.openValve();
    }
    if (triggerCloseSetting == LOW &&
        (valveHandler.getStatus() != VALVESTATE::CLOSED && valveHandler.getStatus() != VALVESTATE::CLOSING)) {
        mainLog.verbose("Trigger Closing Valve: %d\n", valveHandler.getStatus());

        valveHandler.closeValve();
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
    if (WiFi.status() == WL_IDLE_STATUS && isAPAccociated) {
        startWifi();
    }
    if (!isScanning && storage->getNumberOfKnownNetworks() > 0) {
        while (!isAPAccociated && network != nullptr && network->ssid != nullptr) {
            WifiStorage *wifi = storage->retrieveNetwork(network->ssid);

            if (wifi != nullptr) {
                if (connect_to_wifi(wifi)) {
                    isAPAccociated = true;
                    isSoftAP = false;
                    WiFi.enableAP(false);
                    mainLog.verbose("Connection to AP succeedd: %s\n", wifi->AccessPointName);
                }
            }
            network++;
        }
        WiFi.scanDelete();
        WiFi.scanNetworksAsync(scan_completed);
        isScanning = true;
    }

    if (!isAPAccociated && !isSoftAP) {
        mainLog.verbose("Creating Softap\n");
        CreateWifiSoftAP();
    }

    yield();
}


