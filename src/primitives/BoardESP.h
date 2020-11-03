#ifndef ESP_INC
#define ESP_INC

#define CLASS_ESP "BO"
#define NRO_ATTEMPTS 5
#define WAIT_BEFORE_HTTP_MS 100

#ifndef URL_PRINT_MAX_LENGTH
#define URL_PRINT_MAX_LENGTH 20
#endif // URL_PRINT_MAX_LENGTH

#ifndef LOG_LENGTH_FACTOR_IMPORTANT_MESSAGES
#define LOG_LENGTH_FACTOR_IMPORTANT_MESSAGES 3
#endif // LOG_LENGTH_FACTOR_IMPORTANT_MESSAGES

#define MAX_DEEP_SLEEP_PERIOD_SECS 2100 // 35 minutes
#define MIN_DEEP_SLEEP_PERIOD_SECS 1 // 1 second

#ifndef WIFI_DELAY_MS
#define WIFI_DELAY_MS 2000
#endif // WIFI_DELAY_MS

#define ABORT_LOG_FILENAME "/abort.log"

#define ABORT_LOG_FILE_MAX_LENGTH 512
#define ABORT_LOG_HEADER_LENGTH 64
#define ABORT_LOG_BODY_MAX_LENGTH (ABORT_LOG_FILE_MAX_LENGTH - ABORT_LOG_HEADER_LENGTH)
#define STACKTRACE_MAX_LENGTH 512

#ifndef NW_LOG_BUFFER_MAX_LENGTH
#define NW_LOG_BUFFER_MAX_LENGTH 1024
#endif // NW_LOG_BUFFER_MAX_LENGTH

static_assert(ABORT_LOG_FILE_MAX_LENGTH > ABORT_LOG_HEADER_LENGTH, "Too much header in the abort log file");
static_assert(ABORT_LOG_FILE_MAX_LENGTH <= NW_LOG_BUFFER_MAX_LENGTH, "Too much info to report error via logs (if abort)");
static_assert(ABORT_LOG_HEADER_LENGTH + STACKTRACE_MAX_LENGTH <= NW_LOG_BUFFER_MAX_LENGTH, "Too much stacktrace info to report error via logs (if failure)");

void espWdtFeed();

CustomHTTPClient httpClient;
std::function<void ()> httpClientEnd = []() { httpClient.end();};

void deepSleepNotInterruptableSecsRaw(time_t periodSecs);

WifiNetwork detectWifi(const char *ssid, const char *ssidb) {
  for (int a = 0; a < NRO_ATTEMPTS; a++) {
    log(CLASS_ESP, Debug, "Wifi attempt %d", a);
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
      espWdtFeed();
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
  espWdtFeed();
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
      errorCode = httpClient.sendRequestChunked("POST", body, espWdtFeed);
      break;
    case HttpUpdate:
      log(CLASS_ESPX, Debug, "> PUT");
      errorCode = httpClient.sendRequestChunked("PUT", body, espWdtFeed);
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
  time_t spentSecs = now() - cycleBegin;
  time_t leftSecs = CONSTRAIN_VALUE(periodSecs - spentSecs, MIN_DEEP_SLEEP_PERIOD_SECS, MAX_DEEP_SLEEP_PERIOD_SECS);
  log(CLASS_ESPX, Info, "DS(%ds)...", (int)leftSecs);
  deepSleepNotInterruptableSecsRaw(leftSecs);
}

bool lightSleepInterruptable(time_t cycleBegin, time_t periodSecs, int miniPeriodMsec, bool (*interrupt)(), void (*heartbeat)()) {
  log(CLASS_ESPX, Debug, "LS(%ds)...", (int)periodSecs);
  if (interrupt()) { // first quick check before any time considerations
    return true;
  }
  while (now() < cycleBegin + periodSecs) {
    espWdtFeed();
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
    espWdtFeed();
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

/**
 * Log a line 'str' by class 'clz' of level 'l' (being or not a 'newline') on different sinks:
 * - serial sink (by default)
 * - buffer sink (only if bufMsgLen > 0, at most using bufMsgLen for non important levels, or LOG_LENGTH_FACTOR_IMPORTANT_MESSAGES times that)
 */
void logLineOnto(const char *str, const char *clz, LogLevel l, bool newline, unsigned int bufMsgLen, Buffer* buf) {

#ifdef TIME_LOG
  Buffer time(8);
  int ts = (int)((millis()/1000) % 10000);
  time.fill("%04d|", ts);
#endif // TIME_LOG

  // MAIN SERIAL PRINT
#ifdef HEAP_LOG
  //Serial.print("HEA:");
  //Serial.print(ESP.getFreeHeap()); // caused a crash, reenable upon upgrade
  //Serial.print("|");
#endif // HEAP_LOG
#ifdef VCC_LOG
  Serial.print("VCC:");
  Serial.print(VCC_FLOAT);
  Serial.print("|");
#endif // VCC_LOG
  Serial.print(str);

  // BUFFER PRINT
  if (bufMsgLen > 0) {
#ifdef TIME_LOG
    if (newline) {
      buf->append(time.getBuffer());
    }
#endif // TIME_LOG
    unsigned int logFactor = ((l == User || l == Error || l == Warn)? LOG_LENGTH_FACTOR_IMPORTANT_MESSAGES : 1);
    unsigned int s = (unsigned int)(bufMsgLen * logFactor) + 1;
    char aux2[s];
    strncpy(aux2, str, s);
    aux2[s - 1] = 0;
    aux2[s - 2] = '\n';
    buf->append(aux2);
  }
}

void reportAbort(Buffer msg) {
  log(CLASS_PLATFORM, Error, "Abort!");
  logRaw(CLASS_PLATFORM, Error, msg.getBuffer());

  Buffer fheader(ABORT_LOG_HEADER_LENGTH); // header
  fheader.fill("ABORT msg=%s v=%s t=%ld", msg.getBuffer(), now(), STRINGIFY(PROJ_VERSION));

  logBuffer->last(ABORT_LOG_BODY_MAX_LENGTH); // body

  Buffer fcontent(ABORT_LOG_FILE_MAX_LENGTH); // full file content
  fcontent.fill("%s\n...\n%s\n...\n", fheader.getBuffer(), logBuffer->getBuffer());

  writeFile(ABORT_LOG_FILENAME, fcontent.getBuffer());
}


#endif // ESP_INC
