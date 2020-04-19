
int learnCode(){
  
  logThis("Start Learning......",2);
  unsigned long timer = millis();
  
  irrecv.enableIRIn();  // Start the receiver
  bool found = false;
  irrecv.resume();

  while ((millis() - timer < learningTH) && (!found)){      
   
     blinkLiveLedFast();

    if (irrecv.decode(&results)) {
  
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
    // Check if we got an IR message that was to big for our capture buffer.
    if (results.overflow)
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    // Display the library version the message was captured with.
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_ "\n");
    // Display the basic output of what we found.
    Serial.print(resultToHumanReadableBasic(&results));
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();  // Feed the WDT as the text output can take a while to print.
    // Output the results as source code
    Serial.println(resultToSourceCode(&results));
    Serial.println(getCorrectedRawLength(&results));
    Serial.println(resultToTimingInfo(&results));
    Serial.println(resultToHumanReadableBasic(&results));
    Serial.println(getCorrectedRawLength(&results));
    
    Serial.println();    // Blank line between entries

    Serial.println(buildRawCode(&results));

    yield();             // Feed the WDT (again)
    found = true;
    writeString(10,buildRawCode(&results));
    return 0;
  }    
  }

  Serial.println("Didn't get anything");   
        return 1;
  

}
