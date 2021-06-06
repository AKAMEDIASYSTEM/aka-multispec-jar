#include <SparkFun_Qwiic_Power_Switch_Arduino_Library.h>

/*
   AKA E-Ink ESP32 Starter Jar

   Monitor starter params


   Update to web when possible


   Accept "I fed you" user input buttonpress

   LED PIN is 19, active LOW
   Button pin is 39,
   Has an ADC on pin 35
   contrivance to power a mini fan via a qwiic power switch that powers a qwiic boost module

*/





/****************************************************************************************************************************
  Async_ConfigOnStartup.ino
  For ESP8266 / ESP32 boards

  ESPAsync_WiFiManager is a library for the ESP8266/Arduino platform, using (ESP)AsyncWebServer to enable easy
  configuration and reconfiguration of WiFi credentials using a Captive Portal.

  Modified from
  1. Tzapu               (https://github.com/tzapu/WiFiManager)
  2. Ken Taylor          (https://github.com/kentaylor)
  3. Alan Steremberg     (https://github.com/alanswx/ESPAsyncWiFiManager)
  4. Khoi Hoang          (https://github.com/khoih-prog/ESP_WiFiManager)

  Built by Khoi Hoang https://github.com/khoih-prog/ESPAsync_WiFiManager
  Licensed under MIT license
  Version: 1.6.1

  Version Modified By  Date      Comments
  ------- -----------  ---------- -----------
  1.0.11  K Hoang      21/08/2020 Initial coding to use (ESP)AsyncWebServer instead of (ESP8266)WebServer. Bump up to v1.0.11
                                  to sync with ESP_WiFiManager v1.0.11
  1.1.1   K Hoang      29/08/2020 Add MultiWiFi feature to autoconnect to best WiFi at runtime to sync with
                                  ESP_WiFiManager v1.1.1. Add setCORSHeader function to allow flexible CORS
  1.1.2   K Hoang      17/09/2020 Fix bug in examples.
  1.2.0   K Hoang      15/10/2020 Restore cpp code besides Impl.h code to use if linker error. Fix bug.
  1.3.0   K Hoang      04/12/2020 Add LittleFS support to ESP32 using LITTLEFS Library
  1.4.0   K Hoang      18/12/2020 Fix staticIP not saved. Add functions. Add complex examples.
  1.4.1   K Hoang      21/12/2020 Fix bug and compiler warnings.
  1.4.2   K Hoang      21/12/2020 Fix examples' bug not using saved WiFi Credentials after losing all WiFi connections.
  1.4.3   K Hoang      23/12/2020 Fix examples' bug not saving Static IP in certain cases.
  1.5.0   K Hoang      13/02/2021 Add support to new ESP32-S2. Optimize code.
  1.6.0   K Hoang      25/02/2021 Fix WiFi Scanning bug.
  1.6.1   K Hoang      26/03/2021 Modify multiWiFi-related timings to work better with latest esp32 core v1.0.6
 *****************************************************************************************************************************/
/****************************************************************************************************************************
   This example will open a configuration portal for 60 seconds when first powered up if the boards has stored WiFi Credentials.
   Otherwise, it'll stay indefinitely in ConfigPortal until getting WiFi Credentials and connecting to WiFi

   ConfigOnSwitch is a a bettter example for most situations but this has the advantage
   that no pins or buttons are required on the ESP32/ESP8266 device at the cost of delaying
   the user sketch for the period that the configuration portal is open.

   Also in this example a password is required to connect to the configuration portal
   network. This is inconvenient but means that only those who know the password or those
   already connected to the target WiFi network can access the configuration portal and
   the WiFi network credentials will be sent from the browser over an encrypted connection and
   can not be read by observers.
 *****************************************************************************************************************************/


// AKA NOTE - THIS LASTS 6.5H ON A 450MAH LIPO AS OF APRIL 2 2021



#if !( defined(ESP8266) ||  defined(ESP32) )
#error This code is intended to run on the ESP8266 or ESP32 platform! Please check your Tools->Board setting.
#endif

#define ESP_ASYNC_WIFIMANAGER_VERSION_MIN_TARGET     "ESPAsync_WiFiManager v1.6.1"

// Use from 0 to 4. Higher number, more debugging messages and memory usage.
#define _ESPASYNC_WIFIMGR_LOGLEVEL_    3

//For ESP32, To use ESP32 Dev Module, QIO, Flash 4MB/80MHz, Upload 921600

