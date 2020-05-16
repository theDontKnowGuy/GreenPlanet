int execPlan(int IRCodeID)
{

  int idxIRPlanToRun = -1;
  for (int i = 0; i <= inxParticipatingIRCodes; i++)
  {
    if (IRCodeID == myIRcode[i].IRcodeID)
      idxIRPlanToRun = i;
  }

  logThis(2, "Now executing plan " + String(IRCodeID) + " - " + myIRcode[idxIRPlanToRun].IRcodeDescription, 2);

  for (int i = 0; i < 3; i++)
  {

    logThis(2, "Calling IR sequance", 1);
    irsend.sendRaw(myIRcode[idxIRPlanToRun].IRCodeBitStream, myIRcode[idxIRPlanToRun].IRCodeBitStreamLength, 38); // Send a raw data capture at 38kHz.
    digitalWrite(green, HIGH);
    delay(50);
    digitalWrite(green, LOW);
    delay(delayBetweenExecs * 3);
  }
  return 0;
}
