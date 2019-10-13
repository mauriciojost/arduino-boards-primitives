#ifdef ESP32 // NODEMCU based on ESP32

#define CLASS_ESP32 "32"

WifiNetwork detectWifi(const char *ssid, const char *ssidb);
bool initializeWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfConnected, int retries);
void stopWifi();
int httpGet(const char *url, ParamStream *response, Table *headers);
int httpPost(const char *url, const char *body, ParamStream *response, Table *headers);
bool readFile(const char *fname, Buffer *content);
bool writeFile(const char *fname, const char *content);
void updateFirmwareVersion(const char *url, const char *projVersion);
bool lightSleepInterruptable(time_t cycleBegin, time_t periodSecs);
bool lightSleepNotInterruptable(time_t cycleBegin, time_t periodSecs);
void deepSleepNotInterruptable(time_t cycleBegin, time_t periodSecs);

WifiNetwork detectWifi(const char *ssid, const char *ssidb) {
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    String s = WiFi.SSID(i);
    if (strcmp(s.c_str(), ssid) == 0) {
      log(CLASS_ESP32, Info, "Wifi found '%s'", ssid);
      return WifiMainNetwork;
    } else if (strcmp(s.c_str(), ssidb) == 0) {
      log(CLASS_ESP32, Info, "Wifi found '%s'", ssidb);
      return WifiBackupNetwork;
    }
  }
  return WifiNoNetwork;
}

