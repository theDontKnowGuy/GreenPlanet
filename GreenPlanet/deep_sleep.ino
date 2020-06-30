
void callback() {

}

void wakeupReason() {

  touchPin = esp_sleep_get_touchpad_wakeup_status();

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_TIMER : logThis(3, "Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : logThis(3, "Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : logThis(3, "Wakeup caused by ULP program"); break;
    default : logThis(3, "Wakeup was not caused by deep sleep: " + String(wakeup_reason)); break;
  }

}

void RTC_IRAM_ATTR esp_wake_deep_sleep(void)
{
  esp_default_wake_deep_sleep();
  static RTC_RODATA_ATTR const char wakeuptext[] = "Good Morning.... Waking up from sleep with code %d\n";
  ets_printf(wakeuptext, RTCpanicStateCode);
}

void gotoSleep(int timeToSleep)
{
  gotoSleep(timeToSleep, 0);
}

void gotoSleep(int timeToSleep, int panicCode)
{
  esp_random();
  timeToSleep = timeToSleep - (sleepRandFactor / 2)  + random(sleepRandFactor);
  logThis(2, "Going to sleep now for " + String(timeToSleep) + " secs....", 1);
  logThis(2, "This cycle took: " + String(millis()), 1);

  switch (panicCode)
  {
    case 0: //normal
      { int res = networklogThis(networkLogBuffer);
        if (res != 0) boardPanic(2);
        Serial.println("This cycle really took (including last logging network roundtrip): " + String(millis()));
        break;
      }
    case 1:
      //no network, reset failed, will wait
      RTCpanicStateCode = 1;
      Serial.println("No network. will wait");
      for (int i = 0; i < 3; i++)
      {
        digitalWrite(red, HIGH);
        vTaskDelay(20);
        digitalWrite(red, LOW);
        vTaskDelay(20);
      }
      break;
  }

  touchAttachInterrupt(T4, callback, WakeUpSensorThreshold);
  esp_sleep_enable_touchpad_wakeup();

  esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO);

  Serial.println("Good night now.");
  Serial.flush();
  vTaskDelay(20);
  esp_deep_sleep_start();
}
