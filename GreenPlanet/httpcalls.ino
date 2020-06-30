int initiateNetwork()
{
  int result;
//  digitalWrite(blue, HIGH);

  logThis(4, "connecting to " + String(ssid), 1);

  long interval = 4000;
  unsigned long currentMillis = millis(), previousMillis = millis();
  int countConnect = 0;

  WiFi.begin(ssid, password);
  vTaskDelay(50);
  while ((WiFi.status() != WL_CONNECTED) && (countConnect < 100))
  {
    vTaskDelay(50);
    Serial.print(".");
    countConnect++;
  }
  if (countConnect == 100)
  {
    Serial.println("Timeout waiting for network");
    digitalWrite(red, HIGH);
    return 1;
  }

  logThis(1, "Connected to Wifi. IP Address: " + String(WiFi.localIP().toString().c_str()) + " MAC address: " + String(WiFi.macAddress()), 2);

  result = httpTestRequest();

  if (result == 0)
  {
    digitalWrite(red, LOW);
    if (RTCpanicStateCode == 1)
    {
      RTCpanicStateCode = 0;
      writeString(1, "PANIC=0");
    }
  }
  else
  {
    digitalWrite(red, HIGH);
    logThis("Network problem error code" + String(result), 2);
  }

//  digitalWrite(blue, LOW);

  return result;
}

int httpTestRequest()
{
  return 0; ////   do nothing. code to be fixed
  char *host = "www.yahoo.com";
  int port = 443;
  String URI = "/";
  //  String successValidator = "<!doctype html>";
  String successValidator = "f7c";

  NetworkResponse myNetworkResponse = httpRequest(host, port, "GET", URI, "", successValidator, 0);

  if (myNetworkResponse.resultCode == 0)
  {
    logThis(1, "Internet connection test completed successfully");
  }
  else
  {
    logThis(0, "Internet connection test failed");
    digitalWrite(red, HIGH);
  }

  return myNetworkResponse.resultCode;
}

void networkReset()
{
  digitalWrite(red, HIGH); // network problem
  WiFi.disconnect();

  logThis("PERFORMING NETWORK RESET!");
  vTaskDelay(1000);
  timerWrite(timer, 0); //reset timer (feed watchdog)

  if (initiateNetwork() == 0)
  {
    logThis("DONE AFTER FIRST ATTEMPT");
    digitalWrite(red, LOW);
    return;
  }
  vTaskDelay(3000);
  timerWrite(timer, 0); //reset timer (feed watchdog)

  if (initiateNetwork() == 0)
  {
    logThis("DONE AFTER SECOND ATTEMPT");
    digitalWrite(red, LOW);
    return;
  }
  boardPanic(1);
}

