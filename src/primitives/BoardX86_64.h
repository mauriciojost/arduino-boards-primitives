#ifdef X86_64

#define CLASS_X8664 "BO"

#include <log4ino/Log.h>

#include <cstdio>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <main4ino/HttpResponse.h>
#include <main4ino/Buffer.h>

#include <primitives/Boards.h>

#define CL_MAX_LENGTH 65000
#define HTTP_CODE_KEY "HTTP_CODE:"
#define CURL_COMMAND_GET "/usr/bin/curl --silent -w '" HTTP_CODE_KEY "%%{http_code}' -XGET '%s'"
#define CURL_COMMAND_POST "/usr/bin/curl --silent -w '" HTTP_CODE_KEY "%%{http_code}' -XPOST '%s' -d '%s'"
#define CURL_COMMAND_PUT "/usr/bin/curl --silent -w '" HTTP_CODE_KEY "%%{http_code}' -XPUT '%s' -d '%s'"
unsigned long millis();

ParamStream* response = new ParamStream(1024);
std::function<void ()> nop = []() {};

bool initializeWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfConnected, int retries) {
  log(CLASS_X8664, Debug, "initWifi(%s, %s, %d)", ssid, pass, retries);
  return true;
}

void stopWifi() {
  log(CLASS_X8664, Debug, "stopWifi()");
}

HttpResponse httpMethod(HttpMethod m, const char *url, Stream* body, Table *headers, const char* fingerprints /* ignored */) {
  Buffer aux(CL_MAX_LENGTH);
  int httpCode = HTTP_BAD_REQUEST;
  Buffer b(1024);
  switch (m) {
    case (HttpPost): 
      body->readBytes(b.getUnsafeBuffer(), b.getCapacity());
      aux.fill(CURL_COMMAND_POST, url, b.getBuffer());
      log(CLASS_X8664, Debug, "POST: '%s'", aux.getBuffer());
      break;
    case (HttpGet): 
      aux.fill(CURL_COMMAND_GET, url);
      log(CLASS_X8664, Debug, "GET: '%s'", aux.getBuffer());
      break;
    case (HttpUpdate): 
      body->readBytes(b.getUnsafeBuffer(), b.getCapacity());
      aux.fill(CURL_COMMAND_PUT, url, b.getBuffer());
      log(CLASS_X8664, Debug, "PUT: '%s'", aux.getBuffer());
      break;
    default:
      aux.fill("INVALID_METHOD");
  }
  for (KV kv = headers->next(KV()); kv.isValid(); kv = headers->next(kv)) {
    aux.append(" -H '");
    aux.append(kv.getKey());
    aux.append(": ");
    aux.append(kv.getValue());
    aux.append("'");
  }
  FILE *fp = popen(aux.getBuffer(), "r");
  if (fp == NULL) {
    log(CLASS_X8664, Warn, "HTTP method failed");
    return HttpResponse(HTTP_BAD_REQUEST, response, nop);
  }
  while (fgets(aux.getUnsafeBuffer(), CL_MAX_LENGTH - 1, fp) != NULL) {
    const char *codeStr = aux.since(HTTP_CODE_KEY);
    httpCode = (codeStr != NULL ? atoi(codeStr + strlen(HTTP_CODE_KEY)) : HTTP_BAD_REQUEST);
    if (response != NULL) {
      response->fillUntil(aux.getBuffer(), HTTP_CODE_KEY);
      log(CLASS_X8664, Debug, "-> %s", response->content());
    }
  }
  pclose(fp);
  return HttpResponse(httpCode, response, nop);
}

bool readFile(const char *fname, Buffer *content) {
  bool success = false;
  char c;
  int i = 0;
  FILE *fp = fopen(fname, "r");
  content->clear();
  if (fp != NULL) {
    while ((c = getc(fp)) != EOF) {
      content->append(c);
      i++;
    }
    fclose(fp);
    success = true;
  } else {
    log(CLASS_X8664, Warn, "Could not load file: %s", fname);
    success = false;
  }
  return success;
}

bool writeFile(const char *fname, const char *content) {
  bool success = false;
  FILE *file = fopen(fname, "w+");
  int results = fputs(content, file);
  if (results == EOF) {
    log(CLASS_X8664, Warn, "Failed to write %s ", fname);
    success = false;
  } else {
    success = true;
  }
  fclose(file);
  return success;
}

void updateFirmware(const char *url, const char *currentVersion) {
}
void deepSleepNotInterruptable(time_t cycleBegin, time_t periodSecs) {
  log(CLASS_X8664, Info, "DeepSleep(%ds)...", (int)periodSecs);
  sleep(1);
}

bool lightSleepInterruptable(time_t cycleBegin, time_t periodSecs, int miniPeriodMsec, bool (*interrupt)(), void (*heartbeat)()) {
  log(CLASS_X8664, Info, "Sleep(%ds)...", (int)periodSecs);
  sleep(1);
  return false;
}

bool lightSleepNotInterruptable(time_t cycleBegin, time_t periodSecs, int miniPeriodMsec, void (*heartbeat)()) {
  log(CLASS_X8664, Info, "Sleep(%ds)...", (int)periodSecs);
  sleep(1);
  return false;
}

unsigned long millis() {
  static unsigned long boot = -1;
  struct timespec tms;
  if (clock_gettime(CLOCK_REALTIME, &tms)) {
    log(CLASS_X8664, Warn, "Couldn't get time");
    return -1;
  }
  unsigned long m = tms.tv_sec * 1000 + tms.tv_nsec / 1000000;
  if (boot == -1) {
    boot = m;
  }
  log(CLASS_X8664, Debug, "Millis: %lu", m);
  return m - boot;
}

#endif // X86_64
