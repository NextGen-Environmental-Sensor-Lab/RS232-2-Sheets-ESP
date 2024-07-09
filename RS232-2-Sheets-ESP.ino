/*
  This code takes an RS232 stream and uploads data to Google Sheets via wifi. 
  Originally for the 2BTech 205 Ozone monitor but may work with any device that outputs RS232 ascii, with mods possibly
  Device I made uses a Wemos D1 Mini and a MAX3232 for rs232-to-3.3v logic translation.
  Will prob work fine with any ESP board. 
  It will provision wifi SSID and PASSCODE, and GOOGLESCRIPTID through Acces Point and a simple Web Server
  It will save setting in EEPROM
  It needs a google script running to get the data and put in google sheets. See file *.gs

  Much of this comes from https://github.com/electronicsguy/HTTPSRedirect

  NGENS LAB, ASRC ESI GC CUNY. Ricardo Toledo-Crow 2023

*/

#include "Provisioning.h"

extern SoftwareSerial swSerial;
extern settings_t settings;
extern HTTPSRedirect* client;
extern char* host;
extern String url;
extern String payload_prefix;
extern String payload_suffix;
extern String payload;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(8000);
  Serial.println();
  Serial.println(__FILE__);

  swSerial.begin(2400);  //Initialize software serial with 2400 baud for '205 Ozone Monitor'

  Serial.println("\nESP8266. rs232 to Google Sheets via wifi.");
  Serial.println("Solid light = Starting");
  Serial.println("Fast blink = 15s to go into Provisioning by joining wifi AP 'csl-clearEEPROMxxx'");
  Serial.println("Slow blink = Provisioning. Join wifi 'csl_espxxx', open webpage at 192.168.4.1");
  Serial.println("Off = Connected to wifi\n");

  clearSettings();  // gives 15s to clear eeprom by connecting to ssid cleareeprom-MAC. This forces provisioning

  connectToWifi();  // gets settings from eeprom or provisioning, and connects to wifi

  // make GoogleSheet url with GSID from settings
  url = String("/macros/s/") + String(settings.saved_gsid) + "/exec?cal";

  delete client;
  client = nullptr;
  clientConnect();
  delay(1000);
  while (swSerial.available() > 0) swSerial.read();  // clean out the serial
}

void loop() {
  char rs232Buffer[128] = "";
  int indx = 0;
  char tmpr = 0x00;

  while (swSerial.available() > 0) {  // if data from rs232 available get it
    tmpr = swSerial.read();
    rs232Buffer[indx++] = tmpr;
    Serial.write(tmpr);  // put it to serial monitor
    delay(10);
  }

  if (tmpr == 0x0A) {           // if end of line from rs232 send to Google Sheets
    rs232Buffer[indx - 2] = 0;  // terminate as c string with a zero

    Serial.println("POST: append data to spreadsheet:");
    payload = payload_prefix + String(rs232Buffer) + payload_suffix;

    indx = 0;
    while (!client->connected()) {
      Serial.println("reconnecting to host...");
      delete client;
      client = nullptr;
      clientConnect();
      delay(1000);
      if (indx++ > 3) // try 3 times
        break;
    };

    if (!client->POST(url, host, payload)) {
      Serial.println("Error connecting to host");
      ESP.restart();  // reboot the system
      // while(1); // lock the system
    }
  }

  // we also send data to the rs232 device if available
  while (Serial.available() > 0) {  // if data avail from serial monitor get it
    char tmpt = Serial.read();
    if (tmpt == 0x0A)
      swSerial.write(0x0D);
    swSerial.write(tmpt);  // send it to rs232
  }
}
