#ifdef ESP8266 // NODEMCU based on ESP8266

#define CLASS_ESPX "BO"

#include <log4ino/Log.h>
#include <main4ino/Timing.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <EspSaveCrash.h>
#include <FS.h>
#include <primitives/Boards.h>
#include <primitives/CustomHTTPClient.h>
#include <primitives/BoardESP.h>
#include <EspSaveCrash.h>

EspSaveCrash espSaveCrash;

void espWdtFeed() {
  ESP.wdtFeed();
}

bool initializeWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfConnected, int retries) {
  wl_status_t status;

  log(CLASS_ESPX, Fine, "WifiIn(%s) '%s'/'%s'", BOOL(skipIfConnected), ssid, ssidb);
  bool wifiIsOff = (wifi_get_opmode() == NULL_MODE);
  if (wifiIsOff) {
    log(CLASS_ESPX, Fine, "Turning on...");
    wifi_fpm_do_wakeup();
    wifi_fpm_close();
    wifi_set_opmode(STATION_MODE);
    wifi_station_connect();
  } else {
    log(CLASS_ESPX, Fine, "On already");
  }

  if (skipIfConnected) { // check if connected
    log(CLASS_ESPX, Fine, "Already connected?");
    status = WiFi.status();
    if (status == WL_CONNECTED) {
      log(CLASS_ESPX, Fine, "IP: %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
  } else {
    stopWifi();
  }

  log(CLASS_ESPX, Fine, "Scanning...");
  WifiNetwork w = detectWifi(ssid, ssidb);

  WiFi.mode(WIFI_STA);
  delay(WIFI_DELAY_MS);
  switch (w) {
    case WifiMainNetwork:
      log(CLASS_ESPX, Fine, "%s => Connecting...", ssid);
      WiFi.begin(ssid, pass);
      break;
    case WifiBackupNetwork:
      log(CLASS_ESPX, Fine, "%s => Connecting...", ssidb);
      WiFi.begin(ssidb, passb);
      break;
    default:
      log(CLASS_ESPX, Error, "Unexpected WIFI");
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
    log(CLASS_ESPX, Fine, "...(%d left)", attemptsLeft);
    attemptsLeft--;
    if (status == WL_CONNECTED) {
      log(CLASS_ESPX, Fine, "Connected! %s", WiFi.localIP().toString().c_str());
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
  SPIFFS.begin();
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
  SPIFFS.end();
  return success;
}

bool writeFile(const char *fname, const char *content) {
  espWdtFeed();
  bool success = false;
  SPIFFS.begin();
  File f = SPIFFS.open(fname, "w+");
  if (!f) {
    log(CLASS_ESPX, Warn, "File writing failed: %s", fname);
    success = false;
  } else {
    f.write((const uint8_t *)content, strlen(content));
    f.close();
    log(CLASS_ESPX, Debug, "File written: %s", fname);
    success = true;
  }
  SPIFFS.end();
  return success;
}

void updateFirmware(const char *url, const char *currentVersion) { // already connected to wifi
  espWdtFeed();
  ESP8266HTTPUpdate updater;

  log(CLASS_ESPX, Warn, "Updating firmware from '%s'...", url);

  t_httpUpdate_return ret = updater.update(url, currentVersion);
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
void deepSleepNotInterruptableSecsRaw(time_t t) {
  ESP.deepSleep(t * FACTOR_USEC_TO_SEC_DEEP_SLEEP, WAKE_RF_DEFAULT);
}

bool wasHardwareReset() {
  return (ESP.getResetInfoPtr()->reason == REASON_EXT_SYS_RST);
}

int failuresInPast() {
  // Useful links for debugging:
  // https://links2004.github.io/Arduino/dc/deb/md_esp8266_doc_exception_causes.html
  // ./packages/framework-arduinoespressif8266@2.20502.0/tools/sdk/include/user_interface.h
  // https://bitbucket.org/mauriciojost/esp8266-stacktrace-translator/src/master/
  int e = espSaveCrash.count();
  Buffer fcontent(16);
  bool abrt = readFile(ABORT_LOG_FILENAME, &fcontent);
  return e + (abrt?1:0);
}

void reportFailureLogs() { // failures in previous life detected, report them now
  {
    Buffer fheader(ABORT_LOG_HEADER_LENGTH);
    fheader.fill("Had aborted!!! v=%s", STRINGIFY(PROJ_VERSION));
    logRaw(CLASS_ESPX, Error, fheader.getBuffer());
  }
  log(CLASS_ESPX, Error, "Crashes: %d", espSaveCrash.count());
  if (espSaveCrash.count() == 1) {
	  // report if only one to be reported, because reporting many
		// can cause (as it happened) too long for the report, causing WDT
		// to trigger, causing more crashes, so better limit to 1 the amount
		// of crashes to be reported at a time
    espWdtFeed();
    // crash, stacktrace available, no more
    
    Buffer* fcontent = new Buffer(STACKTRACE_MAX_LENGTH);
    

    espSaveCrash.print();
		espWdtFeed();
    espSaveCrash.print(fcontent->getUnsafeBuffer(), STACKTRACE_MAX_LENGTH - 8);
    logRaw(CLASS_ESPX, Error, fcontent->getBuffer());
    delete fcontent;
  } else if (espSaveCrash.count() > 1) {
    logRaw(CLASS_ESPX, Error, "Many crashes, previous one reported already");
  } else {
    // no crash, so abort should have taken place
    espWdtFeed();
    Buffer* fcontent = new Buffer(ABORT_LOG_FILE_MAX_LENGTH);
    bool abrt = readFile(ABORT_LOG_FILENAME, fcontent);
    if (abrt) {
      logRaw(CLASS_ESPX, Error, "==>" ABORT_LOG_FILENAME);
      logRaw(CLASS_ESPX, Error, fcontent->getBuffer());
      logRaw(CLASS_ESPX, Error, "<==");
    } else {
      log(CLASS_ESPX, Error, "File not found");
    }
    delete fcontent;
  }
}

void cleanFailures() {
  SPIFFS.begin();
  SPIFFS.remove(ABORT_LOG_FILENAME);
  SPIFFS.end();
  espSaveCrash.clear();
}


#endif // ESP8266
