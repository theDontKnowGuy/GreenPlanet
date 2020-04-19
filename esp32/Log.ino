
void logThis(String strMessage) {
  logThis(0, strMessage, 2);
  return;
}

void logThis(String strMessage, int newLineHint) {
  logThis(0, strMessage, newLineHint);
  return;
}

void logThis(int debuglevel, String strMessage) {
  logThis(debuglevel, strMessage, 2);
  return;
}

void logThis(int debuglevel, String strMessage, int newLineHint) {
  // newLineHint: 0- nothing 1- before 2- after 3- before and after

  if (DEBUGLEVEL < debuglevel) return;

  String pre = "";
  String post = "";

  if (newLineHint == 1) pre = "\n";
  if (newLineHint > 1 ) post = "\n";
  if (log2Serial) Serial.print(pre + strMessage + post);


  strMessage.replace(",", " * ");
  strMessage.replace("|", " ** ");
  strMessage.replace(char(34), char(33));
  strMessage.replace("\n", " ");
  strMessage.replace("&", "*");

  loggingCounter++;
  String head = "999," + deviceID + "," + String((isServer) ? "1" : "0") + "," + bootCount + "," + loggingCounter + "," + DHTt + "," + MACID + "," + String(millis()) + "," + String(millis() - previousTimeStamp) + ",,,,";
  //add headers is new or ended previous msg with |

  int l = networkLogBuffer.length();
  if (l == 0 )
  { networkLogBuffer = head;
    previousTimeStamp = millis();
  }

  if (l > 0 ) {
    if (networkLogBuffer.substring(l - 1 , l) == "|") {
      networkLogBuffer += head;
      previousTimeStamp = millis();
    }
  }

  if (newLineHint == 1 ) {
    if (!(networkLogBuffer.substring(l - 1, l) == ",")) {
      networkLogBuffer += "|" + head ;
      previousTimeStamp = millis();
    }
  }

  networkLogBuffer += strMessage;

  if (newLineHint > 1 )  networkLogBuffer += "|";

  return;
}

int networklogThis(String message, bool asProxy = false) {

  if (logTarget == "") return 0;   //value empty - network logging off
  if (DEBUGLEVEL == 6) return 0;
  timerWrite(timer, 0); //reset timer (feed watchdog)

  NetworkResponse myNetworkResponse;
  switch (loggingType) {
    case 1: ///  thingspeak
      message = "write_api_key=NGOL1T65IJHKTURU&time_format=relative&updates=" + message;
      myNetworkResponse = httpSecurePost("api.thingspeak.com", 443, logTarget, message, "HTTP/1.1 202 Accepted");
      break;

    case 2:   //// PHPSERVER
      String m;
      message.replace("\n", " | ");
      m = "msg = | " + deviceID + ", " + (isServer) ? " - client, " : " - server, " + String(WiFi.localIP().toString().c_str()) +  message;
      myNetworkResponse = httpRequest(loggerHost, loggerHostPort, "POST", logTarget, m, "Logged successfully", 0);
      break;
  }

  if (myNetworkResponse.resultCode == 0) {
    failedLogging2NetworkCounter =0;
  } else {
    Serial.println("FAILED LOGGING TO NETWORK");
    digitalWrite(red, HIGH); delay(60); digitalWrite(red, LOW);
    failedLogging2NetworkCounter++;
    if (failedLogging2NetworkCounter == 10) boardpanic(2);
    return 1;
  }
  return 0;
}
