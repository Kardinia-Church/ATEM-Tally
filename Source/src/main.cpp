/*
  ATEM Tally by Kardinia Church 2020
  A simple tally light that shows an input's tally state using NodeRed as a server

  https://github.com/Kardinia-Church/ATEM-Tally

  main.cpp file responsible for the main entry point and code functions
*/

#define VERSION strip.Color(255, 0, 255)

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define SERIAL_BAUD 115200
#define INCOMING_PORT 5657
#define OUTGOING_PORT 8001
#define DEBUG false
unsigned long thisMs = 0;

#define CMD_SUBSCRIBE 0xAF
#define CMD_PING 0xFA
#define CMD_PROGRAM 0x01
#define CMD_PREVIEW 0x02
#define CMD_DSKEY 0x03
#define CMD_USKEY 0x04

WiFiUDP udp;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(2, 4, NEO_GRB + NEO_KHZ800);
IPAddress serverIP(255,255,255,255);
unsigned long lastMessage = millis();
bool programTally = false;
bool previewTally = false;
bool usKeyTally = false;
bool dsKeyTally = false;
bool pingSent = false;

#include "settings.h"

//Send a request to subscribe
void subscribe() {
  Serial.println("Subscribe");
  //Subscribe cmd, inputId, ignoreME(s)
  udp.beginPacket(serverIP, OUTGOING_PORT);
  uint8_t buffer[(sizeof(ignoredMEs) / sizeof(ignoredMEs[0])) + 2] = {CMD_SUBSCRIBE, inputID};
  for(int i = 0; i < (sizeof(ignoredMEs) / sizeof(ignoredMEs[0])); i++) {
    buffer[i + 2] = ignoredMEs[i];
  }

  udp.write(buffer, (sizeof(ignoredMEs) / sizeof(ignoredMEs[0])) + 4);
  udp.endPacket();
  lastMessage = millis();
}

//Send a ping request
void ping() {
  pingSent = true;
  Serial.println("Send ping");
  udp.beginPacket(serverIP, OUTGOING_PORT);
  uint8_t buffer[1] = {CMD_PING};
  udp.write(buffer, 1);
  udp.endPacket();
  lastMessage = millis();
}

//Process the incoming packet
byte packetBuffer[UDP_TX_PACKET_MAX_SIZE];
void receivePacket() {
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    udp.read(packetBuffer, packetSize);
    switch(packetBuffer[0]) {
      case CMD_PROGRAM: {
        programTally = packetBuffer[1] == 0x01;
        pingSent = false;
        lastMessage = millis();
        break;
      }
      case CMD_PREVIEW: {
        previewTally = packetBuffer[1] == 0x01;
        pingSent = false;
        lastMessage = millis();
        break;
      }
      case CMD_DSKEY: {
        dsKeyTally = packetBuffer[1] == 0x01;
        pingSent = false;
        lastMessage = millis();
        break;
      }
      case CMD_USKEY: {
        usKeyTally = packetBuffer[1] == 0x01;
        pingSent = false;
        lastMessage = millis();
        break;
      }
      case CMD_PING: {
        pingSent = false;
        lastMessage = millis();
        break;
      }
    }

    //Do colours
    if((dsKeyTally && keyerDSEnabled) || (usKeyTally && keyerUSEnabled)) {
      //Tally colour
      strip.setPixelColor(0, keyerColor);
      strip.setPixelColor(1, keyerColor);
    }
    else if(programTally && programEnabled) {
      //Program colour
      strip.setPixelColor(0, liveColor);
      strip.setPixelColor(1, liveColor);
    }
    else if(previewTally && previewEnabled) {
      //Preview colour
      strip.setPixelColor(0, standbyColor);
      if(showBlue) {strip.setPixelColor(1, blueColor);}else{strip.setPixelColor(1, offColor);}
    }
    else if(showBlue) {
      //Blue colour
      strip.setPixelColor(0, offColor);
      strip.setPixelColor(1, blueColor);
    }
    else {
      //All off
      strip.setPixelColor(0, offColor);
      strip.setPixelColor(1, offColor);
    }
  }
  else if(lastMessage + 1000 < millis()) {
    if(pingSent) {
      strip.setPixelColor(0, blueColor);
      strip.setPixelColor(1, blueColor);
      subscribe();
    }
    else {
      ping();
    }
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Connecting to " + String(ssid));
  strip.begin();
  strip.setPixelColor(0, VERSION);
  strip.setPixelColor(1, offColor);
  strip.show();
  delay(2000);
  WiFi.begin(ssid, password);
  WiFi.softAPdisconnect(true);
  
  int i = 0;
  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if(i == 0) {
         strip.setPixelColor(0, offColor);
         i = 1;
    }
    else{
         strip.setPixelColor(0, blueColor);
         i = 0;

         //If we have failed many times restart
         if(count++ > 20) {
           ESP.restart();
         }
    }
    strip.show();
    delay(500);
    Serial.print(".");
  }

  //Show the input id flashes = input id
  strip.setPixelColor(0, offColor);
  strip.show();
  delay(1000);  
  for(int i = 0; i < inputID; i++) {
      strip.setPixelColor(0, liveColor);
      strip.show();
      delay(200);
      strip.setPixelColor(0, offColor);
      strip.show();
      delay(400);
  }
 
  strip.setPixelColor(0, standbyColor);
  strip.show();
  delay(1000);

  udp.begin(INCOMING_PORT);
  subscribe();
  strip.setPixelColor(0, offColor);
  if(showBlue) {strip.setPixelColor(1, blueColor);}else{strip.setPixelColor(1, offColor);}
  Serial.print("My IP:");
  Serial.println(WiFi.localIP());
  Serial.println("Done");
}

void loop() {
  //Check the wifi status
  if(WiFi.status() != WL_CONNECTED) {
    ESP.restart();
  }

   receivePacket();
   strip.show();
}