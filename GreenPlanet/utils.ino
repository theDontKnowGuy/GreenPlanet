void blinkLiveLed() {
  updateTime (0); ////??????????
  //  timerWrite(timer, 0); //reset timer (feed watchdog)

  if (millis() - LiveSignalPreviousMillis > 50) {
    digitalWrite(blue, !(LivePulseLedStatus));
    LivePulseLedStatus  = !(LivePulseLedStatus);
    totalLifes += 1;
    LiveSignalPreviousMillis = millis();
    DHTsensor.read();
    DHTt = DHTsensor.getTemperature();
    DHTh = DHTsensor.getHumidity();
    
  }
  //  if(logBuffer.length() > 0) {if(networklogThis(logBuffer) == 0) {  logBuffer = ""; }}
  logAge++;
  if ((logAge > maxLogAge * 1000) && (networkLogBuffer.length() > 1))
    if (networklogThis(networkLogBuffer) == 0) {
      networkLogBuffer = "";
      logAge = 0;
    }
    else
    {
      logAge = 0;  // last sent was unsuccesful. lets wait ~10 sec and retry
    }

  if ((totalLifes > 60 * 60 * 24 * 2) && (timeinfo.tm_hour == maintenanceRebootHour)) {
    logThis("Rebooting for maintenance...");
    networklogThis(networkLogBuffer);
    ESP.restart();
  }


#if defined(SERVER)

  if ((totalLifes % (ServerConfigurationRefreshRate * 2) == 0) || (serverConfiguration == "")) {
    JSONVar myConfig = loadConfiguration();
    checkForFirmwareUpdates(myConfig);
    parseConfiguration(myConfig);
    serverConfiguration = JSON.stringify(myConfig);
    logThis(1, "Server Configuration refreshed", 2);
    totalLifes += 1;
  }

#endif


}

void blinkLiveLedFast() {
  timerWrite(timer, 0); //reset timer (feed watchdog)
  vTaskDelay(10 / portTICK_RATE_MS);

  if (millis() - LiveSignalPreviousMillis > 100) {
    digitalWrite(green, !(LivePulseLedStatus));
    LivePulseLedStatus  = !(LivePulseLedStatus);
    LiveSignalPreviousMillis = millis();
  }
}

int checkPanicMode(void) {

  String panicState = readEEPROM(1);

  if (panicState.substring(0, 5) != "PANIC")
  { writeString(1, "PANIC=0");
    return 0;
  }
  if ((RTCpanicStateCode == 0) &&  (panicState == "PANIC=0")) return 0;// no deep sleep after panic nor after SW restart
  else RTCpanicStateCode = panicState.substring(7, 1).toInt();

  return RTCpanicStateCode;
}

String getDigits(int digits) {
  return (digits < 10) ? "0" + String(digits) : String(digits);
}

void boardpanic(int panicReason) {
  String panicState = readEEPROM(1);

  if (panicState == "PANIC=0") {  //first panic
    Serial.println("Reseting for panic !!!!!!!!!!!!!!!!!!!!!!!!!!!  Panic Reason is:" + String(panicReason));
    writeString(1, "PANIC=" + String(panicReason));
    ESP.restart();
  }
  else
  {
    Serial.println("Second panic call!!!!!!!!!!!!!!!!!!!!!!!!!!! Panic Reason is:" + String(panicReason));
    writeString(1, "PANIC=0"); //if fail next time - let's try restart instead
    gotoSleep((isServer) ? sleepAfterPanic : 60, 1);
  }
}

void printLocalTime() {

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void updateTime (uint64_t elapsedTime) { // elapsedTime in us
  if (elapsedTime == 0) Mics += micros();
  else Mics += elapsedTime;
  if (Mics > 1000000) {
    Mics = Mics % 1000000;
    rightNow += Mics / 1000000;
  }
}

String cleanQuote(String s) {
  String s2 = "";
  for (int i = 0; i < s.length(); i++) {
    if (!(s[i] == char(34))) {
      s2 = s2 + s[i];
    }
  }
  return s2;
}


String mac2long(String s) {
  //246F289D9A64

  s.replace(":", "");
  s = s.substring(6);
  s.replace("A", "10");
  s.replace("B", "11");
  s.replace("C", "12");
  s.replace("D", "13");
  s.replace("E", "14");
  s.replace("F", "15");

  return s;
}
