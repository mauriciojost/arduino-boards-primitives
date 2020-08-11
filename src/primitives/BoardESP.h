#ifndef ESP_INC
#define ESP_INC

#define CLASS_ESP "ES"
#define NRO_ATTEMPTS 5
#define WAIT_BEFORE_HTTP_MS 100

#ifndef URL_PRINT_MAX_LENGTH
#define URL_PRINT_MAX_LENGTH 20
#endif // URL_PRINT_MAX_LENGTH


#define MAX_DEEP_SLEEP_PERIOD_SECS 2100 // 35 minutes



#ifndef WIFI_DELAY_MS
#define WIFI_DELAY_MS 2000
#endif // WIFI_DELAY_MS


CustomHTTPClient httpClient;
std::function<void ()> httpClientEnd = []() { httpClient.end();};

WifiNetwork detectWifi(const char *ssid, const char *ssidb) {
  for (int a = 0; a < NRO_ATTEMPTS; a++) {
    log(CLASS_ESP, Debug, "Wifi attempt %d", a);
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
      ESP.wdtFeed();
      String s = WiFi.SSID(i);
      if (strcmp(s.c_str(), ssid) == 0) {
        log(CLASS_ESP, Debug, "Wifi found '%s'", ssid);
        return WifiMainNetwork;
      } else if (strcmp(s.c_str(), ssidb) == 0) {
        log(CLASS_ESP, Debug, "Wifi found '%s'", ssidb);
        return WifiBackupNetwork;
      }
    }
  }
  return WifiNoNetwork;
}


HttpResponse httpMethod(HttpMethod method, const char *url, Stream *body, Table *headers, const char *fingerprint) {
  int errorCode;
  ESP.wdtFeed();
  if (fingerprint == NULL) {
    httpClient.begin(url);
  } else {
    httpClient.begin(url, fingerprint);
  }
  log(CLASS_ESPX, Fine, "> %s:..'%s'", HTTP_METHOD_STR(method), tailStr(url, URL_PRINT_MAX_LENGTH));
  for (KV kv = headers->next(KV()); kv.isValid(); kv = headers->next(kv)) {
    httpClient.addHeader(kv.getKey(), kv.getValue());
#ifdef INSECURE
    log(CLASS_ESPX, Fine, "- H '%s':'%s'", kv.getKey(), kv.getValue());
#endif // INSECURE
  }
  switch(method) {
    case HttpPost:
      log(CLASS_ESPX, Debug, "> POST");
      errorCode = httpClient.sendRequestChunked("POST", body);
      break;
    case HttpUpdate:
      log(CLASS_ESPX, Debug, "> PUT");
      errorCode = httpClient.sendRequestChunked("PUT", body);
      break;
    case HttpGet:
      log(CLASS_ESPX, Debug, "> GET");
      errorCode = httpClient.sendRequest("GET");
      break;
    default:
      log(CLASS_ESPX, Error, "Not supported %d HTTP method", (int)method);
      errorCode = -1;
  }

  log(CLASS_ESPX, Debug, "< %d (%s)", errorCode, httpClient.errorToString(errorCode).c_str());
  delay(WAIT_BEFORE_HTTP_MS);
  return HttpResponse(errorCode, httpClient.getStreamPtr(), httpClientEnd);
}


void deepSleepNotInterruptable(time_t cycleBegin, time_t periodSecs) {
  log(CLASS_ESPX, Info, "DS(period=%d)", (int)periodSecs);
  deepSleepNotInterruptableSecs(cycleBegin, periodSecs);
  delay(5000); // the above statement is async, wait until effective
}

bool lightSleepInterruptable(time_t cycleBegin, time_t periodSecs, int miniPeriodMsec, bool (*interrupt)(), void (*heartbeat)()) {
  log(CLASS_ESPX, Debug, "LS(%ds)...", (int)periodSecs);
  if (interrupt()) { // first quick check before any time considerations
    return true;
  }
  while (now() < cycleBegin + periodSecs) {
    ESP.wdtFeed();
    if (interrupt()) {
      return true;
    }
    if (heartbeat) heartbeat();
    delay(miniPeriodMsec);
  }
  return false;
}

bool lightSleepNotInterruptable(time_t cycleBegin, time_t periodSecs, void (*heartbeat)()) {
  log(CLASS_ESPX, Debug, "LS(%ds)...", (int)periodSecs);
  while (now() < cycleBegin + periodSecs) {
    ESP.wdtFeed();
    if (heartbeat) heartbeat();
    delay(1000);
  }
  return false;
}

void stopWifi() {
  log(CLASS_ESPX, Debug, "W.Off.");
  WiFi.disconnect();
  delay(WIFI_DELAY_MS);
  WiFi.mode(WIFI_OFF); // to be removed after SDK update to 1.5.4
  delay(WIFI_DELAY_MS);
}



#endif // ESP_INC