//Ported to ESP32
#ifdef ESP32
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClient.h>

// From v1.1.1
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

// LittleFS has higher priority than SPIFFS
#define USE_LITTLEFS    true
#define USE_SPIFFS      false
#if USE_LITTLEFS
// Use LittleFS
#include "FS.h"

// The library will be depreciated after being merged to future major Arduino esp32 core release 2.x
// At that time, just remove this library inclusion
#include <LITTLEFS.h>             // https://github.com/lorol/LITTLEFS

FS* filesystem =      &LITTLEFS;
#define FileFS        LITTLEFS
#define FS_Name       "LittleFS"
#elif USE_SPIFFS
#include <SPIFFS.h>
FS* filesystem =      &SPIFFS;
#define FileFS        SPIFFS
#define FS_Name       "SPIFFS"
#else
// Use FFat
#include <FFat.h>
FS* filesystem =      &FFat;
#define FileFS        FFat
#define FS_Name       "FFat"
#endif
//////

#define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

#define LED_BUILTIN       2
#define LED_ON            HIGH
#define LED_OFF           LOW

#else
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>

// From v1.1.0
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;

#define USE_LITTLEFS      true

#if USE_LITTLEFS
#include <LittleFS.h>
FS* filesystem = &LittleFS;
#define FileFS    LittleFS
#define FS_Name       "LittleFS"
#else
FS* filesystem = &SPIFFS;
#define FileFS    SPIFFS
#define FS_Name       "SPIFFS"
#endif
//////

#define ESP_getChipId()   (ESP.getChipId())

#define LED_ON      LOW
#define LED_OFF     HIGH
#endif

// include library, include base class, make path known
#include <GxEPD.h>
#include "SD.h"
#include "SPI.h"
#include <SparkFun_RFD77402_Arduino_Library.h>
#include <Adafruit_VCNL4040.h>
#include <Adafruit_VCNL4010.h>
#include "Adafruit_VL6180X.h"
#include "Adafruit_SGP30.h"
#include <Adafruit_SCD30.h>
Adafruit_SCD30 scd;
Adafruit_SGP30 sgp;
Adafruit_VCNL4010 vc;
Adafruit_VCNL4040 vcnl = Adafruit_VCNL4040();
Adafruit_VL6180X vl = Adafruit_VL6180X();
RFD77402 rfd;
QWIIC_POWER mySwitch;
bool SGP = false; // SGP30
bool VC = false; // vcnl4010
bool RFD = false; // sfe rfd ToF, best
bool VCNL = false; // adafruit vcnl4040
bool VL = true; // adafruit vl6180

// HEY HEY HEY IT'S Trotz Idee
/*
  · Operating voltage: 3.3V
  · Interface: 3-wire SPI, 4-wire SPI
  · Display color:  black, white
  · Grey level: 2
  · Full refresh time: 8s
  · Refresh power: 26.4mW(typ.)
  - Resolution 250x122
  Driver chip：SSD1680Z8
*/
#include <GxGDEH0213B73/GxGDEH0213B73.h>  // 2.13" b/w newer panel THIS ONE IS AKA LILYGO JAN 2021
// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>
#include <Fonts/FreeSansBoldOblique12pt7b.h>
#include <Fonts/FreeSansBoldOblique18pt7b.h>
#include <Fonts/FreeSansBoldOblique24pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerif24pt7b.h>
#include <Fonts/Picopixel.h>
#include <Fonts/Org_01.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#define SPI_MOSI 23
#define SPI_MISO -1
#define SPI_CLK 18

#define ELINK_SS 5
#define ELINK_BUSY 4
#define ELINK_RESET 16
#define ELINK_DC 17

#define SDCARD_SS 13
#define SDCARD_CLK 14
#define SDCARD_MOSI 15
#define SDCARD_MISO 2

#define BUTTON_PIN 39
#define FAN_PIN 12
#define MARGIN 5
#define SAMPLE_DEPTH 225  // this is determined by GRAPH_W
#define DISPLAY_UPDATE_LOOPS 3
#define SENSOR_MAX 2048
#define TICK_INTERVAL 15
// want screen to show last 3 hours -> 180 minutes, so if we sample every 1.25 minutes->75secs
// we'll have a series 225samples/pixels long, leaving enough for axes and borders
#define LOOP_DELAY_SECS 75  // every tick is 75 secs; 250ticks = 312.5mins = 5.2 hours
//#define LOOP_DELAY_SECS 15  // every tick is 15 secs; 250ticks = 62.5mins
//#define LOOP_DELAY_SECS 5  // every tick is 5 secs; 250ticks = 20.8mins
#define FAN_DURATION_MS 5000 // length of time DURING the sampler() function that fan runs before we read the sensors
#define VERBOSE 0
#define DEBUG 1
#define PORTAL_TIMEOUT 40  // host own AP for PORTAL_TIMEOUT seconds before joining wifi using any saved credentials

