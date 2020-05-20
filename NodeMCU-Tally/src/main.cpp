/*
 * Camera Tally Light
 * Connect LEDs to pin D2 on the node mcu
 * 
 * Version Purple
 */
#define VERSION strip.Color(0, 255, 255);

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

int inputID = 3;

#define SERIAL_BAUD 115200
#define INCOMING_PORT 5657
#define OUTGOING_PORT 8001

#define WIFI_STA_SSID ""
#define WIFI_STA_PSWD ""
#define DEBUG false

unsigned long thisMs = 0;

#define ME_COUNT 4
uint8_t ignoredMEs[ME_COUNT] = {1, 2, 3, -1};
#define CMD_SUBSCRIBE 0xAF
#define CMD_PING 0xFA
#define CMD_PROGRAM 0x01
#define CMD_PREVIEW 0x02
#define CMD_DSKEY 0x03
#define CMD_USKEY 0x04

const bool previewEnabled = true;
const bool programEnabled = true;
const bool keyerDSEnabled = false;
const bool keyerUSEnabled = false;
const bool showBlue = false;

unsigned long lastMessage = millis();

WiFiUDP udp;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(2, 4, NEO_GRB + NEO_KHZ800);
IPAddress serverIP(255,255,255,255);
bool programTally = false;
bool previewTally = false;
bool usKeyTally = false;
bool dsKeyTally = false;
bool pingSent = false;

//G R B
int version = VERSION
int liveColor = strip.Color(0, 255, 0);
int keyerColor = strip.Color(120, 255, 0);
int standbyColor = strip.Color(255, 0, 0);
int blueColor = strip.Color(0, 0, 255);
int offColor = strip.Color(0, 0, 0);

//Send a request to subscribe
void subscribe() {
  Serial.println("Subscribe");
  //Subscribe cmd, inputId, ignoreME(s)
  udp.beginPacket(serverIP, OUTGOING_PORT);
  uint8_t buffer[ME_COUNT + 2] = {CMD_SUBSCRIBE, inputID};
  for(int i = 0; i < ME_COUNT; i++) {
    buffer[i + 2] = ignoredMEs[i];
  }

  udp.write(buffer, ME_COUNT + 4);
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
  Serial.println("Connecting to " + String(WIFI_STA_SSID));
  strip.begin();
  strip.setPixelColor(0, version);
  strip.setPixelColor(1, offColor);
  strip.show();
  delay(2000);
  WiFi.begin(WIFI_STA_SSID, WIFI_STA_PSWD);
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