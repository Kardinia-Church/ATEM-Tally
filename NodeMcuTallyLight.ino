/*
 * Camera Tally Light
 * Connect LEDs to pin D2 on the node mcu
 * 
 * Version Blue
 */
#define VERSION strip.Color(0, 0, 255);

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define WIFI_STA_SSID ""
#define WIFI_STA_PSWD ""
#define DEBUG false

const int dstPort = 5654;
unsigned long thisMs = 0;

const int inputID = 4;
const bool preview = false;
const bool program = true;
const bool keyer = false;
const bool showBlue = false;

WiFiUDP udp;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(2, 4, NEO_GRB + NEO_KHZ800);
IPAddress serverIP(10,4,10,15);

//G R B
int version = VERSION
int liveColor = strip.Color(0, 255, 0);
int keyerColor = strip.Color(120, 255, 0);
int standbyColor = strip.Color(255, 0, 0);
int blueColor = strip.Color(0, 0, 255);
int offColor = strip.Color(0, 0, 0);

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to " + String(WIFI_STA_SSID));
  strip.begin();
  strip.setPixelColor(0, version);
  strip.setPixelColor(1, offColor);
  strip.show();
  delay(2000);
  WiFi.begin(WIFI_STA_SSID, WIFI_STA_PSWD);
  WiFi.softAPdisconnect(true);
  
  int i = 0;
  while (!WiFi.isConnected()) {
    if(i == 0) {
         strip.setPixelColor(0, offColor);
         i = 1;
    }
    else{
         strip.setPixelColor(0, blueColor);
         i = 0;
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

  udp.begin(dstPort);
  sendRequest();
  strip.setPixelColor(0, offColor);
  if(showBlue) {strip.setPixelColor(1, blueColor);}else{strip.setPixelColor(1, offColor);}
  Serial.print("My IP:");
  Serial.println(WiFi.localIP());
  Serial.println("Done");
}

void loop() {
   receivePacket();
   strip.show();
}

//Send a request to get status of ME
void sendRequest() {
  char requestPacket[] = "dataRequest";
  udp.beginPacket(serverIP, 8000);
  udp.write(requestPacket);
  udp.endPacket();
}

//Process the incoming packet
byte packetBuffer[UDP_TX_PACKET_MAX_SIZE];
void receivePacket() {
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    //Read in the packet
    udp.read(packetBuffer, packetSize);


    //Find the position of the input in the buffer
    for(int i = 0; i < packetSize; i+=5) {
      if((packetBuffer[i] << 8) | (packetBuffer[i + 1] & 0xff) == inputID) {
        boolean programTally = (packetBuffer[i + 2] == 0x01);
        boolean previewTally = (packetBuffer[i + 3] == 0x01);
        boolean keyerTally = (packetBuffer[i + 4] == 0x01);
        Serial.println("Got Update!");
        Serial.print("PROG: ");
        Serial.println(programTally);
        Serial.print("PREV: ");
        Serial.println(previewTally);
        Serial.print("KEY: ");
        Serial.println(keyerTally);

        boolean liveTally = (programTally && program);
        boolean keyTally = (keyerTally && keyer);
        boolean standbyTally = (previewTally && preview);
  
        if(keyTally) {
          strip.setPixelColor(0, keyerColor);
          strip.setPixelColor(1, keyerColor);
        }
        else if(liveTally) {
          strip.setPixelColor(0, liveColor);
          strip.setPixelColor(1, liveColor);
        }
        else if(standbyTally && preview) {
          strip.setPixelColor(0, standbyColor);
          if(showBlue) {strip.setPixelColor(1, blueColor);}else{strip.setPixelColor(1, offColor);}
        }
        else {
          strip.setPixelColor(0, offColor);
          if(showBlue) {strip.setPixelColor(1, blueColor);}else{strip.setPixelColor(1, offColor);}
        }  
        break;
      }
    }
  }
}