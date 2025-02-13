/* 
  Code to provision settings SSID, PASSCODE, GSID

  NGENS LAB, ASRC ESI GC CUNY. Ricardo Toledo-Crow 2023

*/

#include "Provisioning.h"

const char *host = "script.google.com";
const int httpsPort = 443;
SoftwareSerial swSerial(D5, D6);  // softserial bcause esp8266 Serial1 only transmits :(
settings_t settings;              // struct to hold settings
ESP8266WebServer server(80);
WiFiServer wifiServer(80);
String url;  // to upload to google sheets with http
String payload_prefix = "{\"command\": \"appendRow\", \"sheet_name\": \"Sheet1\", \"values\": \"";
String payload_suffix = "\"}";
String payload = "";
HTTPSRedirect *client = nullptr;

// when ipaddress entered run this
void handlePRoot() {

  server.send(200, "text/html", webpage_html);
}

// when webpage sends info on clicking submit, run this
void handleProvision() {

  // save settings in structure 'settings'
  strcpy(settings.saved_ssid, server.arg("SSID").c_str());
  strcpy(settings.saved_passcode, server.arg("passcode").c_str());
  strcpy(settings.saved_gsid, server.arg("GSID").c_str());

  Serial.printf("\nhandleProvision\nssid: %s\n", settings.saved_ssid);
  Serial.printf("passcode: %s\n", settings.saved_passcode);
  Serial.printf("gsid: %s\n", settings.saved_gsid);

  server.send(200, "text/html", "<p>Provision successful. To join ssid: " + String(settings.saved_ssid) + " exit Access Point</p>");

  WiFi.softAPdisconnect(true);  // kick anyone on the AP off to exit loop
  delay(500);
  Serial.printf("Client disconnected\n");
  server.close();  //????????????????????
  delay(500);
}

// Will try to connect to Wifi and if it can't it will go into provisioning
void connectToWifi() {

  bool firstTry = true;
  // get settings from eeprom into struct
  loadStruct(&settings, sizeof(settings));

  while (true) {  // loop to get

    if (isValidSettings(settings)) {  // check settings dont have non-graphic chars

      WiFi.begin(settings.saved_ssid, settings.saved_passcode);
      Serial.printf("\nConnecting to ssid: %s\n", settings.saved_ssid);

      int startTime = millis();  // timeout of 20s
      while ((WiFi.status() != WL_CONNECTED) && (millis() - startTime <= 20000)) {
        //
        Serial.printf(".");
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
        digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
        delay(500);
      }

      if (WiFi.status() == WL_CONNECTED) {  // successfully connected
        Serial.printf("\nConnected to %s\n", settings.saved_ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        if (!firstTry)
          // if it wasn't on first try we entered new settings so save them to eeprom
          storeStruct(&settings, sizeof(settings));
        break;

      } else {
        // didn't connect because settings invalid so lets get new settings
        Serial.printf("Could not connect to %s. Starting provisioning.\n", settings.saved_ssid);
      };
    } else {  // probably never had settings so lets get them
      Serial.printf("Settings not valid. Starting provisioning.\n");
    }

    firstTry = false;
    // if we get here we enter provisioning mode
    APprovision();
    delay(1000);
  }
}

// Access Point provisioning. Makes AP, when client connected serves a simple webpage
// to get settings.
void APprovision() {

  // makes a unique ssid for the ap with the id of the chip
  String ssid = makeMACssidAP("csl_esp-");
  Serial.printf("In provisioning mode. Connect to ssid: %s\n", ssid.c_str());

  /*  By default the local IP address of will be 192.168.4.1
*   you can override it with the following:
*   WiFi.softAPConfig(local_ip(192,168,4,22));
*/
  // start AP
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();

  // set up server
  server.on("/", handlePRoot);                          // go here when conencted
  server.on("/provision", HTTP_POST, handleProvision);  // go here when info submitted

  server.begin();
  Serial.println("HTTP server started");

  if (!WiFi.softAPgetStationNum())
    Serial.printf("no one connected\n");

  // wait for someone to connect to AP with slow blink
  while (!WiFi.softAPgetStationNum()) {
    Serial.printf("_");
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    Serial.printf("|");
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    delay(1000);
  }

  // someone connected now go to a browser and enter the IP
  Serial.printf("\nConnected to AP! In a browser go to ");
  Serial.println(myIP);

  if (WiFi.softAPgetStationNum())
    Serial.printf("Waiting for client to submit settings.\n");

  while (WiFi.softAPgetStationNum()) {
    server.handleClient();
    Serial.printf("_");
    digitalWrite(LED_BUILTIN, LOW);
    delay(300);
    Serial.printf("|");
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    delay(800);
  }
  delay(1000);
}

// gives you 15s to connect to ssid: csl-clearEEPROM to force provisioning
void clearSettings() {

  String ssid = makeMACssidAP("csl-clearEEPROM-");  // unique ssid with cleareeprom in it
  Serial.printf("Connect to ssid %s in <10s to clear EEPROM\n", ssid.c_str());
  WiFi.softAP(ssid);

  // wait for someone to join ap with fast blink
  int startTime = millis();
  while ((!WiFi.softAPgetStationNum()) && (millis() - startTime <= 15000)) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }
  // if we didn't timeout then erase eeprom
  if (WiFi.softAPgetStationNum()) {
    Serial.printf("\nErasing EEPROM\n");
    EEPROM.begin(512);
    for (int i = 0; i < 512; i++) { EEPROM.write(i, 0); }
    EEPROM.end();
  }
}