bool initializeWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfConnected, int retries) {
  wl_status_t status;

  log(CLASS_ESP32, Info, "Init wifi '%s' (or '%s')...", ssid, ssidb);

  if (skipIfConnected) { // check if connected
    log(CLASS_ESP32, Debug, "Already connected?");
    status = WiFi.status();
    if (status == WL_CONNECTED) {
      log(CLASS_ESP32, Info, "IP: %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
  } else {
    stopWifi();
  }

  log(CLASS_ESP32, Debug, "Scanning...");
  WifiNetwork w = detectWifi(ssid, ssidb);

  log(CLASS_ESP32, Debug, "Connecting...");
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
    bool interrupt = lightSleepInterruptable(now(), WIFI_DELAY_MS / 1000);
    if (interrupt) {
      log(CLASS_ESP32, Warn, "Wifi init interrupted");
      return false; // not connected
    }
    status = WiFi.status();
    log(CLASS_ESP32, Debug, "..'%s'(%d left)", ssid, attemptsLeft);
    attemptsLeft--;
    if (status == WL_CONNECTED) {
      log(CLASS_ESP32, Debug, "Connected! %s", WiFi.localIP().toString().c_str());
      return true; // connected
    }
    if (attemptsLeft < 0) {
      log(CLASS_ESP32, Warn, "Connection to '%s' failed %d", ssid, status);
      return false; // not connected
    }
  }
}

void stopWifi() {
  log(CLASS_ESP32, Info, "W.Off.");
  WiFi.disconnect();
  delay(WIFI_DELAY_MS);
  WiFi.mode(WIFI_OFF); // to be removed after SDK update to 1.5.4
  delay(WIFI_DELAY_MS);
}

// TODO: add https support, which requires fingerprint of server that can be obtained as follows:
//  openssl s_client -connect dweet.io:443 < /dev/null 2>/dev/null | openssl x509 -fingerprint -noout -in /dev/stdin
int httpGet(const char *url, ParamStream *response, Table *headers) {
  httpClient.begin(url);
  int i = 0;
  while ((i = headers->next(i)) != -1) {
    httpClient.addHeader(headers->getKey(i), headers->getValue(i));
    i++;
  }
  log(CLASS_ESP32, Debug, "> GET:..%s", tailStr(url, URL_PRINT_MAX_LENGTH));
  int errorCode = httpClient.GET();
  log(CLASS_ESP32, Debug, "> GET:%d", errorCode);

  if (errorCode == HTTP_OK || errorCode == HTTP_NO_CONTENT) {
    if (response != NULL) {
      httpClient.writeToStream(response);
    }
  } else {
    int e = httpClient.writeToStream(&Serial);
    log(CLASS_ESP32, Error, "> GET(%d):%d %s", e, errorCode, httpClient.errorToString(errorCode).c_str());
  }
  httpClient.end();

  delay(WAIT_BEFORE_HTTP_MS);

  return errorCode;
}

int httpPost(const char *url, const char *body, ParamStream *response, Table *headers) {
  httpClient.begin(url);
  int i = 0;
  while ((i = headers->next(i)) != -1) {
    httpClient.addHeader(headers->getKey(i), headers->getValue(i));
    i++;
  }

  log(CLASS_ESP32, Debug, "> POST:..%s", tailStr(url, URL_PRINT_MAX_LENGTH));
  log(CLASS_ESP32, Debug, "> POST:'%s'", body);
  int errorCode = httpClient.POST(body);
  log(CLASS_ESP32, Debug, "> POST:%d", errorCode);

  if (errorCode == HTTP_OK || errorCode == HTTP_CREATED) {
    if (response != NULL) {
      httpClient.writeToStream(response);
    }
  } else {
    int e = httpClient.writeToStream(&Serial);
    log(CLASS_ESP32, Error, "> POST(%d):%d %s", e, errorCode, httpClient.errorToString(errorCode).c_str());
  }
  httpClient.end();

  delay(WAIT_BEFORE_HTTP_MS);

  return errorCode;
}

bool readFile(const char *fname, Buffer *content) {
  bool success = false;
  bool exists = SPIFFS.exists(fname);
  if (!exists) {
    log(CLASS_ESP32, Warn, "File does not exist: %s", fname);
    content->clear();
    success = false;
  } else {
    File f = SPIFFS.open(fname, "r");
    if (!f) {
      log(CLASS_ESP32, Warn, "File reading failed: %s", fname);
      content->clear();
      success = false;
    } else {
      String s = f.readString();
      content->load(s.c_str());
      log(CLASS_ESP32, Debug, "File read: %s", fname);
      success = true;
    }
  }
  return success;
}

bool writeFile(const char *fname, const char *content) {
  bool success = false;
  File f = SPIFFS.open(fname, "w+");
  if (!f) {
    log(CLASS_ESP32, Warn, "File writing failed: %s", fname);
    success = false;
  } else {
    f.print(content);
    f.close();
    log(CLASS_ESP32, Info, "File written: %s", fname);
    success = true;
  }
  return success;
}

void updateFirmwareVersion(const char *url, const char *projVersion) {
  HTTPUpdate updater;

  Settings *s = m->getModuleSettings();
  bool connected = initWifi(s->getSsid(), s->getPass(), false, 10);
  if (!connected) {
    log(CLASS_ESP32, Error, "Cannot connect to wifi");
    m->getNotifier()->message(0, USER_LCD_FONT_SIZE, "Cannot connect to wifi: %s", s->getSsid());
    return; // fail fast
  }

  log(CLASS_ESP32, Warn, "Current firmware '%s'", projVersion);
  log(CLASS_ESP32, Warn, "Updating firmware from '%s'...", url);

  t_httpUpdate_return ret = updater.update(httpClient.getStream(), url, projVersion);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      log(CLASS_ESP32, Error, "HTTP_UPDATE_FAILD Error (%d): %s\n", updater.getLastError(), updater.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      log(CLASS_ESP32, Debug, "No updates.");
      break;
    case HTTP_UPDATE_OK:
      log(CLASS_ESP32, Debug, "Done!");
      break;
  }
}
void deepSleepNotInterruptable(time_t cycleBegin, time_t periodSecs) {
  Timing t = Timing();
  time_t n = now();

  // light sleep to allow user intervention
  bool inte = lightSleepInterruptable(n, PRE_DEEP_SLEEP_WINDOW_SECS);

  // calculate time to boot regularly at the same moments
  t.setCurrentTime(n);
  t.setFreqEverySecs((int)periodSecs);
  time_t toSleepSecs = t.secsToMatch(MAX_DEEP_SLEEP_PERIOD_SECS);

  if (!inte) {
    // if no intervention, deep sleep
    deepSleepNotInterruptableSecs(n, toSleepSecs + DEEP_SLEEP_SUPPLEMENT_SECS);
  }
}

bool lightSleepInterruptable(time_t cycleBegin, time_t periodSecs, int miniPeriodMsec) {
  log(CLASS_ESP32, Debug, "Light Sleep(%ds)...", (int)periodSecs);
  if (haveToInterrupt()) { // first quick check before any time considerations
    return true;
  }
  while (now() < cycleBegin + periodSecs) {
    if (haveToInterrupt()) {
      return true;
    }
    heartbeat();
    delay(miniPeriodMsec);
  }
  return false;
}

void deepSleepNotInterruptableSecs(time_t cycleBegin, time_t periodSecs) {
  time_t p = (periodSecs > MAX_DEEP_SLEEP_PERIOD_SECS ? MAX_DEEP_SLEEP_PERIOD_SECS : periodSecs);
  log(CLASS_MAIN, Debug, "Deep Sleep(%ds)...", (int)p);
  time_t spentSecs = now() - cycleBegin;
  time_t leftSecs = p - spentSecs;
  if (leftSecs > 0) {
    // lcd->command(PCD8544_FUNCTIONSET | PCD8544_POWERDOWN);
    ESP.deepSleep(leftSecs * 1000000L);
  }
}

bool lightSleepNotInterruptable(time_t cycleBegin, time_t periodSecs, int miniPeriodMsec) {
  log(CLASS_ESP32, Debug, "Light Sleep(%ds)...", (int)periodSecs);
  while (now() < cycleBegin + periodSecs) {
    heartbeat();
    delay(miniPeriodMsec);
  }
  return false;
}

#endif // ESP32
