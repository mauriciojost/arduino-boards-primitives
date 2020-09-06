#ifdef ESP32 // NODEMCU based on ESP32

#define CLASS_ESPX "32"

#include <log4ino/Log.h>
#include <main4ino/Timing.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <SPIFFS.h>
//#include <EspSaveCrash.h> // not found for esp32 yet
#include <FS.h>
#include <primitives/Boards.h>
#include <primitives/CustomHTTPClient.h>
#include <primitives/BoardESP.h>

void espWdtFeed() {
  yield();
}

bool initializeWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfConnected, int retries) {
  wl_status_t status;

  log(CLASS_ESPX, Debug, "WifiIn(%s) '%s'/'%s'", BOOL(skipIfConnected), ssid, ssidb);

  if (skipIfConnected) { // check if connected
    log(CLASS_ESPX, Debug, "Already connected?");
    status = WiFi.status();
    if (status == WL_CONNECTED) {
      log(CLASS_ESPX, Info, "IP: %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
  } else {
    stopWifi();
  }

  log(CLASS_ESPX, Debug, "Scanning...");
  WifiNetwork w = detectWifi(ssid, ssidb);

  WiFi.mode(WIFI_STA);
  delay(WIFI_DELAY_MS);
  switch (w) {
    case WifiMainNetwork:
      log(CLASS_ESPX, Debug, "%s => Connecting...", ssid);
      WiFi.begin(ssid, pass);
      break;
    case WifiBackupNetwork:
      log(CLASS_ESPX, Debug, "%s => Connecting...", ssidb);
      WiFi.begin(ssidb, passb);
      break;
    default:
      return false;
  }

  int attemptsLeft = retries;
  while (true) {
    espWdtFeed();
    bool interrupt = lightSleepNotInterruptable(now(), WIFI_DELAY_MS / 1000, NULL);
    if (interrupt) {
      log(CLASS_ESPX, Warn, "Wifi init interrupted");
      return false; // not connected
    }
    status = WiFi.status();
    log(CLASS_ESPX, Debug, "...(%d left)", attemptsLeft);
    attemptsLeft--;
    if (status == WL_CONNECTED) {
      log(CLASS_ESPX, Debug, "Connected! %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
    if (attemptsLeft < 0) {
      log(CLASS_ESPX, Warn, "Connection failed %d", status);
      return false; // not connected
    }
  }
}

bool readFile(const char *fname, Buffer *content) {
  bool success = false;
  bool exists = SPIFFS.exists(fname);
  espWdtFeed();
  if (!exists) {
    log(CLASS_ESPX, Debug, "File does not exist: %s", fname);
    content->clear();
    success = false;
  } else {
    File f = SPIFFS.open(fname, "r");
    if (!f) {
      log(CLASS_ESPX, Debug, "File reading failed: %s", fname);
      content->clear();
      success = false;
    } else {
      String s = f.readString();
      content->load(s.c_str());
      log(CLASS_ESPX, Debug, "File read: %s", fname);
      success = true;
    }
  }
  return success;
}

bool writeFile(const char *fname, const char *content) {
  espWdtFeed();
  bool success = false;
  File f = SPIFFS.open(fname, "w+");
  if (!f) {
    log(CLASS_ESPX, Warn, "File writing failed: %s", fname);
    success = false;
  } else {
    f.print(content);
    f.close();
    log(CLASS_ESPX, Debug, "File written: %s", fname);
    success = true;
  }
  return success;
}

void updateFirmware(const char *url, const char *currentVersion) { // already connected to wifi
  espWdtFeed();
  HTTPUpdate updater;

  log(CLASS_ESPX, Warn, "Updating firmware from '%s'...", url);

  t_httpUpdate_return ret = updater.update(httpClient.getStream(), url, currentVersion);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      log(CLASS_ESPX,
          Warn,
          "UPGR %d/%d/'%s'\n",
          ret,
          updater.getLastError(),
          updater.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      log(CLASS_ESPX, Info, "No updates.");
      break;
    case HTTP_UPDATE_OK:
      log(CLASS_ESPX, Info, "Done!");
      break;
    default:
      log(CLASS_ESPX, Warn, "Unknown response %d", (int)ret);
      break;
  }
}
void deepSleepNotInterruptableSecs(time_t cycleBegin, time_t periodSecs) {
  time_t p = CONSTRAIN_VALUE(periodSecs, 0, MAX_DEEP_SLEEP_PERIOD_SECS);
  log(CLASS_ESPX, Info, "DS(%ds)...", (int)p);
  time_t spentSecs = now() - cycleBegin;
  time_t leftSecs = p - spentSecs;
  if (leftSecs > 0) {
    ESP.deepSleep(leftSecs * FACTOR_USEC_TO_SEC_DEEP_SLEEP);
  }
}


#endif // ESP32
