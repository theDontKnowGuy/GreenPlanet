JSONVar loadConfiguration() {

  JSONVar myConfig;
  myConfig = loadConfigurationFromServer();


  if (JSON.stringify(myConfig) == "-1") myConfig = loadConfigurationFromEEPROM();
  if (JSON.stringify(myConfig) == "-1") {
    logThis("Panic. No configuration found. I'm Dead.");
    boardPanic(3);
  }


  int newFWVersion;
  if (isServer)
    newFWVersion = (int)myConfig["ServerConfiguration"]["targetFWVersion_server"];
  else
    newFWVersion = (int)myConfig["targetFWVersion_client"];

  checkForFirmwareUpdates(newFWVersion);

#if defined(SERVER)
  serverConfiguration = JSON.stringify(myConfig);
#endif

  return myConfig;
}

JSONVar loadConfigurationFromEEPROM() {

  return -1 ;
}

JSONVar loadConfigurationFromServer() {

#if defined(SERVER)
  memcpy(dataUpdateHost, serverDataUpdateHost, 100);
  dataUpdatePort =    serverDataUpdatePort;
  dataUpdateURI =     serverDataUpdateURI;
#else /// if client:

  if (strlen(dataUpdateHost) > 0) {
    dataUpdateURI = String(c_dataUpdateURI); //getting c_dataUpdateURI from EEProm then converting back to string.
  }
  else { // not found in EEPROM
    //not working: endpoint localUpdateServer = findLocalServer(serverMDNSname, "tcp");
    //if ((localUpdateServer.endpointDescription == "error_servernotfound") || (localUpdateServer.endpointDescription == "error_startMDNS"))
    if (true) // because it wasnt working
    {
      memcpy(dataUpdateHost, dataUpdateHost_fallback, 100);
      dataUpdatePort =    dataUpdatePort_fallback;
      dataUpdateURI =     dataUpdateURI_fallback;
    }
    //    else
    //    {
    //      memcpy(dataUpdateHost, localUpdateServer.host, 100);
    //      dataUpdatePort = localUpdateServer.port;
    //      dataUpdateURI = dataUpdateURI_fallback_local;
    //    }
  }

#endif
  NetworkResponse myNetworkResponse = httpRequest(dataUpdateHost, dataUpdatePort, "GET", dataUpdateURI, "", "}", 0);

  if (!(myNetworkResponse.resultCode) == 0) {
    logThis(0, "Failed to read configuration from server " + String(dataUpdateHost) + ", trying server " + String(dataUpdateHost_fallback), 1);

    memcpy(dataUpdateHost, dataUpdateHost_fallback, 100);
    dataUpdatePort =  dataUpdatePort_fallback;
    dataUpdateURI =   dataUpdateURI_fallback;

    myNetworkResponse = httpRequest(dataUpdateHost, dataUpdatePort, "GET", dataUpdateURI, "", "}", 0);

  }
  if (!(myNetworkResponse.resultCode) == 0) {
    logThis(0, "Failed to read configuration from fallback server " + String(dataUpdateHost_fallback), 1);
    return "-1";
  }

  if (myNetworkResponse.resultCode == 0) return JSON.parse(myNetworkResponse.body);
  else  {
    logThis(1, "Network configurartion loading failed.", 1);
    return "-1";
  }
}

