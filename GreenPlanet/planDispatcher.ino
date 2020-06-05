bool planDispatcher()
{

  bool fired = false;

  for (int i = 0; i < inxParticipatingPlans; i++)
  {

    if ((myOperationPlans[i].weekdays.indexOf(String(timeinfo.tm_wday + 1)) >= 0) && ((timeinfo.tm_hour == myOperationPlans[i].hour) && (abs(timeinfo.tm_min - myOperationPlans[i].minute) <= 2)))
    {
      if ((millis() - myOperationPlans[i].recentExecution < recessTime * 1000) && (!(myOperationPlans[i].recentExecution == 0)))
      {
        return false;
      }
      fired = true;
      myOperationPlans[i].recentExecution = millis();
      logThis(3, "Calling execution of plan  " + String(myOperationPlans[i].operationPlanID) + " - " + myOperationPlans[i].operationPlanName, 2);
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

    if ((myOperationPlans[i].weekdays.indexOf(String(timeinfo.tm_wday + 1)) != 0) &&
        (myOperationPlans[i].hour - timeinfo.tm_hour >= 0) &&
        (myOperationPlans[i].minute - timeinfo.tm_min > 2))
    {
      int tmp_timediff = (myOperationPlans[i].hour - timeinfo.tm_hour) * 3600 +
                         (myOperationPlans[i].minute - timeinfo.tm_min) * 60;
      if (tmp_timediff < timeDiff)
        timeDiff = tmp_timediff;
    }
  }
  logThis(2, "Going to wake in " + String(timeDiff), 2);
  return timeDiff;
}