// Display consts
#define GRAPH_X 25
#define GRAPH_W 225
#define GRAPH_Y 40
#define GRAPH_H 40
#define NAME_X 0
#define NAME_W 150
#define NAME_Y 15
#define NAME_H 30
#define TREND_X 225
#define TREND_W 25
#define TREND_Y 25
#define TREND_H 50


#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

GxIO_Class io(SPI, /*CS=5*/ ELINK_SS, /*DC=*/ ELINK_DC, /*RST=*/ ELINK_RESET);
GxEPD_Class display(io, /*RST=*/ ELINK_RESET, /*BUSY=*/ ELINK_BUSY);
SPIClass sdSPI(VSPI);
int bmpWidth = 150, bmpHeight = 39;
//lilygo bmp is width:150,height:39
const unsigned char lilygo[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x31, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xfc, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0xfe, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0xff, 0x20, 0x7f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x07, 0xf8, 0x0f, 0xf0, 0x00, 0xfe, 0x00, 0x03, 0xff, 0x80, 0x19, 0xe7, 0x30, 0x7f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x07, 0xfc, 0x0f, 0xf0, 0x07, 0xff, 0xc0, 0x0f, 0xff, 0xe0, 0x19, 0xe7, 0xb0, 0x7f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x03, 0xfc, 0x1f, 0xe0, 0x0f, 0xff, 0xe0, 0x1f, 0xff, 0xf8, 0x19, 0xff, 0x10, 0x7f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x03, 0xfe, 0x1f, 0xe0, 0x1f, 0xff, 0xf0, 0x3f, 0xff, 0xfc, 0x19, 0xff, 0x10, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x03, 0xfe, 0x1f, 0xc0, 0x3f, 0xff, 0xf0, 0x7f, 0xff, 0xfe, 0x19, 0xfe, 0x10, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x01, 0xfe, 0x3f, 0xc0, 0x7f, 0xff, 0xe0, 0x7f, 0xff, 0xfe, 0x19, 0xfe, 0x10, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x01, 0xff, 0x3f, 0x80, 0xff, 0xc7, 0xc0, 0xff, 0xff, 0xff, 0x1d, 0xfe, 0x10, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x00, 0xff, 0x7f, 0x80, 0xff, 0x81, 0x80, 0xff, 0xef, 0xff, 0x1d, 0xef, 0x00, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0xc3, 0xff, 0x8f, 0xef, 0x00, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x00, 0x7f, 0xff, 0x01, 0xff, 0x00, 0x01, 0xff, 0xc3, 0xff, 0x8f, 0x87, 0x80, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x00, 0x7f, 0xfe, 0x01, 0xfe, 0x00, 0x01, 0xff, 0xc1, 0xff, 0x87, 0x81, 0xc0, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x00, 0x7f, 0xfe, 0x01, 0xfe, 0x1f, 0x81, 0xff, 0x81, 0xff, 0x83, 0xff, 0x80, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x00, 0x3f, 0xfc, 0x01, 0xfe, 0x3f, 0xf9, 0xff, 0x81, 0xff, 0x80, 0xfe, 0x00, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x00, 0x3f, 0xfc, 0x01, 0xfe, 0x3f, 0xf9, 0xff, 0x81, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x00, 0x1f, 0xf8, 0x01, 0xfe, 0x3f, 0xf9, 0xff, 0x81, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x00, 0x1f, 0xf0, 0x01, 0xff, 0x3f, 0xf9, 0xff, 0xc1, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x00, 0x0f, 0xf0, 0x01, 0xff, 0x3f, 0xf8, 0xff, 0xc1, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x00, 0x0f, 0xf0, 0x00, 0xff, 0x9f, 0xf8, 0xff, 0xc1, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xc0, 0x03, 0xfc, 0x7f, 0x80, 0x00, 0x0f, 0xf0, 0x00, 0xff, 0x83, 0xf0, 0xff, 0xe1, 0xff, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x03, 0xfc, 0x7f, 0xf8, 0x00, 0x0f, 0xf0, 0x00, 0xff, 0xe3, 0xf0, 0x7f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xe3, 0xfc, 0x7f, 0xff, 0xc0, 0x0f, 0xf0, 0x00, 0x7f, 0xff, 0xf0, 0x7f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xe3, 0xfc, 0x7f, 0xff, 0xc0, 0x0f, 0xf0, 0x00, 0x7f, 0xff, 0xf0, 0x3f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xe3, 0xfc, 0x7f, 0xff, 0xc0, 0x0f, 0xf0, 0x00, 0x3f, 0xff, 0xf0, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xe3, 0xfc, 0x7f, 0xff, 0xc0, 0x0f, 0xf0, 0x00, 0x1f, 0xff, 0xf0, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xe3, 0xf8, 0x7f, 0xff, 0xc0, 0x0f, 0xf0, 0x00, 0x0f, 0xff, 0xf0, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xc3, 0xf8, 0x1f, 0xff, 0xc0, 0x0f, 0xe0, 0x00, 0x03, 0xff, 0xe0, 0x03, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0xf0, 0x00, 0x3f, 0x80, 0x07, 0xe0, 0x00, 0x00, 0xff, 0x80, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int series1[SAMPLE_DEPTH] = {0};
int series1der[SAMPLE_DEPTH] = {0};
int series1max = 0;
int series1min = 2000;
int series1max_h = 0; // _h means all-time max and min
int series1min_h = 2000; // _h means all-time max and min
int series1dermin = 0;
int series1dermax = 0;

int series2[SAMPLE_DEPTH] = {0};
int series2der[SAMPLE_DEPTH] = {0};
int series2max = 0;
int series2min = 2000;
int series2max_h = 0; // _h means all-time max and min
int series2min_h = 2000; // _h means all-time max and min
int series2dermin = 0;
int series2dermax = 0;

const char *starterName = "LENNY-GASSO";
bool sdOK = false;
int startX = 5, startY = 5;
unsigned int distance = 0;
unsigned int gas = 0;
int sc = 0;
bool nearby = false;
int NEARBY_THRESHOLD = 1000;

// SSID and PW for Config Portal
String ssid = "aka_" + String(ESP_getChipId(), HEX);
const char* password = "boysboysboys";

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

// From v1.1.1
// You only need to format the filesystem once
//#define FORMAT_FILESYSTEM       true
#define FORMAT_FILESYSTEM         false

#define MIN_AP_PASSWORD_SIZE    8

#define SSID_MAX_LEN            32
//From v1.0.10, WPA2 passwords can be up to 63 characters long.
#define PASS_MAX_LEN            64

typedef struct
{
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw  [PASS_MAX_LEN];
}  WiFi_Credentials;

typedef struct
{
  String wifi_ssid;
  String wifi_pw;
}  WiFi_Credentials_String;

#define NUM_WIFI_CREDENTIALS      2

typedef struct
{
  WiFi_Credentials  WiFi_Creds [NUM_WIFI_CREDENTIALS];
} WM_Config;

WM_Config         WM_config;

#define  CONFIG_FILENAME              F("/wifi_cred.dat")
//////

// Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
bool initialConfig = false;

// Use false if you don't like to display Available Pages in Information Page of Config Portal
// Comment out or use true to display Available Pages in Information Page of Config Portal
// Must be placed before #include <ESP_WiFiManager.h>
#define USE_AVAILABLE_PAGES     true

// From v1.0.10 to permit disable/enable StaticIP configuration in Config Portal from sketch. Valid only if DHCP is used.
// You'll loose the feature of dynamically changing from DHCP to static IP, or vice versa
// You have to explicitly specify false to disable the feature.
//#define USE_STATIC_IP_CONFIG_IN_CP          false

// Use false to disable NTP config. Advisable when using Cellphone, Tablet to access Config Portal.
// See Issue 23: On Android phone ConfigPortal is unresponsive (https://github.com/khoih-prog/ESP_WiFiManager/issues/23)
#define USE_ESP_WIFIMANAGER_NTP     false

// Use true to enable CloudFlare NTP service. System can hang if you don't have Internet access while accessing CloudFlare
// See Issue #21: CloudFlare link in the default portal (https://github.com/khoih-prog/ESP_WiFiManager/issues/21)
#define USE_CLOUDFLARE_NTP          false

// New in v1.0.11
#define USING_CORS_FEATURE          true
//////
// Use USE_DHCP_IP == true for dynamic DHCP IP, false to use static IP which you have to change accordingly to your network
#if (defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP)
// Force DHCP to be true
#if defined(USE_DHCP_IP)
#undef USE_DHCP_IP
#endif
#define USE_DHCP_IP     true
#else
// You can select DHCP or Static IP here
#define USE_DHCP_IP     true
//#define USE_DHCP_IP     false
#endif

#if ( USE_DHCP_IP )
// Use DHCP
#warning Using DHCP IP
IPAddress stationIP   = IPAddress(0, 0, 0, 0);
IPAddress gatewayIP   = IPAddress(192, 168, 0, 1);
IPAddress netMask     = IPAddress(255, 255, 255, 0);
#else
// Use static IP
#warning Using static IP

#ifdef ESP32
IPAddress stationIP   = IPAddress(192, 168, 1, 132);
#else
IPAddress stationIP   = IPAddress(192, 168, 2, 186);
#endif

IPAddress gatewayIP   = IPAddress(192, 168, 0, 1);
IPAddress netMask     = IPAddress(255, 255, 255, 0);
#endif

#define USE_CONFIGURABLE_DNS      true

IPAddress dns1IP      = gatewayIP;
IPAddress dns2IP      = IPAddress(8, 8, 8, 8);

IPAddress APStaticIP  = IPAddress(192, 168, 101, 1);
IPAddress APStaticGW  = IPAddress(192, 168, 101, 1);
IPAddress APStaticSN  = IPAddress(255, 255, 255, 0);

#include <ESPAsync_WiFiManager.h>              //https://github.com/khoih-prog/ESPAsync_WiFiManager

#define HTTP_PORT     80

const int PIN_LED = 2; // D4 on NodeMCU and WeMos. GPIO2/ADC12 of ESP32. Controls the onboard LED.

///////////////////////////////////////////
// New in v1.4.0
/******************************************
   // Defined in ESPAsync_WiFiManager.h
  typedef struct
  {
  IPAddress _ap_static_ip;
  IPAddress _ap_static_gw;
  IPAddress _ap_static_sn;

  }  WiFi_AP_IPConfig;

  typedef struct
  {
  IPAddress _sta_static_ip;
  IPAddress _sta_static_gw;
  IPAddress _sta_static_sn;
  #if USE_CONFIGURABLE_DNS
  IPAddress _sta_static_dns1;
  IPAddress _sta_static_dns2;
  #endif
  }  WiFi_STA_IPConfig;
******************************************/

WiFi_AP_IPConfig  WM_AP_IPconfig;
WiFi_STA_IPConfig WM_STA_IPconfig;

void initAPIPConfigStruct(WiFi_AP_IPConfig &in_WM_AP_IPconfig)
{
  in_WM_AP_IPconfig._ap_static_ip   = APStaticIP;
  in_WM_AP_IPconfig._ap_static_gw   = APStaticGW;
  in_WM_AP_IPconfig._ap_static_sn   = APStaticSN;
}

void initSTAIPConfigStruct(WiFi_STA_IPConfig &in_WM_STA_IPconfig)
{
  in_WM_STA_IPconfig._sta_static_ip   = stationIP;
  in_WM_STA_IPconfig._sta_static_gw   = gatewayIP;
  in_WM_STA_IPconfig._sta_static_sn   = netMask;
#if USE_CONFIGURABLE_DNS
  in_WM_STA_IPconfig._sta_static_dns1 = dns1IP;
  in_WM_STA_IPconfig._sta_static_dns2 = dns2IP;
#endif
}

void displayIPConfigStruct(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
  LOGERROR3(F("stationIP ="), in_WM_STA_IPconfig._sta_static_ip, ", gatewayIP =", in_WM_STA_IPconfig._sta_static_gw);
  LOGERROR1(F("netMask ="), in_WM_STA_IPconfig._sta_static_sn);
#if USE_CONFIGURABLE_DNS
  LOGERROR3(F("dns1IP ="), in_WM_STA_IPconfig._sta_static_dns1, ", dns2IP =", in_WM_STA_IPconfig._sta_static_dns2);
#endif
}

void configWiFi(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
#if USE_CONFIGURABLE_DNS
  // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
  WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn, in_WM_STA_IPconfig._sta_static_dns1, in_WM_STA_IPconfig._sta_static_dns2);
#else
  // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
  WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn);
