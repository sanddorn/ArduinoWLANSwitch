#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include "../lib/EEPROM/EEPromStorage.h"


#define BLINK_LED D0
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

boolean loadCredentials();

boolean CreateWifiSoftAP();

byte ConnectWifiAP();

void SetDefaultWiFiConfig();

boolean saveCredentials();

void InitalizeHTTPServer();

void handleRoot();

void handleWifi();

void handleNotFound();

boolean configureWifi();

void doReboot();

void setup() {
    bool ConnectSuccess = false;
    bool CreateSoftAPSucc = false;
    byte len;
    pinMode(BLINK_LED, OUTPUT); // Initialize the BUILTIN_LED1 pin as an output
    Serial.begin(9600);
    Serial.println();
    WiFi.hostname(ESPHostname); // Set the DHCP hostname assigned to ESP station.
    if (loadCredentials()) // Load WLAN credentials for WiFi Settings
    {
        // Valid Credentials found.
        if (MyWiFiConfig.APSTA == true)  // AP Mode
        {
            //Serial.println("AP Mode");
            //Serial.println(MyWiFiConfig.APSTA);
            len = strlen(MyWiFiConfig.APSTAName);
            MyWiFiConfig.APSTAName[len + 1] = '\0';
            len = strlen(MyWiFiConfig.WiFiPwd);
            MyWiFiConfig.WiFiPwd[len + 1] = '\0';
            CreateSoftAPSucc = CreateWifiSoftAP();
        } else {
            //Serial.println("STA Mode");
            len = strlen(MyWiFiConfig.APSTAName);
            MyWiFiConfig.APSTAName[len + 1] = '\0';
            len = strlen(MyWiFiConfig.WiFiPwd);
            MyWiFiConfig.WiFiPwd[len + 1] = '\0';
            len = ConnectWifiAP();
            ConnectSuccess = (len == 3);
        }
    } else { //Set default Config - Create AP
        Serial.println("DefaultWiFi Cnf");
        SetDefaultWiFiConfig();
        CreateSoftAPSucc = CreateWifiSoftAP();
        saveCredentials();
        // Blink
        digitalWrite(BLINK_LED, LOW); // Pull to LOW _Led ON
        delay(500);
        digitalWrite(BLINK_LED, HIGH);
        delay(500);
        digitalWrite(BLINK_LED, LOW); // Pull to LOW _Led ON
        delay(500);
        digitalWrite(BLINK_LED, HIGH);
        delay(500);
        digitalWrite(BLINK_LED, LOW); // Pull to LOW _Led ON
    }
    if (ConnectSuccess or CreateSoftAPSucc) {
        InitalizeHTTPServer();
        digitalWrite(D0, LOW); // Pull to LOW _Led ON
        Serial.println("Setup Succes for STA or SoftAP");
    } else {
        Serial.setDebugOutput(true); //Debug Output for WLAN on Serial Interface.
        Serial.println("No SoftAP or STA could be started. Resetting to default");
        SetDefaultWiFiConfig();
        CreateWifiSoftAP();
        saveCredentials();
        InitalizeHTTPServer();
    }
}

void InitalizeHTTPServer() {
    /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
    server.on("/", handleRoot);
    server.on("/wifi", handleWifi);
    server.onNotFound(handleNotFound);
    server.begin(); // Web server start
}

