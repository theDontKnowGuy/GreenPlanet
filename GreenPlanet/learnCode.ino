#if defined (SERVER)

String learnCode()
{

  logThis("Start Learning......", 2);
  unsigned long learningTimer = millis();

  irrecv.enableIRIn(); // Start the receiver
  bool found = false;
  irrecv.resume();
  Serial.print("Setup: Executing on core ");
  Serial.println(xPortGetCoreID());
  while ((millis() - learningTimer < learningTH) && (!found))
  {

    blinkLiveLedFast();

    if (irrecv.decode(&results))
    {

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
      if (description.length())
        Serial.println(D_STR_MESGDESC ": " + description);
      //yield();  // Feed the WDT as the text output can take a while to print.
      // Output the results as source code
      Serial.println(resultToSourceCode(&results));
      Serial.println(getCorrectedRawLength(&results));
      Serial.println(resultToTimingInfo(&results));
      Serial.println(resultToHumanReadableBasic(&results));
      Serial.println(getCorrectedRawLength(&results));

      Serial.println(); // Blank line between entries

      // yield();             // Feed the WDT (again)
      found = true;

      String s = resultToHumanReadableBasic(&results);
      logThis(1, "New code learned: " + s, 2);
      //writeString(10, buildRawCode(&results));

      digitalWrite(green, LOW); //maybe the blink remained on
      return s;
    }
  }

  logThis(1, "Learn session ended without findings");
  digitalWrite(green, LOW); //maybe the blink remained on

  return "1";
}

#endif
