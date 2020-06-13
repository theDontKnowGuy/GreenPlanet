/*
   GreenPlanet by theDontKnowGuy

   Control programmable IR remote controls
   Version 2.0 - Webserver goes to a seperate core to give a quicker service to clients
   Version 1.0 - Basic functionality works
*/

#include <Arduino.h>
#include "secrets.h"


#define RELEASE true
//#define SERVER
const int FW_VERSION = 2020061301;   
int DEBUGLEVEL = 2;     // set between 0 and 5. This value will be overridden by dynamic network configuration json if it has a higher value


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// Firmware update over the air (FOTA) SECTION///////////////////////////////////////////????//////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <HTTPUpdate.h>                                                                /// year_month_day_counternumber 2019 is the year, 04 is the month, 17 is the day 01 is the in day release
const char *fwUrlBase = "https://raw.githubusercontent.com/theDontKnowGuy/GreenPlanet/master/fota/"; /// put your server URL where the *.bin & version files are saved in your http ( Apache? ) server


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// NETWORK SECTION/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// defined in secrets.h:    const char* ssid =    "my Wifi SSID";
// defined in secrets.h:    const char* password = "password";

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

typedef struct
{
  int resultCode;
  String header;
  String body;
  int headerLength;
  int bodyLength;
} NetworkResponse;

const int httpsPort = 443;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// WEBSERVER SECTION///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if defined(SERVER)

const bool isServer = true;
#include <WebServer.h>
WebServer server(80);
#else
const bool isServer = false;
#endif

typedef struct
{
  String endpointDescription;
  String descriptor;
  char host[100];
  int port;
  String URI;
  bool isSSL = false;
  String successString;
} endpoint;

#include <ESPmDNS.h>
char *serverMDNSname = "GreenPlanet"; //clients will look for http://GreenPlanet.local

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// LOGGING SECTION/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool log2Serial = true; //move to false in production to save some time


//char loggerHost[100] = "192.168.1.200";
//String logTarget =     "/MyRFDevicesHub/MyRFDevicesHubLogger.php"; /// leave empty if no local logging server (will only Serial.print logs)
////String logTarget =     "/logs"; /// leave empty if no local logging server (will only Serial.print logs)
//int loggerHostPort = 80;

RTC_DATA_ATTR int loggingType = 3;
RTC_DATA_ATTR int loggingCounter = 0;
RTC_DATA_ATTR char c_logTarget[200];
//RTC_DATA_ATTR char loggerHost[100] = "api.thingspeak.com";
RTC_DATA_ATTR char loggerHost[100] = "maker.ifttt.com";
//RTC_DATA_ATTR char loggerHost[100] = "192.168.1.200";
//defined in secrets.h :   String logTarget =     "channels/<channel code here>/bulk_update.csv"; 
//defined in secrets.h :   String logTarget = "/trigger/GreenPlanet2/with/key/xxxxxxx-xxx";
RTC_DATA_ATTR int loggerHostPort = 443;
String write_api_key = "";

//RTC_DATA_ATTR int loggingType = 1;
//RTC_DATA_ATTR char c_logTarget[200];

String logBuffer = "";
String networkLogBuffer = "";
unsigned long previousTimeStamp = millis(), totalLifes, LiveSignalPreviousMillis = millis(), lastMessageTiming = 0;
int maxLogAge = 200; //  how many sec to keep log before attempting to send.
int logAge = 0, LivePulseLedStatus = 0;
int failedLogging2NetworkCounter = 0;
int addFakeSec = -1;
String firstLogTimeStamp = "";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// REMOTE CONFIGURATION SECTION ///////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ConfigurationVersion = 0;
RTC_DATA_ATTR char dataUpdateHost[100];
RTC_DATA_ATTR int dataUpdatePort;
String dataUpdateURI;
RTC_DATA_ATTR char c_dataUpdateURI[200];

char *serverDataUpdateHost = "raw.githubusercontent.com";
int serverDataUpdatePort = 443;

