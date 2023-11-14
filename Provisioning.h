#ifndef PROVISIONING_H
#define PROVISIONING_H

#include <Arduino.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include "HTTPSRedirect.h"

typedef struct {
  boolean valid;
  char saved_ssid[64];
  char saved_passcode[64];
  char saved_gsid[128];
} settings_t;  //__attribute__((packed));

// This is the simple webpage we will serve to client to input settings
const char webpage_html[] PROGMEM = R"rawliteral(
</form>
<p>
  Community Sensor Lab provisioning
</p>
<form action="/provision" method="POST">
  <input type="text" name="SSID" placeholder="SSID"></br>
  <input type="password" name="passcode" placeholder="Passcode"></br>
  <input type="text" name="GSID" placeholder="GSID"></br>
  <input type="submit" value="Submit">
</form>
<p>
  text
</p>
)rawliteral";

bool clientConnect(void);
void handlePRoot(void);
void handleProvision(void);
void connectToWifi(void);
void APprovision(void);
void clearSettings(void);

String makeMACssidAP(String);
void printMacAddress(byte);
void storeStruct(void *, size_t);
void loadStruct(void *, size_t);
bool isValidString(char *, int);
bool isValidSettings(settings_t);
void printWiFiStatus(void);

#endif