int parseConfiguration(JSONVar eyeConfig) {

  if (ConfigurationVersion == (int)eyeConfig["ConfigurationVersion"]) {
    logThis(2, "New configuration available: " + String(ConfigurationVersion));
    return -1;
  } else {
    ConfigurationVersion = (int)eyeConfig["ConfigurationVersion"];
    logThis(3, "Configuration is up to date. Version: " + String(ConfigurationVersion));
  }

  int newDebugLevel = eyeConfig["GeneralConfiguration"]["DEBUGLEVEL"];
  if (DEBUGLEVEL < newDebugLevel) {
    logThis("Setting DEBUGLEVEL to " + String(newDebugLevel));
    DEBUGLEVEL = newDebugLevel;
  }

  red = (int)eyeConfig["GeneralConfiguration"]["red"];
  blue = (int)eyeConfig["GeneralConfiguration"]["blue"];
  green = (int)eyeConfig["GeneralConfiguration"]["green"];

  pinMode(red, OUTPUT); pinMode(green, OUTPUT); pinMode(blue, OUTPUT);

  delayBetweenExecs =   eyeConfig["GeneralConfiguration"]["delayBetweenExecs"];
  sleepAfterExec =      eyeConfig["GeneralConfiguration"]["sleepAfterExec"];
  daylightOffset_sec =  eyeConfig["GeneralConfiguration"]["daylightOffset_sec"];
  gmtOffset_sec =       eyeConfig["GeneralConfiguration"]["gmtOffset_sec"];
  sleepAfterPanic =     eyeConfig["GeneralConfiguration"]["sleepAfterPanic"];
  sleepRandFactor =     eyeConfig["GeneralConfiguration"]["sleepRandFactor"];

  String s = JSON.stringify(eyeConfig["GeneralConfiguration"]["dataUpdateHost"]);
  s = cleanQuote(s);
  s.toCharArray(dataUpdateHost, s.length() + 1);
  logThis(3, "Configuration server is now: " + String(dataUpdateHost));

  dataUpdateURI =       eyeConfig["GeneralConfiguration"]["dataUpdateURI"];
  dataUpdateURI.toCharArray(c_dataUpdateURI, dataUpdateURI.length() + 1); // to survive deep sleep . strings dont
  loggerHostPort =      eyeConfig["GeneralConfiguration"]["loggerHostPort"];

  s = JSON.stringify(eyeConfig["GeneralConfiguration"]["loggerHost"]);
  s = cleanQuote(s);
  s.toCharArray(loggerHost, s.length() + 1);
  logThis(3, "Logging server is now: " + String(loggerHost));

  logTarget =       eyeConfig["GeneralConfiguration"]["logTarget"];
  logTarget.toCharArray(c_logTarget, logTarget.length() + 1); // to survive deep sleep . strings dont
  dataUpdatePort =      eyeConfig["GeneralConfiguration"]["dataUpdatePort"];
  write_api_key = eyeConfig["GeneralConfiguration"]["write_api_key"];


  if (isServer) {

    s = JSON.stringify(eyeConfig["ServerConfiguration"]["serverLoggerHost"]);
    s = cleanQuote(s);
    s.toCharArray(loggerHost, s.length() + 1);
    logThis(3, "Logging server for clients is now: " + String(loggerHost));
    logTarget =            eyeConfig["ServerConfiguration"]["serverLogTarget"];
    loggerHostPort =       int(eyeConfig["ServerConfiguration"]["serverLoggerHostPort"]);
    loggingType =           int(eyeConfig["ServerConfiguration"]["loggingType"]);
  } else {
    logTarget =            eyeConfig["GeneralConfiguration"]["logTarget"];
    loggerHostPort =       int(eyeConfig["GeneralConfiguration"]["loggerHostPort"]);
    loggingType =           int(eyeConfig["GeneralConfiguration"]["loggingType"]);
  }
  int i = 0; bool found = false;
  while ( JSON.typeof(eyeConfig["Devices"][i]) == "object") {

    if (WiFi.macAddress() == eyeConfig["Devices"][i]["DeviceMAC"]) {
      deviceID = eyeConfig["Devices"][i]["deviceID"];
      deviceLocation = eyeConfig["Devices"][i]["deviceLocation"]   ;
      deviceGroup = eyeConfig["Devices"][i]["deviceGroup"]   ;
      memberInOperationPlans = eyeConfig["Devices"][i]["memberInOperationPlans"]   ;
      logThis(3, "Device found. Device name: " + deviceID , 2);
      found = true;
      if (JSON.typeof(eyeConfig["Devices"][i]["irled"]) == "number")
      {
        kIrLed = int(eyeConfig["Devices"][i]["irled"]);
        pinMode(kIrLed, OUTPUT);
        digitalWrite(kIrLed, LOW);
      }
      if (JSON.typeof(eyeConfig["Devices"][i]["blue"]) == "number")
      {
        blue = int(eyeConfig["Devices"][i]["blue"]);
        pinMode(blue, OUTPUT);
        digitalWrite(blue, LOW);
      }
    }
    i++;
  }

  if (!(found)) {
    logThis("Panic. MAC not found in configurarion file. Please add new device to configuation file.");
    gotoSleep(sleepAfterExec);
  }

  i = 0;
  int j = 0;

  //refactor to support more than 10 plans

  while (JSON.typeof(eyeConfig["operationPlans"][j]) == "object") {

    if (memberInOperationPlans.indexOf(JSON.stringify(eyeConfig["operationPlans"][j]["operationPlanID"])) >= 0 ) {

      myOperationPlans[i].operationPlanID =     eyeConfig["operationPlans"][j]["operationPlanID"];
      myOperationPlans[i].operationPlanName =   JSON.stringify(eyeConfig["operationPlans"][j]["operationPlanName"]);
      myOperationPlans[i].IRcodeID =            eyeConfig["operationPlans"][j]["IRcodeID"];
      myOperationPlans[i].hour =                eyeConfig["operationPlans"][j]["hour"];
      myOperationPlans[i].minute =              eyeConfig["operationPlans"][j]["minute"];
      myOperationPlans[i].weekdays =            eyeConfig["operationPlans"][j]["weekdays"];

      i++;
    }
    j++;
  }
  inxParticipatingPlans = i ;
  logThis(3, "Participating in " + String(inxParticipatingPlans) + " plans.", 2);

  i = 0;

  while (JSON.typeof(eyeConfig["IRcode"][i])   == "object")
  {

    myIRcode[i].IRcodeID = eyeConfig["IRcode"][i]["IRcodeID"];
    myIRcode[i].IRcodeDescription =   JSON.stringify(eyeConfig["IRcode"][i]["IRcodeDescription"]);

    int l = 0;
    while ((int)eyeConfig["IRcode"][i]["IRCodeBitStream"][l] > 0) {
      myIRcode[i].IRCodeBitStream[l] =  (int)eyeConfig["IRcode"][i]["IRCodeBitStream"][l];
      l++;
    }
    myIRcode[i].IRCodeBitStreamLength = l;
    //logThis(2, "IR plan code " + String(i) + " loaded with length of " + String(l) + " bits",2);
    i++;
  }

  inxParticipatingIRCodes = i;

  j = 0;
  while (JSON.typeof(eyeConfig["sleepPlans"][j]) == "object") {
    if (timeinfo.tm_hour > (int)eyeConfig["sleepPlans"][j]["planStartHour"])
      sleepTime = (int)eyeConfig["sleepPlans"][j]["sleepTime"];
    j++;
  }


#if defined(SERVER)
  ServerConfigurationRefreshRate = (int)eyeConfig["ServerConfiguration"]["ServerConfigurationRefreshRate"];
  maxLogAge =  (int)eyeConfig["ServerConfiguration"]["maxLogAge"];
  recessTime = (int)eyeConfig["ServerConfiguration"]["recessTime"];
#endif

  logThis(3, "Network configuration loaded and parsed succesfully", 3);
}
