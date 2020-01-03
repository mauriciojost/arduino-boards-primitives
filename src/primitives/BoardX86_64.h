#ifdef X86_64

#define CLASS_X8664 "64"

#include <log4ino/Log.h>

#include <cstdio>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "Boards.h"

#define CL_MAX_LENGTH 65000
#define HTTP_CODE_KEY "HTTP_CODE:"
#define CURL_COMMAND_GET "/usr/bin/curl --silent -w '" HTTP_CODE_KEY "%%{http_code}' -XGET '%s'"
#define CURL_COMMAND_POST "/usr/bin/curl --silent -w '" HTTP_CODE_KEY "%%{http_code}' -XPOST '%s' -d '%s'"
#define CURL_COMMAND_PUT "/usr/bin/curl --silent -w '" HTTP_CODE_KEY "%%{http_code}' -XPUT '%s' -d '%s'"
unsigned long millis();


bool initializeWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfConnected, int retries) {
  log(CLASS_X8664, Debug, "initWifi(%s, %s, %d)", ssid, pass, retries);
  return true;
}

void stopWifi() {
  log(CLASS_X8664, Debug, "stopWifi()");
}

int httpGet(const char *url, ParamStream *response, Table *headers) {
  Buffer aux(CL_MAX_LENGTH);
  int httpCode = HTTP_BAD_REQUEST;
  aux.fill(CURL_COMMAND_GET, url);
  int i = 0;
  while ((i = headers->next(i)) != -1) {
    aux.append(" -H '");
    aux.append(headers->getKey(i));
    aux.append(": ");
    aux.append(headers->getValue(i));
    aux.append("'");
    i++;
  }
  log(CLASS_X8664, Debug, "GET: '%s'", aux.getBuffer());
  FILE *fp = popen(aux.getBuffer(), "r");
  if (fp == NULL) {
    return HTTP_BAD_REQUEST;
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
  return httpCode;
}

int httpPost(const char *url, const char *body, ParamStream *response, Table *headers) {
  Buffer aux(CL_MAX_LENGTH);
  int httpCode = HTTP_BAD_REQUEST;
  aux.fill(CURL_COMMAND_POST, url, body);
  int i = 0;
  while ((i = headers->next(i)) != -1) {
    aux.append(" -H '");
    aux.append(headers->getKey(i));
    aux.append(": ");
    aux.append(headers->getValue(i));
    aux.append("'");
    i++;
  }
  log(CLASS_X8664, Debug, "POST: '%s'", aux.getBuffer());
  FILE *fp = popen(aux.getBuffer(), "r");
  if (fp == NULL) {
    log(CLASS_X8664, Warn, "POST failed");
    return HTTP_BAD_REQUEST;
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
  return httpCode;
}

int httpPut(const char *url, const char *body, ParamStream *response, Table *headers) {
  Buffer aux(CL_MAX_LENGTH);
  int httpCode = HTTP_BAD_REQUEST;
  aux.fill(CURL_COMMAND_PUT, url, body);
  int i = 0;
  while ((i = headers->next(i)) != -1) {
    aux.append(" -H '");
    aux.append(headers->getKey(i));
    aux.append(": ");
    aux.append(headers->getValue(i));
    aux.append("'");
    i++;
  }
  log(CLASS_X8664, Debug, "PUT: '%s'", aux.getBuffer());
  FILE *fp = popen(aux.getBuffer(), "r");
  if (fp == NULL) {
    log(CLASS_X8664, Warn, "PUT failed");
    return HTTP_BAD_REQUEST;
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
  return httpCode;
}

// TODO: add https support, which requires fingerprint of server that can be obtained as follows:
//  openssl s_client -connect dweet.io:443 < /dev/null 2>/dev/null | openssl x509 -fingerprint -noout -in /dev/stdin
int httpMethod(HttpMethod method, const char *url, const char *body, ParamStream *response, Table *headers, const char *fingerprint) {
  int errorCode;
  switch(method) {
    case HttpPost:
      errorCode = httpPost(url, body, response, headers);
      break;
    case HttpUpdate:
      errorCode = httpPut(url, body, response, headers);
      break;
    case HttpGet:
      errorCode = httpGet(url, response, headers);
      break;
    default:
      log(CLASS_X8664, Error, "Not supported %d HTTP method", (int)method);
      errorCode = -1;

  }
  return errorCode;
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