#if (RELEASE)
String serverDataUpdateURI = "/theDontKnowGuy/GreenPlanet/master/configuration/GreenPlanetConfig.json";
String dataUpdateURI_fallback = "/theDontKnowGuy/GreenPlanet/master/configuration/GreenPlanetConfig.json"; /// see example json file in github. leave
#else
String serverDataUpdateURI = "/theDontKnowGuy/GreenPlanet/master/configuration/GreenPlanetConfig_dev.json";
String dataUpdateURI_fallback = "/theDontKnowGuy/GreenPlanet/master/configuration/GreenPlanetConfig_dev.json"; /// see example json file in github. leave
#endif

char *dataUpdateHost_fallback = "raw.githubusercontent.com";
int dataUpdatePort_fallback = 443;
String dataUpdateURI_fallback_local = "/GreenPlanet/GreenPlanetConfig.json";                               /// see example json file in github. leave value empty if no local server

int ServerConfigurationRefreshRate = 60;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// MAINTENANCE AND HARDWARE SECTION ///////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int red = 2;
int green = 17;
int blue = 12;

int maintenanceRebootHour = 4; // the hub will reboot once a day at aproximitly this UTC hour (default 4 am), provided it was running ~24 hours

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// JSON SECTION ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Arduino_JSON.h>
JSONVar eyeConfig;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// TIME AND CLOCK SECTION /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <time.h>

const char *ntpServer = "pool.ntp.org";
long gmtOffset_sec = 7200;
int daylightOffset_sec = 0;
struct tm timeinfo;
bool ihaveTime = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// MQTT SECTION ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//char* MQTTtopicDevices = "MyRFDeviceHub";
//char* MQTTtopicControl = "MyRFDeviceHub/control";
//char* MQTTendpointID = "Hub1";
//char* MQTTHost = "192.168.1.200";
//int MQTTPort = 1883;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// IR SECTION//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include <IRremote.h>

#include <IRrecv.h>
#include <IRsend.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

long learningTH = 15000;

uint16_t kIrLed = 14;
const uint16_t kRecvPin = 15;

const uint16_t kCaptureBufferSize = 2048;
const uint16_t kMinUnknownSize = 12;
const uint8_t kTimeout = 50;

int maxSignalLength = 200;
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results; // Somewhere to store the results

uint16_t ACcode[200];
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// DHT SECTION  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <dhtnew.h>
#define DHTleg 4
DHTNEW DHTsensor(DHTleg);
float DHTt = 0;
float DHTh = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// EEPROM SECTION  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <EEPROM.h>
int maxEEPROMMessageLength = 1000;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// WATCHGDOG SECTIO1N //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int wdtTimeout = 22000; //time in ms to trigger the watchdog // NOTE: less than that, on upgrade it barks. alternatively, kill dog before update
hw_timer_t *timer = NULL;
RTC_DATA_ATTR byte bootCount = 0;
RTC_DATA_ATTR time_t rightNow;
RTC_DATA_ATTR uint64_t Mics = 0;
//unsigned long chrono;

