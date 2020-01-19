#ifdef ESP8266 // NODEMCU based on ESP8266

#define CLASS_ESP8266 "82"

#include <log4ino/Log.h>
#include <main4ino/Timing.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <EspSaveCrash.h>
#include <FS.h>
#include "Boards.h"

#define MAX_DEEP_SLEEP_PERIOD_SECS 2100 // 35 minutes

#ifndef WIFI_DELAY_MS
#define WIFI_DELAY_MS 2000
#endif // WIFI_DELAY_MS

#ifndef URL_PRINT_MAX_LENGTH
#define URL_PRINT_MAX_LENGTH 20
#endif // URL_PRINT_MAX_LENGTH

#define WAIT_BEFORE_HTTP_MS 100

HTTPClient httpClient;

WifiNetwork detectWifi(const char *ssid, const char *ssidb) {
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    String s = WiFi.SSID(i);
    if (strcmp(s.c_str(), ssid) == 0) {
      log(CLASS_ESP8266, Info, "Wifi found '%s'", ssid);
      return WifiMainNetwork;
    } else if (strcmp(s.c_str(), ssidb) == 0) {
      log(CLASS_ESP8266, Info, "Wifi found '%s'", ssidb);
      return WifiBackupNetwork;
    }
  }
  return WifiNoNetwork;
}