NetworkResponse httpRequest(char *host, int port, String requestType, String URI, String postData, String successValidator, bool quicky)
{

  NetworkResponse myNetworkResponse;

  if (WiFi.status() != WL_CONNECTED)
    initiateNetwork();

  if (!((requestType == "POST") || (requestType == "GET")))
  {
    logThis("Unsupported call type");
    myNetworkResponse.resultCode = 2;
    return myNetworkResponse;
  }
  logThis(4, "requesting URI: " + URI);

  bool SecureConnection;
  if (port == 443)
    SecureConnection = true;
  else
    SecureConnection = false;

  String httpComm; /* = requestType + " " + URI;

  if ((requestType == "GET") && (postData.length() > 0)) {Serial.println("***************");
    httpComm = httpComm + "?" + postData;
  }

  httpComm = httpComm + " HTTP/1.1\r\n" +
             "Host: " + String(host) + "\r\n" +
             "User-Agent: Mozilla/5.0\r\n" +
             "Cache-Control: no-cache \r\n" +
             "Content-Type: application/x-www-form-urlencoded;\r\n" ;

  // if (requestType == "POST"){httpComm = httpComm + "Content-Length: " + postData.length() + String("\r\n\r\n") + postData + String("\r\n\r\n");}

*/
  if (requestType == "POST")
  {

    httpComm = postData;
  }

  if (requestType == "GET")
  {
    httpComm = "http";
    if (SecureConnection)
      httpComm = httpComm + "s";
    httpComm = httpComm + "://" + host;
    if (!((port == 80) || (port == 443)))
      httpComm = httpComm + ":" + String(port);
    httpComm = httpComm + URI;
    if (postData.length() > 0)
      httpComm = httpComm + "?" + postData;
  }

  logThis(3, "http request:", 0);
  logThis(3, httpComm, 0);

  if (SecureConnection)
    myNetworkResponse = secureHttpRequestExecuter(host, port, URI, httpComm, requestType);
  else
    myNetworkResponse = httpRequestExecuter2(host, port, URI, httpComm, requestType);

  myNetworkResponse.headerLength = myNetworkResponse.header.length();
  myNetworkResponse.bodyLength = myNetworkResponse.body.length();
  ///  deleted header check as new lib doesnt support reading headers
  if (myNetworkResponse.headerLength < -999)
  {
    logThis("Extremely short headers: " + String(myNetworkResponse.headerLength) + "\n " + String(myNetworkResponse.header));
    myNetworkResponse.resultCode = 4;
    digitalWrite(green, LOW);
    digitalWrite(red, HIGH);
    vTaskDelay(500);
    digitalWrite(red, LOW);
    vTaskDelay(500);
    digitalWrite(red, HIGH);
    vTaskDelay(500);
    digitalWrite(red, LOW);
    vTaskDelay(500);

    return myNetworkResponse;
  }

  logThis(4, "Headers: " + myNetworkResponse.header);
  logThis(6, "Body:  " + myNetworkResponse.body);

  if ((myNetworkResponse.body.indexOf(successValidator) == -1) &&
      (myNetworkResponse.header.indexOf(successValidator) == -1))
  {
    logThis("Unanticipated Reponse received.");
    logThis(1, myNetworkResponse.header);
    logThis(1, myNetworkResponse.body);
    myNetworkResponse.resultCode = 5;
    digitalWrite(green, LOW);
    digitalWrite(red, HIGH);
    return myNetworkResponse;
  }

  logThis(3, "Anticipated Reponse received.");
  myNetworkResponse.resultCode = 0;

  return myNetworkResponse;
}

NetworkResponse httpRequestExecuter(char *host, int port, String URI, String httpComm, String requestType)
{

  NetworkResponse myNetworkResponse;
  myNetworkResponse.header = "";
  myNetworkResponse.body = "";

  int resultCode = 0;
//  digitalWrite(blue, HIGH);
  // WiFiClientSecure client;
  WiFiClient client;

  logThis(3, "connecting to " + String(host));

  if (!client.connect(host, port))
  {
    logThis("connection failed");
    client.stop();
    digitalWrite(red, HIGH);
    myNetworkResponse.resultCode = 3;
    return myNetworkResponse;
  }

  client.print(httpComm);

  // String response="";
  unsigned long timeout = millis();
  bool isHeader = true;
  while (client.available() == 0)
  {
    if (millis() - timeout > 15000)
    {
      logThis("connection Timeout");
      client.stop();
      digitalWrite(red, HIGH);
      myNetworkResponse.resultCode = 5;
      return myNetworkResponse;
    }
  }

  timeout = millis();
  while (client.available() > 0)
  {

    if (millis() - timeout > 5000)
    {
      logThis(1, "Timeout waiting for response http", 1);
      break;
    }

    String line = client.readStringUntil('\n');
    if (line == "\r")
      isHeader = false;
    //        if (!(line == "")) {
    //        if (isHeader) {myNetworkResponse.header = myNetworkResponse.header + line; }
    //          else
    {
      myNetworkResponse.body = myNetworkResponse.body + line;
    }

    //      }
  }

  //  myNetworkResponse.headerLength = myNetworkResponse.header.length();
  myNetworkResponse.bodyLength = myNetworkResponse.body.length();

  if (DEBUGLEVEL > 5)
  {
    Serial.println("\nrequest");
    Serial.println(httpComm);
    Serial.println("headers");
    Serial.println(myNetworkResponse.headerLength);
    Serial.println(myNetworkResponse.header);
    Serial.println("body");
    Serial.println(myNetworkResponse.body.length());
    Serial.println(myNetworkResponse.body);
    Serial.println("end");
  }

  myNetworkResponse.resultCode = 0;
  client.stop();
//  digitalWrite(blue, LOW);
  return myNetworkResponse;
}