#endif
}

///////////////////////////////////////////

uint8_t connectMultiWiFi()
{
#if ESP32
  // For ESP32, this better be 0 to shorten the connect time.
  // For ESP32-S2, must be > 500
#if ( ARDUINO_ESP32S2_DEV || ARDUINO_FEATHERS2 || ARDUINO_PROS2 || ARDUINO_MICROS2 )
#define WIFI_MULTI_1ST_CONNECT_WAITING_MS           500L
#else
  // For ESP32 core v1.0.6, must be >= 500
#define WIFI_MULTI_1ST_CONNECT_WAITING_MS           800L
#endif
#else
  // For ESP8266, this better be 2200 to enable connect the 1st time
#define WIFI_MULTI_1ST_CONNECT_WAITING_MS             2200L
#endif

#define WIFI_MULTI_CONNECT_WAITING_MS                   100L

  uint8_t status;

  LOGERROR(F("ConnectMultiWiFi with :"));

  if ( (Router_SSID != "") && (Router_Pass != "") )
  {
    LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID, F(", Router_Pass = "), Router_Pass );
  }

  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
  {
    // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
    if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
    {
      LOGERROR3(F("* Additional SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
    }
  }

  LOGERROR(F("Connecting MultiWifi..."));

  WiFi.mode(WIFI_STA);

#if !USE_DHCP_IP
  // New in v1.4.0
  configWiFi(WM_STA_IPconfig);
  //////
#endif

  int i = 0;
  status = wifiMulti.run();
  delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);

  while ( ( i++ < 20 ) && ( status != WL_CONNECTED ) )
  {
    status = wifiMulti.run();

    if ( status == WL_CONNECTED )
      break;
    else
      delay(WIFI_MULTI_CONNECT_WAITING_MS);
  }

  if ( status == WL_CONNECTED )
  {
    LOGERROR1(F("WiFi connected after time: "), i);
    LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
    LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP() );
  }
  else
    LOGERROR(F("WiFi not connected"));

  return status;
}

