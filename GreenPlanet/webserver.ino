#if (!defined(SERVER))

//endpoint findLocalServer(const char *service, const char *protocol)
//{
//
//  endpoint localServer;
//  localServer.endpointDescription = "local esp32 server";
//
//  if (!MDNS.begin("ESP32_Browser"))
//  {
//    Serial.println("Error setting up MDNS responder!");
//    localServer.endpointDescription = "error_startMDNS";
//    return localServer;
//  }
//
//  Serial.printf("Browsing for service _%s._%s.local. ... ", service, protocol);
//  int n = MDNS.queryService(service, protocol);
//  if (n == 0)
//  {
//    logThis(1, "no local servers found", 3);
//    localServer.endpointDescription = "error_servernotfound";
//    return localServer;
//  }
//  else
//  {
//    localServer.endpointDescription = MDNS.hostname(0);
//    String s = String() + MDNS.IP(0)[0] + "." + MDNS.IP(0)[1] + "." + MDNS.IP(0)[2] + "." + MDNS.IP(0)[3];
//    Serial.println(String(service) + " service found on " + s);
//    s.toCharArray(localServer.host, 16);
//    localServer.port = MDNS.port(0);
//    logThis(2, "Local server found: " + String(localServer.endpointDescription) + " at " + String(localServer.host) + ": " + String(localServer.port), 2);
//  }
//
//  return localServer;
//}

#endif

#if defined(SERVER)

void startWebServer()
{

  //  if (!MDNS.begin(serverMDNSname))
  //  {
  //    logThis(1, "Error setting up MDNS responder!");
  //    networkReset();
  //  }
  //
  //  MDNS.addService(String(serverMDNSname), "tcp", 80);
  //
  //  logThis(1, "mDNS responder started on http://" + String(serverMDNSname) + ".local", 3);
  server.begin();
  server.on("/GreenPlanet/GreenPlanetConfig.json", handleGetConfig);

  server.on("/logs/{}", []() {
    String t;
    for (uint8_t i = 0; i < server.args(); i++)
    {
      t += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }

    Serial.println(t);

    logThis(5, logBuffer, 1);
    String body = "Congratulations";
    server.send(200, "text/html", body);
    if (logBuffer.length() > 0)
      if (networklogThis(logBuffer) == 0)
      {
        logBuffer = "";
      }
    bool asProxy = true;
    if (networklogThis(t, asProxy) == 0)
    { //// this need a fix
    }
    else
    {
      logThis(1, "Error logging clients!", 1);
    }
    Serial.println("end proxy proxy");
  });

  server.on("/executePlan/{}", []() {
    String arg1 = server.pathArg(0);
    int IRcodeID;
    IRcodeID = arg1.toInt();
    String message = "";
    int r = execPlan(IRcodeID);
    message = "Consider it done for plan " + String(IRcodeID);
    server.send(200, "text/plain", message);
  });

  server.on("/reboot", []() {
    logThis(1, "reboot per webserver order");
    networklogThis(networkLogBuffer);
    ESP.restart();
  });

  server.on("/testIR", []() {
    handleTestIR();
  });

  server.on("/testColors", []() {
    handleTestColors();
  });

  server.on("/learn", []() {
    learnCode();
  });
  server.on("/testWeed", []() {
    testWeed();
  });
  server.onNotFound(handleNotFound);
}

void handleGetConfig()
{
  logThis(3, "Request for configuration served", 1);
  int l = serverConfiguration.length() + 1;
  char body[l];
  serverConfiguration.toCharArray(body, l);
  server.send(200, "text/html", body);
}

void handleRoot()
{

  char body[100] = "hello world";
  server.send(200, "text/html", body);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}


void handleTestIR()
{


  char body[100] = "Running Test";
  server.send(200, "text/html", body);

  Serial.println("long ir seq");

  for (int k = 0; k < 5; k++) {
    //    for (int i = 0; i < 8; i++) {
    //      digitalWrite(blue, HIGH);
    //  //    digitalWrite(kIrLed, HIGH);
    //      vTaskDelay(2000);
    //      timerWrite(timer, 0); //reset timer (feed watchdog)
    //      digitalWrite(blue, LOW);
    //      digitalWrite(kIrLed, LOW);
    //      vTaskDelay(500);
    //    }

    digitalWrite(red, HIGH);
    vTaskDelay(500);
    digitalWrite(red, LOW);
    vTaskDelay(500);
    timerWrite(timer, 0); //reset timer (feed watchdog)

    Serial.println(kIrLed);

    for (int i = 0; i < 2; i++) {
      execPlan(4);//enable interrupt
      timerWrite(timer, 0); //reset timer (feed watchdog)

    }

    digitalWrite(red, HIGH);
    vTaskDelay(500);
    digitalWrite(red, LOW);

    digitalWrite(red, HIGH);
    vTaskDelay(1000);
    digitalWrite(red, LOW);

    timerWrite(timer, 0); //reset timer (feed watchdog)

  }
}


void handleTestColors()
{


  char body[100] = "Running Test";
  server.send(200, "text/html", body);
    digitalWrite(green, LOW);
    digitalWrite(red, LOW);
    digitalWrite(blue, LOW);
    digitalWrite(kIrLed, LOW);

  Serial.println("color seq");
  for (int k = 0; k < 15; k++) {
    timerWrite(timer, 0); //reset timer (feed watchdog)
    digitalWrite(blue, HIGH);
    vTaskDelay(500);
    digitalWrite(blue, LOW);
    vTaskDelay(500);
    digitalWrite(red, HIGH);
    vTaskDelay(500);
    digitalWrite(red, LOW);
    vTaskDelay(500);
    digitalWrite(green, HIGH);
    vTaskDelay(500);    
    digitalWrite(green, LOW);
    vTaskDelay(500);
    digitalWrite( kIrLed, HIGH);
    vTaskDelay(500);
    digitalWrite(kIrLed , LOW);
    vTaskDelay(500);
  }
}
void testWeed() {
  digitalWrite(blue , LOW);
  Serial.println("weed seq");
  digitalWrite(green, HIGH);
  vTaskDelay(500);
  digitalWrite(green , LOW);
  vTaskDelay(500);
  digitalWrite(green, HIGH);
  vTaskDelay(500);
  digitalWrite(green , LOW);
  vTaskDelay(500);
  digitalWrite(green, HIGH);
  vTaskDelay(500);
  digitalWrite(green , LOW);
  vTaskDelay(500);
  digitalWrite(green, HIGH);
  vTaskDelay(500);
  digitalWrite(green , LOW);
  vTaskDelay(500);
  digitalWrite(green, HIGH);
  vTaskDelay(500);
  digitalWrite(green , LOW);
  vTaskDelay(500);

}

#endif