void IRAM_ATTR resetModule()
{
  ets_printf("reboot for freeze\n");
  esp_restart();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// DEEPSLEEP SECTION //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
int WakeUpSensorThreshold = 50;    /* Greater the value, more the sensitivity */
touch_pad_t touchPin;
int vTaskDelayBetweenExecs = 3;
int sleepAfterExec = 1800;
int sleepTime = 1800;
int sleepAfterPanic = 200;
int sleepRandFactor = 120;

//#include "esp_system.h"

RTC_DATA_ATTR int RTCpanicStateCode = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// THIS PROGRAM SPECIFIC SECTION //////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

String deviceID;                    // = "Unknown";  // will be overwritten by dynamically loaded configuration file. if you see this on the hub - update configuration to new MAC
String deviceGroup = "New Devices"; // will be overwritten by dynamically loaded configuration file. if you see this on the hub - associate new device to a group
String deviceLocation = "default";  // same idea
String memberInOperationPlans = ""; // same idea
String MACID;                       // MAC address converted to long

typedef struct
{
  int operationPlanID;
  String operationPlanName;
  int IRcodeID;
  int hour;
  int minute;
  String weekdays;
  long recentExecution = 0;
} operationPlans;
operationPlans myOperationPlans[10];

typedef struct
{
  int IRcodeID;
  String IRcodeDescription;
  uint16_t IRCodeBitStream[300];
  int IRCodeBitStreamLength;
} IRcode;
IRcode myIRcode[20];

int inxParticipatingPlans = 0;
int inxParticipatingIRCodes = 0;
int recessTime = 160; // between executions, not to repeat same exection until clock procceeds

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// SERVER CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(SERVER)

String serverConfiguration = "";

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// END OF DECLERATION /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  pinMode(red, OUTPUT);  pinMode(green, OUTPUT);  pinMode(blue, OUTPUT); pinMode(kIrLed, OUTPUT); digitalWrite(kIrLed, LOW);
  digitalWrite(blue, HIGH); vTaskDelay(100); digitalWrite(blue, LOW); digitalWrite(red, LOW); digitalWrite(green, LOW);

  // Hardeware Watchdog
  timer = timerBegin(0, 80, true);                  //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);  //attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, false); //set time in us
  timerAlarmEnable(timer);                          //enable interrupt

  MACID = mac2long(WiFi.macAddress());

  if (log2Serial)
    Serial.begin(115200);

#if (RELEASE)
#else
  logThis(1, "********** NOT A RELEASE VERSION ******************* NOT A RELEASE VERSION ******************* NOT A RELEASE VERSION ********* ", 2);
#endif
  logThis(3, "Starting GreenPlanet Device by the DontKnowGuy", 2);
  logThis(3, "Firmware version " + String(FW_VERSION) + ". Unique device identifier: " + MACID, 2);

  EEPROM.begin(4096);
  checkPanicMode();

  if (initiateNetwork() > 0)
  {
    networkReset();
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  ihaveTime = true;
  updateTime(0);        ////??????????
  timerWrite(timer, 0); //reset timer (feed watchdog)

  LiveSignalPreviousMillis = millis();
  bootCount++;
  logThis(0, "This is boot No. " + String(bootCount), 3);
  if (bootCount == 1)
  {
    digitalWrite(red, HIGH); vTaskDelay(100); digitalWrite(red, LOW); vTaskDelay(50); digitalWrite(red, HIGH); vTaskDelay(100); digitalWrite(red, LOW); vTaskDelay(50); digitalWrite(red, HIGH); vTaskDelay(100); digitalWrite(red, LOW); vTaskDelay(50);
  }
  if (bootCount > 48)
    ESP.restart();

  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
  if (isServer) irrecv.enableIRIn(); // Start the receiver

  parseConfiguration(loadConfiguration());

  logThis(3, "This is device " + String(deviceID), 3);

#if defined(SERVER)
  logThis(1, "I am a server", 2);
  startWebServer();
#endif

  wakeupReason();
  DHTsensor.read();
  DHTt = DHTsensor.getTemperature();
  DHTh = DHTsensor.getHumidity();

  logThis(1, "Temperature: " + String(DHTt) + " Humidity: " + String(DHTh));
  logThis(3,"Initialization Completed.", 3);
  digitalWrite(blue, LOW); // system live indicator

#if defined(SERVER)

  xTaskCreatePinnedToCore(
    serverOtherFunctions, "serverOtherFunctions" // A name just for humans
    ,
    7000 // This stack size can be checked & adjusted by reading the Stack Highwater
    ,
    NULL, 0 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,
    NULL, 1);

  xTaskCreatePinnedToCore(
    webServerFunction, "webServerFunction" // A name just for humans
    ,
    7000 // This stack size can be checked & adjusted by reading the Stack Highwater
    ,
    NULL, 3 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,
    NULL, 0);

#else
  planDispatcher();
  gotoSleep(calcTime2Sleep()); ///is this order right ????????
#endif
} //setup


#if defined(SERVER)
void webServerFunction(void *pvParameters) {

  (void) pvParameters;
  for (;;) {
    server.handleClient();
    vTaskDelay(10 / portTICK_RATE_MS);
    timerWrite(timer, 0); //reset timer (feed watchdog)
  }
}

void serverOtherFunctions(void *pvParameters) {

  (void) pvParameters;
  for (;;) {
    planDispatcher();
    blinkLiveLed();
    timerWrite(timer, 0); //reset timer (feed watchdog)
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}
#endif

void loop()
{
  vTaskDelay(10 / portTICK_RATE_MS);
  timerWrite(timer, 0);
}