void toggleLED()
{
  //toggle state
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void heartBeatPrint()
{
  static int num = 1;

  if (WiFi.status() == WL_CONNECTED)
    Serial.print(F("H "));        // H means connected to WiFi
  else
    Serial.print(F("F "));        // F means not connected to WiFi

  if (num == 80)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    Serial.print(F(" "));
  }
}

void check_WiFi()
{
  if ( (WiFi.status() != WL_CONNECTED) )
  {
    Serial.println(F("\nWiFi lost. Call connectMultiWiFi in loop"));
    connectMultiWiFi();
  }
}

void check_status()
{
  static ulong checkstatus_timeout  = 0;
  static ulong LEDstatus_timeout    = 0;
  static ulong checkwifi_timeout    = 0;

  static ulong current_millis;

#define WIFICHECK_INTERVAL    1000L
#define LED_INTERVAL          2000L
#define HEARTBEAT_INTERVAL    10000L

  current_millis = millis();

  // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
  if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
  {
    check_WiFi();
    checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
  }

  if ((current_millis > LEDstatus_timeout) || (LEDstatus_timeout == 0))
  {
    // Toggle LED at LED_INTERVAL = 2s
    toggleLED();
    LEDstatus_timeout = current_millis + LED_INTERVAL;
  }

  // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
  if ((current_millis > checkstatus_timeout) || (checkstatus_timeout == 0))
  {
    heartBeatPrint();
    checkstatus_timeout = current_millis + HEARTBEAT_INTERVAL;
  }
}

