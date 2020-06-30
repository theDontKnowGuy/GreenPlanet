bool planDispatcher()
{

  bool fired = false;
  for (int i = 0; i < inxParticipatingPlans; i++)
    {
    if ((myOperationPlans[i].weekdays.indexOf(String(timeinfo.tm_wday + 1)) >= 0) &&
        (abs((timeinfo.tm_hour * 60 + timeinfo.tm_min) - (myOperationPlans[i].hour * 60 + myOperationPlans[i].minute)) <= 4))
    {
      if ((millis() - myOperationPlans[i].recentExecution < recessTime * 1000) && (!(myOperationPlans[i].recentExecution == 0)))
      {
        return false;
      }
      fired = true;
      myOperationPlans[i].recentExecution = millis();
      logThis(1, "Calling execution of plan  " + String(myOperationPlans[i].operationPlanID) + " - " + myOperationPlans[i].operationPlanName, 2);
      execPlan(int(myOperationPlans[i].IRcodeID));
    }
  }

  return fired;
}

int calcTime2Sleep()
{
  int timeDiff = sleepTime;

  for (int i = 0; i < inxParticipatingPlans; i++)
  { 
    if ((myOperationPlans[i].weekdays.indexOf(String(timeinfo.tm_wday + 1)) >= 0))
    { int gap = (myOperationPlans[i].hour - timeinfo.tm_hour) * 3600 + (myOperationPlans[i].minute - timeinfo.tm_min) * 60;
      if ((gap > 2) && (gap < timeDiff)) {
        timeDiff = gap;
      }
    }
  }
  logThis(3, "Going to wake in " + String(timeDiff), 2);
  return timeDiff;
}
