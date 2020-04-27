int checkForFirmwareUpdates(JSONVar eyeConfig) {

  logThis(3, "Checking for firmware updates.", 2);
  logThis(3, "Current firmware version: " + String(FW_VERSION) , 2);
  int newFWVersion;
  if (isServer)
    newFWVersion = (int)eyeConfig["ServerConfiguration"]["targetFWVersion_server"];
  else
    newFWVersion = (int)eyeConfig["targetFWVersion_client"];

  if ( newFWVersion <= FW_VERSION ) {
    logThis(4, "Already on the recent firmware.", 2);
    return 0;
  }

  logThis(1, "Available firmware version: " + String(newFWVersion), 2 );

  String fwImageURL = String( fwUrlBase ) + String (newFWVersion);
  if (isServer) fwImageURL = fwImageURL + "_s.bin";
  else fwImageURL = fwImageURL + "_c.bin";
  logThis(2, "Firmware version URL: " + fwImageURL , 2);

  WiFiClientSecure client;
  HTTPClient httpClient;
  httpClient.begin( client, fwImageURL );
  //  int httpCode = httpClient.GET();
  timerWrite(timer, 0); //reset timer (feed watchdog)
  timerAlarmWrite(timer, wdtTimeout * 1000 * 5, false); //set time in us
  timerAlarmEnable(timer);                          //enable interrupt


  t_httpUpdate_return ret = httpUpdate.update( client, fwImageURL ); /// FOR ESP32 HTTP FOTA

  switch (ret) {

    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str()); /// FOR ESP32          logThis(0, strErr, 2); /// FOR ESP32
      char currentString[64];
      sprintf(currentString, "\nHTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str()); /// FOR ESP32          break;

    case HTTP_UPDATE_NO_UPDATES:
      logThis(0, "HTTP_UPDATE_NO_UPDATES.", 2);
      break;
  }
  httpClient.end();
  return -1;
}