bool loadConfigData()
{
  File file = FileFS.open(CONFIG_FILENAME, "r");
  LOGERROR(F("LoadWiFiCfgFile "));

  memset(&WM_config,       0, sizeof(WM_config));

  // New in v1.4.0
  memset(&WM_STA_IPconfig, 0, sizeof(WM_STA_IPconfig));
  //////

  if (file)
  {
    file.readBytes((char *) &WM_config,   sizeof(WM_config));

    // New in v1.4.0
    file.readBytes((char *) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
    //////

    file.close();
    LOGERROR(F("OK"));

    // New in v1.4.0
    displayIPConfigStruct(WM_STA_IPconfig);
    //////

    return true;
  }
  else
  {
    LOGERROR(F("failed"));

    return false;
  }
}

void saveConfigData()
{
  File file = FileFS.open(CONFIG_FILENAME, "w");
  LOGERROR(F("SaveWiFiCfgFile "));

  if (file)
  {
    file.write((uint8_t*) &WM_config,   sizeof(WM_config));

    displayIPConfigStruct(WM_STA_IPconfig);

    // New in v1.4.0
    file.write((uint8_t*) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
    //////

    file.close();
    LOGERROR(F("OK"));
  }
  else
  {
    LOGERROR(F("failed"));
  }
}

void setup()
{
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial);

  delay(200); // for Serial

  SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, ELINK_SS);
  display.init(); // enable diagnostic output on Serial
  display.setRotation(1);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(0, 0);
  sdSPI.begin(SDCARD_CLK, SDCARD_MISO, SDCARD_MOSI, SDCARD_SS);
  Serial.println("setup: AKA-JAR, multispec-jar version");
  if (!SD.begin(SDCARD_SS, sdSPI)) {
    sdOK = false;
  } else {
    sdOK = true;
  }
  initSensors();
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(NAME_X, NAME_Y);
  display.setFont(&FreeSansBoldOblique9pt7b);
  display.print(starterName);
  mySwitch.begin();
  mySwitch.pinMode(1, OUTPUT);
  stopFan();

  //  if (sdOK) {
  //    uint32_t cardSize = SD.cardSize() / (1024 * 1024);
  //    display.println("SDCard:" + String(cardSize) + "MB");
  //  } else {
  //    display.println("SDCard  None");
  //  }
//  clobber();
  display.update();
  // goto sleep
  //  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, LOW);
  //  esp_deep_sleep_start();

  Serial.print(F("\nStarting Async_ConfigOnStartup using ")); Serial.print(FS_Name);
  Serial.print(F(" on ")); Serial.println(ARDUINO_BOARD);
  Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);

  if ( String(ESP_ASYNC_WIFIMANAGER_VERSION) < ESP_ASYNC_WIFIMANAGER_VERSION_MIN_TARGET )
  {
    Serial.print("Warning. Must use this example on Version later than : ");
    Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION_MIN_TARGET);
  }

  Serial.setDebugOutput(false);

  if (FORMAT_FILESYSTEM)
    FileFS.format();

  // Format FileFS if not yet
