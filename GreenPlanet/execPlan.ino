int execPlan(int IRCodeID)
{
  IRsend irsend(kIrLed);      // Set the GPIO to be used to sending the message.
  irsend.begin();

  int idxIRPlanToRun = -1;
  for (int i = 0; i <= inxParticipatingIRCodes; i++)
  {
    if (IRCodeID == myIRcode[i].IRcodeID)
      idxIRPlanToRun = i;
  }

  logThis(2, "Now executing IRcode# " + String(IRCodeID) + " - " + myIRcode[idxIRPlanToRun].IRcodeDescription, 2);

  for (int i = 0; i < 3; i++)
  {

    logThis(3, "Calling IR sequance", 1);
    irsend.sendRaw(myIRcode[idxIRPlanToRun].IRCodeBitStream, myIRcode[idxIRPlanToRun].IRCodeBitStreamLength, 38); // Send a raw data capture at 38kHz.
    digitalWrite(green, HIGH);
    vTaskDelay(50);
    digitalWrite(green, LOW);
    vTaskDelay(delayBetweenExecs * 3);
  }
  return 0;
}