NetworkResponse secureHttpRequestExecuter(char *host, int port, String URI, String httpComm, String requestType)
{
  NetworkResponse myNetworkResponse;
  myNetworkResponse.header = "";
  myNetworkResponse.body = "";

  int resultCode = 0;
 // digitalWrite(blue, HIGH);
  //WiFiClientSecure client;
  //WiFiClient client;
  WiFiClientSecure *client = new WiFiClientSecure;

  logThis(3, "connecting with https to " + String(host));

  if (!client)
  {
    logThis("connection failed");
    delete client;
    digitalWrite(red, HIGH);
    vTaskDelay(500);
    digitalWrite(red, LOW);
    vTaskDelay(500);
    digitalWrite(red, HIGH);
    vTaskDelay(500);
    digitalWrite(red, LOW);
    myNetworkResponse.resultCode = 3;
    return myNetworkResponse;
  }

  HTTPClient https;   
  

  // client.print(httpComm);

  String connectionString = "http://" + String(host) + ":" + String(port) + URI;
  if (https.begin(*client, connectionString))
  {

//    https.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header
    https.addHeader("User-Agent", "ESP32WiFi/1.1");                       //Specify content-type header
    int httpCode = -1;

    if (requestType == "GET")
    {
      httpCode = https.GET();
    }
    if (requestType == "POST")
    {

      https.addHeader("Connection", "close");                             //Specify content-type header
      //https.addHeader("User-Agent", "application/x-www-form-urlencoded"); //Specify content-type header
     https.addHeader("Content-Type", "application/json"); //Specify content-type header
      https.addHeader("X-THINGSPEAKAPIKEY", "OAYEJT42I0SNZLVW");          //Specify content-type header

      https.addHeader("Content-Length", String(httpComm.length())); //Specify content-type header

      httpCode = https.POST(httpComm);
    }
    /*
      // String response="";
      unsigned long timeout = millis();
      bool isHeader = true;
      if (!client) {
          if (millis() - timeout > 5000) {
                          logThis("connection Timeout");
                          delete client;
                          digitalWrite(red,HIGH);
                          myNetworkResponse.resultCode=5;
                          return myNetworkResponse;
                        }
       }

      timeout = millis();
      //while((client) ) {
      //       if (millis() - timeout > 5000) {logThis(1,"Timeout waiting for response https",1);
      //                                       break;}
      //     String line = client.readStringUntil('\n');
    */
    String line = https.getString();
    logThis(5, line, 2);

    if (line.length() == 0)
      logThis(1, https.errorToString(httpCode).c_str(), 3);

    //   if (line == "\r") isHeader=false;
    //     if (!(line == "")) {
    //         if (isHeader) {myNetworkResponse.header = myNetworkResponse.header + line; }
    //  else
    {
      myNetworkResponse.body = myNetworkResponse.body + line;
    }

    //    }
  }
  myNetworkResponse.headerLength = myNetworkResponse.header.length();
  myNetworkResponse.bodyLength = myNetworkResponse.body.length();

  if (DEBUGLEVEL > 5)
  {
    Serial.println("\nrequest");
    Serial.println(httpComm);
    Serial.println("headers");
    Serial.println(myNetworkResponse.body.length());
    Serial.println(myNetworkResponse.header);
    Serial.println("body");
    Serial.println(myNetworkResponse.body.length());
    Serial.println(myNetworkResponse.body);
    Serial.println("end");
  }

  myNetworkResponse.resultCode = 0;
  https.end();
  delete client;
 // digitalWrite(blue, LOW);    


  return myNetworkResponse;
}