#ifdef ESP32
  if (!FileFS.begin(true))
#else
  if (!FileFS.begin())
#endif
  {
    Serial.print(FS_Name);
    Serial.println(F(" failed! AutoFormatting."));

#ifdef ESP8266
    FileFS.format();
#endif
  }

  unsigned long startedAt = millis();

  // New in v1.4.0
  initAPIPConfigStruct(WM_AP_IPconfig);
  initSTAIPConfigStruct(WM_STA_IPconfig);
  //////

  //Local intialization. Once its business is done, there is no need to keep it around
  // Use this to default DHCP hostname to ESP8266-XXXXXX or ESP32-XXXXXX
  //ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer);
  // Use this to personalize DHCP hostname (RFC952 conformed)
  AsyncWebServer webServer(HTTP_PORT);

#if ( ARDUINO_ESP32S2_DEV || ARDUINO_FEATHERS2 || ARDUINO_PROS2 || ARDUINO_MICROS2 )
  ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, NULL, "AsyncConfigOnStartup");
#else
  DNSServer dnsServer;

  ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "AsyncConfigOnStartup");
#endif

  //set custom ip for portal
  // New in v1.4.0
  ESPAsync_wifiManager.setAPStaticIPConfig(WM_AP_IPconfig);
  //////

  ESPAsync_wifiManager.setMinimumSignalQuality(-1);

  // From v1.0.10 only
  // Set config portal channel, default = 1. Use 0 => random channel from 1-13
  ESPAsync_wifiManager.setConfigPortalChannel(0);
  //////

#if !USE_DHCP_IP
  // Set (static IP, Gateway, Subnetmask, DNS1 and DNS2) or (IP, Gateway, Subnetmask). New in v1.0.5
  // New in v1.4.0
  ESPAsync_wifiManager.setSTAStaticIPConfig(WM_STA_IPconfig);
  //////
#endif

  // New from v1.1.1
#if USING_CORS_FEATURE
  ESPAsync_wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");