/**
*   Create an SSID  formed with a string
*   and the last 2 hex digits of the board MAC address.
*
*   @param startString a string to preface the ssid
*/
String makeMACssidAP(String startString) {
  byte localMac[6];
  // Serial.print(F("Device MAC address: "));
  WiFi.macAddress(localMac);
  // printMacAddress(localMac);
  char myHexString[5];
  sprintf(myHexString, "%02X%02X", localMac[1], localMac[0]);
  return startString + String((char *)myHexString);
}

/**
* print formatted MAC address to Serial
*
*/
void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16)
      Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i > 0)
      Serial.print(":");
  }
  Serial.println();
}

// Write struct to eeprom
// from https://github.com/esp8266/Arduino/issues/1539#issue-130025977
void storeStruct(void *data_source, size_t size) {
  EEPROM.begin(size * 2);
  for (size_t i = 0; i < size; i++) {
    char data = ((char *)data_source)[i];
    EEPROM.write(i, data);
  }
  EEPROM.commit();
}
// Read from eeprom into struct
void loadStruct(void *data_dest, size_t size) {
  EEPROM.begin(size * 2);
  for (size_t i = 0; i < size; i++) {
    char data = EEPROM.read(i);
    ((char *)data_dest)[i] = data;
  }
}

// Check if string is valid because only printable chars in it
bool isValidString(char *b, int length) {
  if ((length == 0) || (b[0] == 0))
    return false;
  for (int i = 0; b[i] != '\0'; i++) {
    if ((b[i] > 126) || (b[i] < 32) || (i > length)) {
      return false;
    }
  }
  return true;
}

// check all settings are valid
bool isValidSettings(settings_t mysettings) {
  if (isValidString(mysettings.saved_ssid, sizeof(mysettings.saved_ssid))) {
    Serial.print("valid ssid: ");
    Serial.println(mysettings.saved_ssid);
  } else {
    Serial.println("saved ssid not valid");
    return false;
  }
  if (isValidString(mysettings.saved_passcode, sizeof(mysettings.saved_passcode))) {
    Serial.print("valid passcode: ");
    Serial.println(mysettings.saved_passcode);
  } else {
    Serial.println("saved passcode not valid");
    return false;
  }
  if (isValidString(mysettings.saved_gsid, sizeof(mysettings.saved_gsid))) {
    Serial.print("valid gsid: ");
    Serial.println(mysettings.saved_gsid);
  } else {
    Serial.println("saved gsid not valid");
    return false;
  }
  return true;
}

/**
* print wifi relevant info to Serial
*
*/
void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.softAPIP();
  Serial.print(F("AP IP Address: "));
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}

// this from  *  Author: Sujay Phadke *  Github: @electronicsguy
bool clientConnect() {
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  Serial.print("Connecting to ");
  Serial.println(host);

  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(host, httpsPort);
    delay(100);
    if (retval == 1) {
      flag = true;
      break;
    } else {
      Serial.printf("Connection failed. Retrying... return value %d\n",retval);
      delay(200);
    }
  }
  if (!flag) {
    Serial.printf("Could not connect to server: %s\n Exiting...\n", host);
    return false;
  } else {
    Serial.print("Connected to host!\n");
    return true;
  }
}
