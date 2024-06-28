# RS232-2-Sheets-ESP
 - This project uploads an RS232 ASCII serial stream to Google Sheets via
   WiFi on an ESP8266.
 - Originally made for the 2BTech 205 Ozone monitor but may work with any
   device that outputs RS232 ascii, with mods possibly.
 - The device I made uses a Wemos D1 Mini and a MAX3232 for rs232-to-3.3v
   logic translation.
 - It will prob work fine with any ESP board. 
 - It has a settings provisioning mode for WiFi SSID and PASSCODE, and GOOGLESCRIPT_ID through
   an Access Point and a simple Web Server
 - It will save setting in EEPROM
 - It uses a Google Script deployed to get the data through HTTP and put into the Google Sheet. See file *.gs.
 - Much of this comes from https://github.com/electronicsguy/HTTPSRedirect

![photo](images/IMG_1954.jpeg)

![sketch](images/RS232-2-Sheets-ESP.jpg)