#endif

  // We can't use WiFi.SSID() in ESP32 as it's only valid after connected.
  // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are also cleared in reboot
  // Have to create a new function to store in EEPROM/SPIFFS for this purpose
  Router_SSID = ESPAsync_wifiManager.WiFi_SSID();
  Router_Pass = ESPAsync_wifiManager.WiFi_Pass();

  //Remove this line if you do not want to see WiFi password printed
  Serial.println("ESP Self-Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);

  //Check if there is stored WiFi router/password credentials.
  //If not found, device will remain in configuration mode until switched off via webserver.
  Serial.println(F("Opening configuration portal."));

  bool configDataLoaded = false;

  // From v1.1.0, Don't permit NULL password
  if ( (Router_SSID != "") && (Router_Pass != "") )
  {
    LOGERROR3(F("* Add SSID = "), Router_SSID, F(", PW = "), Router_Pass);
    wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());

    ESPAsync_wifiManager.setConfigPortalTimeout(PORTAL_TIMEOUT); //If no access point name has been previously entered disable timeout.
    Serial.println(F("Got ESP Self-Stored Credentials. Timeout 120s for Config Portal"));
  }
  else if (loadConfigData())
  {
    configDataLoaded = true;

    ESPAsync_wifiManager.setConfigPortalTimeout(PORTAL_TIMEOUT); //If no access point name has been previously entered disable timeout.
    Serial.println(F("Got stored Credentials. Timeout 120s for Config Portal"));
  }
  else
  {
    // Enter CP only if no stored SSID on flash and file
    Serial.println(F("Open Config Portal without Timeout: No stored Credentials."));
    initialConfig = true;
  }

  // SSID to uppercase
  ssid.toUpperCase();

  Serial.println(F("Starting configuration portal."));
  digitalWrite(LED_BUILTIN, LED_ON); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.

  // Starts an access point
  if (!ESPAsync_wifiManager.startConfigPortal((const char *) ssid.c_str(), password))
    Serial.println(F("Not connected to WiFi but continuing anyway."));
  else
  {
    Serial.println(F("WiFi connected... :)"));
  }

  // Only clear then save data if CP entered and with new valid Credentials
  // No CP => stored getSSID() = ""
  if ( String(ESPAsync_wifiManager.getSSID(0)) != "" && String(ESPAsync_wifiManager.getSSID(1)) != "" )
  {
    // Stored  for later usage, from v1.1.0, but clear first
    memset(&WM_config, 0, sizeof(WM_config));

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      String tempSSID = ESPAsync_wifiManager.getSSID(i);
      String tempPW   = ESPAsync_wifiManager.getPW(i);

      if (strlen(tempSSID.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1)
        strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
      else
        strncpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);

      if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1)
        strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
      else
        strncpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);

      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
        LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }

    // New in v1.4.0
    ESPAsync_wifiManager.getSTAStaticIPConfig(WM_STA_IPconfig);
    //////

    saveConfigData();

    initialConfig = true;
  }

  digitalWrite(LED_BUILTIN, LED_OFF); // Turn led off as we are not in configuration mode.

  startedAt = millis();

  if (!initialConfig)
  {
    // Load stored data, the addAP ready for MultiWiFi reconnection
    if (!configDataLoaded)
      loadConfigData();

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
        LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }

    if ( WiFi.status() != WL_CONNECTED )
    {
      Serial.println(F("ConnectMultiWiFi in setup"));

      connectMultiWiFi();
    }
  }

  Serial.print(F("After waiting "));
  Serial.print((float) (millis() - startedAt) / 1000L);
  Serial.print(F(" secs more in setup(), connection result is "));

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print(F("connected. Local IP: "));
    Serial.println(WiFi.localIP());
  }
  else
    Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status()));
}



void loop()
{
  // put your main code here, to run repeatedly
  check_status();

  // sample every LOOP_DELAY seconds
  sampler();

  // update the screen every DISPLAY_UPDATE_LOOPs
  sc++;
  if (sc == DISPLAY_UPDATE_LOOPS) {
    if (VERBOSE) {
      Serial.println("UPDATE_SCREEN triggered");
    }
    sc = 0;
    display.fillScreen(GxEPD_WHITE);
    drawGraph();
    drawTrend();
    drawName();
    Serial.println("screen update");
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
  }

  if (VERBOSE) {
    Serial.print("Series1 max/min this 15-loop are: ");
    Serial.print(series1max);
    Serial.print(" ");
    Serial.println(series1min);

    Serial.print("Series2 max/min this 15-loop are: ");
    Serial.print(series2max);
    Serial.print(" ");
    Serial.println(series2min);
  }

  // blocking delay() until next sample; this should all be interrupt-timer driven
  delay(LOOP_DELAY_SECS * 1000);
}