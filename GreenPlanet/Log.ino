
void logThis(String strMessage)
{
  logThis(0, strMessage, 2);
  return;
}

void logThis(String strMessage, int newLineHint)
{
  logThis(0, strMessage, newLineHint);
  return;
}

void logThis(int debuglevel, String strMessage)
{
  logThis(debuglevel, strMessage, 2);
  return;
}

void logThis(int debuglevel, String strMessage, int newLineHint)
{
  // newLineHint: 0- nothing 1- before 2- after 3- before and after

  if (DEBUGLEVEL < debuglevel)
    return;

  String pre = "";
  String post = "";

  if (newLineHint == 1)
    pre = "\n";
  if (newLineHint > 1)
    post = "\n";

  if (log2Serial)
    Serial.print(pre + strMessage + post);

  strMessage.replace(",", " * ");
  strMessage.replace("|", " ** ");
  strMessage.replace(char(34), char(33));
  strMessage.replace("\n", " ");
  strMessage.replace("&", "*");

  String tDHTt = (DHTt == 0.0) ? "" : String(DHTt);

  loggingCounter++;

  if (  ihaveTime  )  getLocalTime(&timeinfo);

  if (addFakeSec == -1)
    addFakeSec = timeinfo.tm_sec;
  else if ((millis() - previousTimeStamp) > 1000 * 60)
    addFakeSec = timeinfo.tm_sec;
  else
  {
    addFakeSec++;
    if (addFakeSec > 60)
      addFakeSec = 0;
  }

  String t;
  if (loggingType == 1) t = String(timeinfo.tm_year + 1900) + "-" + String(timeinfo.tm_mon + 1) + "-" + String(timeinfo.tm_mday) + "T" + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(addFakeSec) + "-0000";
  if (loggingType == 3) t = String(timeinfo.tm_year + 1900) + "/" + String(timeinfo.tm_mon + 1) + "/" + String(timeinfo.tm_mday) + " " + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(addFakeSec);

  String head = t + "," +                                    //timestamp
                deviceID + "," +                             //1
                MACID + "," +                                //2
                loggingCounter + "," +                       //3
                bootCount + "," +                            //4
                FW_VERSION + "," +                           //5
                tDHTt + "," +                                //6
                String(millis()) + "," +                     //7
                String(millis() - previousTimeStamp) + "," + //8
                (float)debuglevel / 10 + "," +               //lon
                (float)ConfigurationVersion / 10.0 + "," +   //lat
                String((t, isServer) ? "1" : "0") + ",";     //elev

  //add headers is new or ended previous msg with |

  int l = networkLogBuffer.length();
  if (l == 0)
  {
    networkLogBuffer = head + networkLogBuffer + strMessage + "|";
    previousTimeStamp = millis();
    return;
  }

  if (networkLogBuffer.substring(l - 1) == "|")
  {
    networkLogBuffer += head;
    previousTimeStamp = millis();
  }

  if (newLineHint == 1)
  {
    if ((networkLogBuffer.substring(l - 1) == ","))
    {
      networkLogBuffer += "|" + head;
      previousTimeStamp = millis();
    }
  }

  networkLogBuffer += strMessage;

  if (newLineHint > 1)
    networkLogBuffer += "|";

  return;
}

int networklogThis(String message, bool asProxy = false)
{

  if (logTarget == "")
    return 0; //value empty - network logging off
  if (DEBUGLEVEL == 6)
    return 0;
  timerWrite(timer, 0); //reset timer (feed watchdog)
  NetworkResponse myNetworkResponse;

  String m;

  switch (loggingType)
  {
    case 1: ///  thingspeak

      message.replace("1900-1-0T0", "1900-1-1T");
      message = "write_api_key=NGOL1T65IJHKTURU&time_format=absolute&updates=" + message;
      myNetworkResponse = httpSecurePost(loggerHost, 443, logTarget, message, "HTTP/1.1 202 Accepted");

      break;

    case 2: //// PHPSERVER


      // message.replace("\n", " | ");
      m = "msg=" + message;

      myNetworkResponse = httpRequest(loggerHost, loggerHostPort, "POST", logTarget, m, "Logged successfully", 0);

      break;

    case 3: //// IFTTT

      Serial.println(loggerHost);
      myNetworkResponse = httpRequest(loggerHost, loggerHostPort, "POST", logTarget, "{\"value1\":\"" + message + "\"}", "Congratulations! You've", 0);

      break;
  }

  if (myNetworkResponse.resultCode == 0)
  {
    failedLogging2NetworkCounter = 0;
    logThis(2, "Log sent and received successfully.", 3);
  }
  else
  {
    logThis(1, "FAILED LOGGING TO NETWORK", 3);
    digitalWrite(red, HIGH);
    vTaskDelay(60);
    digitalWrite(red, LOW);
    RTCpanicStateCode = 3;
    failedLogging2NetworkCounter++;
    if (failedLogging2NetworkCounter == 5)
      boardPanic(2);
    return 1;
  }
  if (readEEPROM(1) == "PANIC=2") writeString(1, "PANIC=0");
  return 0;
}