boolean CreateWifiSoftAP() {
    WiFi.disconnect();
    Serial.print("SoftAP ");
    WiFi.softAPConfig(apIP, apIP, netMsk);
    if (MyWiFiConfig.PwDReq) {
        SoftAccOK = WiFi.softAP(MyWiFiConfig.APSTAName, MyWiFiConfig.WiFiPwd); // Passwortlänge mindestens 8 Zeichen !
    } else {
        SoftAccOK = WiFi.softAP(MyWiFiConfig.APSTAName); // Access Point WITHOUT Password
        // Overload Function:; WiFi.softAP(ssid, password, channel, hidden)
    }
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
    WiFi.disconnect();
    WiFi.softAPdisconnect(
            true); // Function will set currently configured SSID and password of the soft-AP to null values. The parameter  is optional. If set to true it will switch the soft-AP mode off.
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
        //Serial.print("PwLen:");
        //Serial.println(strlen(MyWiFiConfig.WiFiPwd));
        //Serial.print("PwSize");
        //Serial.println(sizeof(MyWiFiConfig.WiFiPwd));
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
    if (MyWiFiConfig.APSTA == true) {
        if (MyWiFiConfig.PwDReq and (sizeof(String(MyWiFiConfig.WiFiPwd)) < 8)) {
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
    MyWiFiConfig.APSTA = true;
    MyWiFiConfig.PwDReq = true;  // default PW required
    MyWiFiConfig.CapPortal = true;
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
    // FSInfo fs_info;
    String temp = "";
    //Building Page
    // HTML Header
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    // HTML Content
    temp = "";
    temp += "<!DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    temp += "<style type='text/css'><!-- DIV.container { min-height: 10em; display: table-cell; vertical-align: middle }.button {height:35px; width:90px; font-size:16px}";
    temp += "body {background-color: powderblue;}</style>";
    temp += "<head><title>Captive Portal</title></head>";
    temp += "<h2>Captive Portal</h2>";
    temp += "<body>";
    temp += "<br><table border=2 bgcolor = white width = 500 cellpadding =5 ><caption><p><h3>Sytemlinks:</h2></p></caption>";
    temp += "<tr><th><br>";
    temp += "<a href='/wifi'>WIFI Einstellungen</a><br><br>";
    temp += "</th></tr></table><br><br>";
    temp += "</body></html>";
    server.setContentLength(temp.length());
    server.send(200, "text/html", temp);
    server.client().stop(); // Stop is needed because we sent no content length
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
    String temp = "";
    // Check for Site Parameters
    // Reboot System
    if (server.hasArg("Reboot")) {
        temp = "Rebooting System in 5 Seconds..";
        server.send(200, "text/plain", temp);
        delay(5000);
        server.client().stop();
        doReboot();
        return;
    }
    if (configureWifi()) {
        // HTML Header
        server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        server.sendHeader("Pragma", "no-cache");
        server.sendHeader("Expires", "-1");
        // HTML Content
        temp += "<!DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name= viewport content='width=device-width, initial-scale=1.0'>";
        temp += "<style type='text/css'><!-- DIV.container { min-height: 10em; display: table-cell; vertical-align: middle }.button {height:35px; width:90px; font-size:16px}";
        temp += "body {background-color: powderblue;}</style><head><title>Wifi Switch</title></head>";
        temp += "<h2>WiFi Einstellungen</h2><body><left>";
        temp += "<table border=2 bgcolor = white width = 500 ><td><h4>Current WiFi Settings: </h4>";
        if (server.client().localIP() == apIP) {
            temp += "Mode : Soft Access Point (AP)<br>";
            temp += "SSID : " + String(MyWiFiConfig.APSTAName) + "<br><br>";
        } else {
            temp += "Mode : Station (STA) <br>";
            temp += "SSID  :  " + String(MyWiFiConfig.APSTAName) + "<br>";
            temp += "BSSID :  " + WiFi.BSSIDstr() + "<br><br>";
        }
        temp += "</td></table><br>";
        temp += "<form action='/wifi' method='post'>";
        temp += "<table border=2 bgcolor = white width = 500><tr><th><br>";
        if (MyWiFiConfig.APSTA == 1) {
            temp += "<input type='radio' value='1' name='WiFiMode' > WiFi Station Mode<br>";
        } else {
            temp += "<input type='radio' value='1' name='WiFiMode' checked > WiFi Station Mode<br>";
        }
        temp += "Available WiFi Networks:<table border=2 bgcolor = white ></tr></th><td>Number </td><td>SSID  </td><td>Encryption </td><td>WiFi Strength </td>";

        WiFi.scanDelete();
        int scannedNetworks  = WiFi.scanNetworks(false, false); //WiFi.scanNetworks(async, show_hidden)
        if (scannedNetworks > 0) {
            for (int j = 0; j < scannedNetworks; j++) {
                temp += "</tr></th>";
                String Nrb = String(j);
                temp += "<td>" + Nrb + "</td>";
                temp += "<td>" + WiFi.SSID(j) + "</td>";

                Nrb = GetEncryptionType(WiFi.encryptionType(j));
                temp += "<td>" + Nrb + "</td>";
                temp += "<td>" + String(WiFi.RSSI(j)) + "</td>";
            }
        } else {
            temp += "</tr></th>";
            temp += "<td>1 </td>";
            temp += "<td>No WLAN found</td>";
            temp += "<td> --- </td>";
            temp += "<td> --- </td>";
        }
        temp += "</table><table border=2 bgcolor = white ></tr></th><td>Connect to WiFi SSID: </td><td><select name='WiFi_Network' >";
        if (scannedNetworks > 0) {
            for (int j = 0; j < scannedNetworks; j++) {
                temp += "<option value='" + WiFi.SSID(j) + "'>" + WiFi.SSID(j) + "</option>";
            }
        } else {
            temp += "<option value='No_WiFi_Network'>No WiFiNetwork found !/option>";
        }
        temp += "</select></td></tr></th></tr></th><td>WiFi Password: </td><td>";
        temp += "<input type='text' name='STAWLanPW' maxlength='40' size='40'>";
        temp += "</td></tr></th><br></th></tr></table></table><table border=2 bgcolor = white width = 500 ><tr><th><br>";
        if (MyWiFiConfig.APSTA == true) {
            temp += "<input type='radio' name='WiFiMode' value='2' checked> WiFi Access Point Mode <br>";
        } else {
            temp += "<input type='radio' name='WiFiMode' value='2' > WiFi Access Point Mode <br>";
        }
        temp += "<table border=2 bgcolor = white ></tr></th> <td>WiFi Access Point Name: </td><td>";
        if (MyWiFiConfig.APSTA == true) {
            temp += "<input type='text' name='APPointName' maxlength='" + String(ACCESSPOINT_NAME_LENGTH - 1) +
                    "' size='30' value='" +
                    String(MyWiFiConfig.APSTAName) + "'></td>";
        } else {
            temp += "<input type='text' name='APPointName' maxlength='" + String(ACCESSPOINT_NAME_LENGTH - 1) +
                    "' size='30' ></td>";
        }
        if (MyWiFiConfig.APSTA == true) {
            temp += "</tr></th><td>WiFi Password: </td><td>";
            temp += "<input type='password' name='APPW' maxlength='" + String(WIFI_PASSWORD_LENGTH - 1) +
                    "' size='30' value='" +
                    String(MyWiFiConfig.WiFiPwd) + "'> </td>";
            temp += "</tr></th><td>Repeat WiFi Password: </td>";
            temp += "<td><input type='password' name='APPWRepeat' maxlength='" + String(WIFI_PASSWORD_LENGTH - 1) +
                    "' size='30' value='" + String(MyWiFiConfig.WiFiPwd) + "'> </td>";
        } else {
            temp += "</tr></th><td>WiFi Password: </td><td>";
            temp += "<input type='password' name='APPW' maxlength='" + String(WIFI_PASSWORD_LENGTH - 1) +
                    "' size='30'> </td>";
            temp += "</tr></th><td>Repeat WiFi Password: </td>";
            temp += "<td><input type='password' name='APPWRepeat' maxlength='" + String(WIFI_PASSWORD_LENGTH - 1) +
                    "' size='30'> </td>";
        }
        temp += "</table>";
        if (MyWiFiConfig.PwDReq) {
            temp += "<input type='checkbox' name='PasswordReq' checked> Password for Login required. ";
        } else {
            temp += "<input type='checkbox' name='PasswordReq' > Password for Login required. ";
        }
        if (MyWiFiConfig.CapPortal) {
            temp += "<input type='checkbox' name='CaptivePortal' checked> Activate Captive Portal";
        } else {
            temp += "<input type='checkbox' name='CaptivePortal' > Activate Captive Portal";
        }
        temp += "<br></tr></th></table><br> <button type='submit' name='Settings' value='1' style='height: 50px; width: 140px' autofocus>Set WiFi Settings</button>";
        temp += "<button type='submit' name='Reboot' value='1' style='height: 50px; width: 200px' >Reboot System</button>";
        temp += "<button type='reset' name='action' value='1' style='height: 50px; width: 100px' >Reset</button></form>";
        temp += "<table border=2 bgcolor = white width = 500 cellpadding =5 ><caption><p><h3>Sytemlinks:</h2></p></caption><tr><th><br>";
        temp += "<a href='/'>Main Page</a><br><br></th></tr></table><br><br>";
        //temp += "<footer><p>Programmed and designed by: Tobias Kuch</p><p>Contact Information: <a href='mailto:tobias.kuch@googlemail.com'>tobias.kuch@googlemail.com</a>.</p></footer>";
        temp += "</body></html>";
        server.setContentLength(temp.length());
        server.send(200, "text/html", temp);
        server.client().stop(); // Stop is needed because we sent no content length
    }
}

void doReboot() {
    WiFi.disconnect();
    delay(1000);
    pinMode(D6, OUTPUT);
    digitalWrite(D6, LOW);
}

boolean configureWifi() {
    String temp = "";
    // STA Station Mode Connect to another WIFI Station
    if (server.hasArg("WiFiMode")) {
        if ((server.arg("WiFiMode") == "1")) {

            // Connect to existing STATION
            if (server.arg("WiFi_Network").length() > 0) {
                Serial.println("Configuring STA Mode");
                MyWiFiConfig.APSTA = false; // Access Point or Station Mode - false Station Mode
                memset(MyWiFiConfig.APSTAName, '\0', ACCESSPOINT_NAME_LENGTH);

                String networkName = server.arg("WiFi_Network");
                unsigned int networkNamelength = networkName.length();
                for (unsigned int i = 0; i < networkNamelength; i++) {
                    MyWiFiConfig.APSTAName[i] = temp[i];
                }
                memset(MyWiFiConfig.WiFiPwd, '\0', WIFI_PASSWORD_LENGTH);
                String serverPassword = server.arg("STAWLanPW");
                unsigned int passwordLength = serverPassword.length();
                for (unsigned int i = 0; i < passwordLength; i++) {
                    if (temp[i] > 32) //Steuerzeichen raus
                    {
                        MyWiFiConfig.WiFiPwd[i] = temp[i];
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
                    temp = "Daten des STA Modes erfolgreich gespeichert. ";
                } else {
                    temp = "Daten des STA Modes fehlerhaft.";
                }
                server.send(200, "text/html", temp);
                server.sendContent(temp);
                delay(2000);
                server.client().stop();
                server.stop();
                temp = "";
                WiFi.disconnect();
                WiFi.softAPdisconnect(true);
                delay(500);
                // ConnectWifiAP
                pinMode(D6, OUTPUT);
                digitalWrite(D6, LOW);
                int wifiConnectResult = ConnectWifiAP();

                // 4: WL_CONNECT_FAILED - Password is incorrect 1: WL_NO_SSID_AVAILin - Configured SSID cannot be reached
                if ( wifiConnectResult != 3) {
                    Serial.printf("Err STA Result: %i", wifiConnectResult);
                    server.client().stop();
                    delay(100);
                    WiFi.setAutoReconnect(false);
                    delay(100);
                    WiFi.disconnect();
                    delay(1000);
                    pinMode(D6, OUTPUT);
                    digitalWrite(D6, LOW);
                    //                    SetDefaultWiFiConfig();
                    CreateWifiSoftAP();
                } else {
                    // Safe Config
                    InitalizeHTTPServer();
                }
            }
            return false;
        } else {
            unsigned int passwordLength;
            // Configure Access Point
            String apName = server.arg("APPointName");
            unsigned int apNameLength = apName.length();
            String password = server.arg("APPW");
            if (server.hasArg("PasswordReq")) {
                passwordLength = password.length();
            } else {
                passwordLength = 0;
            }

            if ((apNameLength > 1) and (password == server.arg("APPWRepeat")) and (passwordLength > 7)) {
                temp = "";
                Serial.println("Configuring APMode");
                MyWiFiConfig.APSTA = true; // Access Point or Sation Mode - true AP Mode

                MyWiFiConfig.CapPortal = server.hasArg("CaptivePortal");

                MyWiFiConfig.PwDReq = server.hasArg("PasswordReq");

                // reset AP Name
                memset(MyWiFiConfig.APSTAName, '\0', ACCESSPOINT_NAME_LENGTH);

                for (unsigned int i = 0; i < apNameLength; i++) {
                    MyWiFiConfig.APSTAName[i] = temp[i];
                }

                memset(MyWiFiConfig.WiFiPwd, '\0', WIFI_PASSWORD_LENGTH);
                for (unsigned int i = 0; i < passwordLength; i++) {
                    MyWiFiConfig.WiFiPwd[i] = temp[i];
                }
                if (saveCredentials()) // Save AP ConfigCongfig
                {
                    temp = "Daten des AP Modes erfolgreich gespeichert. Reboot notwendig.";
                } else {
                    temp = "Daten des AP Modes fehlerhaft.";
                }
                server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
                server.sendHeader("Pragma", "no-cache");
                server.sendHeader("Expires", "-1");
                server.setContentLength(temp.length());
                server.send(200, "text/plain", temp );
                Serial.println(temp);
                doReboot();
                return false;
            } else if (server.arg("APPW") != server.arg("APPWRepeat")) {
                temp = "WLAN Passwort nicht gleich. Abgebrochen.";
            } else {
                temp = "WLAN Passwort oder AP Name zu kurz. Abgebrochen.";
            }
            if (temp.length()>0) {
                // HTML Header
                server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
                server.sendHeader("Pragma", "no-cache");
                server.sendHeader("Expires", "-1");
                server.setContentLength(temp.length());
                server.send(417, "text/plain", temp );
                Serial.println(temp);
                return false;
            }
            // End WifiAP
        }
    }
    return true;
}

void loop() {
    if (SoftAccOK) {
        dnsServer.processNextRequest(); //DNS
    }
    //HTTP
    server.handleClient();
    yield();
}
