# RS232-2-Sheets-ESP
Upload ASCII serial stream from rs232 to Google Sheets with WiFi using ESP8266

This code takes an RS232 stream and uploads data to Google Sheets via wifi.

Originally for the 2BTech 205 Ozone monitor but may work with any device that outputs RS232 ascii, with mods possibly.

Device I made uses a Wemos D1 Mini and a MAX3232 for rs232-to-3.3v logic translation.

Will prob work fine with any ESP board. 

It will provision wifi SSID and PASSCODE, and GOOGLESCRIPTID through Acces Point and a simple Web Server

It will save setting in EEPROM

It needs a google script running to get the data and put in google sheets. See file *.gs

Much of this comes from https://github.com/electronicsguy/HTTPSRedirect

NGENS LAB, ASRC ESI GC CUNY. Ricardo Toledo-Crow 2023