bool initializeWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfConnected, int retries) {
  wl_status_t status;

  log(CLASS_ESP8266, Debug, "Init wifi (sic=%s) '%s' (or '%s')...", BOOL(skipIfConnected), ssid, ssidb);
  bool wifiIsOff = (wifi_get_opmode() == NULL_MODE);
  if (wifiIsOff) {
    log(CLASS_ESP8266, Debug, "Wifi off, turning on...");
    wifi_fpm_do_wakeup();
    wifi_fpm_close();
    wifi_set_opmode(STATION_MODE);
    wifi_station_connect();
  } else {
    log(CLASS_ESP8266, Debug, "Wifi on already");
  }

  if (skipIfConnected) { // check if connected
    log(CLASS_ESP8266, Debug, "Already connected?");
    status = WiFi.status();
    if (status == WL_CONNECTED) {
      log(CLASS_ESP8266, Info, "IP: %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
  } else {
    stopWifi();
  }

  log(CLASS_ESP8266, Debug, "Scanning...");
  WifiNetwork w = detectWifi(ssid, ssidb);

  log(CLASS_ESP8266, Debug, "Connecting...");
  WiFi.mode(WIFI_STA);
  delay(WIFI_DELAY_MS);
  switch (w) {
    case WifiMainNetwork:
      WiFi.begin(ssid, pass);
      break;
    case WifiBackupNetwork:
      WiFi.begin(ssidb, passb);
      break;
    default:
      return false;
  }

  int attemptsLeft = retries;
  while (true) {
    bool interrupt = lightSleepNotInterruptable(now(), WIFI_DELAY_MS / 1000, NULL);
    if (interrupt) {
      log(CLASS_ESP8266, Warn, "Wifi init interrupted");
      return false; // not connected
    }
    status = WiFi.status();
    log(CLASS_ESP8266, Debug, "..'%s'(%d left)", ssid, attemptsLeft);
    attemptsLeft--;
    if (status == WL_CONNECTED) {
      log(CLASS_ESP8266, Debug, "Connected! %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
    if (attemptsLeft < 0) {
      log(CLASS_ESP8266, Warn, "Connection to '%s' failed %d", ssid, status);
      return false; // not connected
    }
  }
}

void stopWifi() {
  log(CLASS_ESP8266, Debug, "W.Off.");
  WiFi.disconnect();
  delay(WIFI_DELAY_MS);
  WiFi.mode(WIFI_OFF); // to be removed after SDK update to 1.5.4
  delay(WIFI_DELAY_MS);
}

// TODO: add https support, which requires fingerprint of server that can be obtained as follows:
//  openssl s_client -connect dweet.io:443 < /dev/null 2>/dev/null | openssl x509 -fingerprint -noout -in /dev/stdin
int httpMethod(HttpMethod method, const char *url, const char *body, ParamStream *response, Table *headers, const char *fingerprint) {
  int errorCode;
  if (fingerprint == NULL) {
    httpClient.begin(url);
  } else {
    httpClient.begin(url, fingerprint);
  }
  log(CLASS_ESP8266, Debug, "> %s:..%s", HTTP_METHOD_STR(method), tailStr(url, URL_PRINT_MAX_LENGTH));

  int i = 0;
  while ((i = headers->next(i)) != -1) {
    httpClient.addHeader(headers->getKey(i), headers->getValue(i));
    i++;
  }
  switch(method) {
    case HttpPost:
      log(CLASS_ESP8266, Debug, "> POST '%s'", body);
      errorCode = httpClient.POST(body);
      if (response != NULL) {
        httpClient.writeToStream(response);
      }
      break;
    case HttpUpdate:
      log(CLASS_ESP8266, Debug, "> PUTT '%s'", body);
      errorCode = httpClient.PUT(body);
      if (response != NULL) {
        httpClient.writeToStream(response);
      }
      break;
    case HttpGet:
      log(CLASS_ESP8266, Debug, "> GET");
      errorCode = httpClient.GET();
      if (response != NULL) {
        httpClient.writeToStream(response);
      }
      break;
    default:
      log(CLASS_ESP8266, Error, "Not supported %d HTTP method", (int)method);
      errorCode = -1;

  }

  log(CLASS_ESP8266, Debug, "< %d", errorCode);
  httpClient.end();
  delay(WAIT_BEFORE_HTTP_MS);
  return errorCode;
}

bool readFile(const char *fname, Buffer *content) {
  bool success = false;
  SPIFFS.begin();
  bool exists = SPIFFS.exists(fname);
  if (!exists) {
    log(CLASS_ESP8266, Warn, "File does not exist: %s", fname);
    content->clear();
    success = false;
  } else {
    File f = SPIFFS.open(fname, "r");
    if (!f) {
      log(CLASS_ESP8266, Warn, "File reading failed: %s", fname);
      content->clear();
      success = false;
    } else {
      String s = f.readString();
      content->load(s.c_str());
      log(CLASS_ESP8266, Debug, "File read: %s", fname);
      success = true;
    }
  }
  SPIFFS.end();
  return success;
}

bool writeFile(const char *fname, const char *content) {
  bool success = false;
  SPIFFS.begin();
  File f = SPIFFS.open(fname, "w+");
  if (!f) {
    log(CLASS_ESP8266, Warn, "File writing failed: %s", fname);
    success = false;
  } else {
    f.write((const uint8_t *)content, strlen(content));
    f.close();
    log(CLASS_ESP8266, Debug, "File written: %s", fname);
    success = true;
  }
  SPIFFS.end();
  return success;
}

void updateFirmware(const char *url, const char *currentVersion) { // already connected to wifi
  ESP8266HTTPUpdate updater;

  log(CLASS_ESP8266, Warn, "Updating firmware from '%s'...", url);

  t_httpUpdate_return ret = updater.update(url, currentVersion);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      log(CLASS_ESP8266,
          Error,
          "HTTP_UPDATE_FAILED Error (%d): %s\n",
          ESPhttpUpdate.getLastError(),
          ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      log(CLASS_ESP8266, Info, "No updates.");
      break;
    case HTTP_UPDATE_OK:
      log(CLASS_ESP8266, Info, "Done!");
      break;
    default:
      log(CLASS_ESP8266, Warn, "Unknown response %d", (int)ret);
      break;
  }
}
void deepSleepNotInterruptable(time_t cycleBegin, time_t periodSecs) {
  log(CLASS_ESP8266, Info, "DS(period=%d)", (int)periodSecs);
  deepSleepNotInterruptableSecs(cycleBegin, periodSecs);
  delay(5000); // the above statement is async, wait until effective
}

bool lightSleepInterruptable(time_t cycleBegin, time_t periodSecs, int miniPeriodMsec, bool (*interrupt)(), void (*heartbeat)()) {
  log(CLASS_ESP8266, Debug, "LS(%ds)...", (int)periodSecs);
  if (interrupt()) { // first quick check before any time considerations
    return true;
  }
  while (now() < cycleBegin + periodSecs) {
    if (interrupt()) {
      return true;
    }
    if (heartbeat) heartbeat();
    delay(miniPeriodMsec);
  }
  return false;
}

void deepSleepNotInterruptableSecs(time_t cycleBegin, time_t periodSecs) {
  time_t p = CONSTRAIN_VALUE(periodSecs, 0, MAX_DEEP_SLEEP_PERIOD_SECS);
  log(CLASS_ESP8266, Info, "DS(%ds)...", (int)p);
  time_t spentSecs = now() - cycleBegin;
  time_t leftSecs = p - spentSecs;
  if (leftSecs > 0) {
    ESP.deepSleep(leftSecs * FACTOR_USEC_TO_SEC_DEEP_SLEEP, WAKE_RF_DEFAULT);
  }
}

bool lightSleepNotInterruptable(time_t cycleBegin, time_t periodSecs, void (*heartbeat)()) {
  log(CLASS_ESP8266, Debug, "LS(%ds)...", (int)periodSecs);
  while (now() < cycleBegin + periodSecs) {
    if (heartbeat) heartbeat();
    delay(1000);
  }
  return false;
}

#endif // ESP8266