NetworkResponse httpRequestExecuter2(char *host, int port, String URI, String httpComm, String requestType)
{

  NetworkResponse myNetworkResponse;
  myNetworkResponse.header = "";
  myNetworkResponse.body = "";

  int resultCode = 0;
 // digitalWrite(blue, HIGH);
  // WiFiClientSecure client;
  //WiFiClient client;
  WiFiClient *client = new WiFiClient;

  logThis(3, "connecting with http to " + String(host), 3);

  if (!client)
  {
    logThis("connection failed");
    delete client;
    digitalWrite(red, HIGH);
    myNetworkResponse.resultCode = 3;
    return myNetworkResponse;
  }

  HTTPClient http;

  String connectionString = "http://" + String(host) + ":" + String(port) + URI;
  if (http.begin(*client, connectionString))
  {
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header

    int httpCode = -1;

    if (requestType == "GET")
    {
      httpCode = http.GET();
    }
    if (requestType == "POST")
    {
      httpCode = http.POST(httpComm); // have a problem with this
    }

    if (!(httpCode == 200))
      logThis(1, "http error (other than 200): " + String(httpCode), 1);

    // String response="";
    unsigned long timeout = millis();
    bool isHeader = true;
    if (!client)
    {
      if (millis() - timeout > 5000)
      {
        logThis("connection Timeout");
        delete client;
        digitalWrite(red, HIGH);
        myNetworkResponse.resultCode = 5;
        return myNetworkResponse;
      }
    }

    timeout = millis();

    myNetworkResponse.body = http.getString();
  }

  myNetworkResponse.headerLength = 200; // myNetworkResponse.header.length();
  myNetworkResponse.bodyLength = myNetworkResponse.body.length();
  //    http.end();

  if (DEBUGLEVEL > 4)
  {
    Serial.println("\nrequest");
    Serial.println(httpComm);
    Serial.println("headers");
    Serial.println(myNetworkResponse.headerLength);
    Serial.println(myNetworkResponse.header);
    Serial.println("body");
    Serial.println(myNetworkResponse.body.length());
    Serial.println(myNetworkResponse.body);
    Serial.println("end");
  }

  myNetworkResponse.resultCode = 0;

  delete client;
//  digitalWrite(blue, LOW);
  return myNetworkResponse;
}

NetworkResponse httpSecurePost(char *host, int port, String URI, String httpComm, String response)
{

  NetworkResponse myNetworkResponse;
  myNetworkResponse.header = "";
  myNetworkResponse.body = "";
  int resultCode = 0;
//  digitalWrite(blue, HIGH);

  WiFiClientSecure client;
  logThis(3, "connecting with https to " + String(host));

  client.connect(host, port);

  if (!client)
  {
    logThis("connection failed");
    client.stop();
    digitalWrite(red, HIGH);
    vTaskDelay(500);
    digitalWrite(red, LOW);
    vTaskDelay(500);
    digitalWrite(red, HIGH);
    vTaskDelay(500);
    digitalWrite(red, LOW);
    vTaskDelay(500);
    myNetworkResponse.resultCode = 3;
    return myNetworkResponse;
  }

  client.print("POST " + URI + " HTTP/1.1\n");
  client.print("Host: " + String(host) + "\n");
  client.print("Connection: close\n");
  client.print("Content-Type: application/json\n");
 
//  client.print("Content-Type: application/x-www-form-urlencoded\n");

  client.print("Content-Length: ");
  client.print(httpComm.length());
  client.print("\n\n");
  client.print(httpComm);

  String line = client.readStringUntil('\n');
  client.stop();

  if (line.indexOf(response) >= 0)
  {
    myNetworkResponse.resultCode = 0;
    logThis(5, "Anticipatred response recived: " + line, 2);
    logThis(6, "Body sent: " + httpComm, 2);

    return myNetworkResponse;
  }
  else
  {
    logThis(1, "ERROR: Unanticipatred response recived: " + line, 2);
    logThis(1, "ERROR BODY SENT: " + httpComm, 2);

    myNetworkResponse.resultCode = -1;
    return myNetworkResponse;
  }
}
