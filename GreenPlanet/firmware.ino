int checkForFirmwareUpdates(int newFWVersion)
{

  logThis(3, "Checking for firmware updates.", 2);
  logThis(3, "Current firmware version: " + String(FW_VERSION), 2);

  if (newFWVersion <= FW_VERSION)
  {
    logThis(3, "Already on the recent firmware.", 2);
    return 0;
  }

  logThis(1, "Available firmware version: " + String(newFWVersion), 2);

  String fwImageURL = String(fwUrlBase) + String(newFWVersion);
  if (isServer)
    fwImageURL = fwImageURL + "_s.bin";
  else
    fwImageURL = fwImageURL + "_c.bin";
  logThis(2, "Firmware version URL: " + fwImageURL, 2);
  if (networklogThis(networkLogBuffer) == 0)   // flush log to network before start fooling around
  {
    networkLogBuffer = "";
    logAge = 0;
  }

  WiFiClientSecure *client = new WiFiClientSecure;
  HTTPClient httpClient;
  httpClient.begin(*client, fwImageURL);
  //  int httpCode = httpClient.GET();
  timerWrite(timer, 0);                                 //reset timer (feed watchdog)
  timerAlarmWrite(timer, wdtTimeout * 1000 * 5, false); //set time in us
  timerAlarmEnable(timer);                              //enable interrupt

  t_httpUpdate_return ret = httpUpdate.update(*client, fwImageURL); /// FOR ESP32 HTTP FOTA

  switch (ret)
  {

    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str()); /// FOR ESP32          logThis(0, strErr, 2); /// FOR ESP32
      char currentString[64];
      sprintf(currentString, "\nHTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str()); /// FOR ESP32          break;

    case HTTP_UPDATE_NO_UPDATES:
      logThis(0, "HTTP_UPDATE_NO_UPDATES.", 2);
      break;
  }
  httpClient.end();
  delete client;
  return -1;
}
/*

  #include "esp_https_ota.h"

  char* erver_cert_pem_start = "-----BEGIN CERTIFICATE-----MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMTDkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0NlowSjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMTGkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EFq6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWAa6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIGCCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNvbTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9kc3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAwVAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcCARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAzMDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwuY3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsFAAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJouM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwuX4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlGPfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==-----END CERTIFICATE-----";

  esp_err_t do_firmware_upgrade()
  {
  esp_http_client_config_t config = {
    .url = "https://esp32tutorial.netsons.org/https_ota/factory_reset_02.bin",
    .cert_pem = erver_cert_pem_start,
  };
  esp_err_t ret = esp_https_ota(&config);
  if (ret == ESP_OK) {
    esp_restart();
  } else {
    return ESP_FAIL;
  }
  return ESP_OK;
  }
*/
