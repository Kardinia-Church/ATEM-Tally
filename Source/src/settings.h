/*
  ATEM Tally by Kardinia Church 2020
  A simple tally light that shows an input's tally state using NodeRed as a server

  https://github.com/Kardinia-Church/ATEM-Tally

  settings.h The settings file for the tally
*/

int inputID = 1;                            //The input id
const char* ssid = "";                      //The wifi SSID to connect to
const char* password = "";                  //The wifi password
uint8_t ignoredMEs[3] = {2, 3, 4};          //What MEs to ignore. By default this is ME 2, 3, 4 (So ME1 will be the tally ME).
bool previewEnabled = true;                 //Should preview be enabled?
bool programEnabled = true;                 //Should program be enabled?
bool keyerDSEnabled = false;                //Should the DS keyer be enabled?
bool keyerUSEnabled = false;                //Should the US keyer be enabled
bool showBlue = false;                      //Should a blue led appear on the stage led to indicate location?

//Colors Green, Red, Blue
int liveColor = strip.Color(0, 255, 0);     //What colour should live be?
int keyerColor = strip.Color(120, 255, 0);  //What colour should live on a keyer be?
int standbyColor = strip.Color(255, 0, 0);  //What colour should preview/standby be?
int blueColor = strip.Color(0, 0, 255);     //What colour should the indicator be?
int offColor = strip.Color(0, 0, 0);        //What colour should off be?