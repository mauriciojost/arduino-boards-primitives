#ifndef BOARDS_INC

#define BOARDS_INC

#include <main4ino/Table.h>
#include <main4ino/ParamStream.h>
#include <main4ino/Buffer.h>
#include <main4ino/HttpCodes.h>
#include <main4ino/HttpMethods.h>
#include <main4ino/HttpResponse.h>

#define CLASS_BOARDS "BO"

enum WifiNetwork { WifiNoNetwork = 0, WifiMainNetwork, WifiBackupNetwork };

#ifndef MAIN4INOSERVER_API_HOST_BASE 
#define MAIN4INOSERVER_API_HOST_BASE "http://martinenhome.com/main4ino/prd"
#endif // MAIN4INOSERVER_API_HOST_BASE 

#ifndef FACTOR_USEC_TO_SEC_DEEP_SLEEP
#define FACTOR_USEC_TO_SEC_DEEP_SLEEP 1000000L
#endif // FACTOR_USEC_TO_SEC_DEEP_SLEEP

#ifndef SAFEPOINT_THRESHOLD_FAILURES
#define SAFEPOINT_THRESHOLD_FAILURES 0
#endif // SAFEPOINT_THRESHOLD_FAILURES

bool initializeWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfConnected, int retries);
void stopWifi();
HttpResponse httpMethod(HttpMethod method, const char *url, Stream *body, Table *headers, const char *fingerprint);
bool readFile(const char *fname, Buffer *content);
bool writeFile(const char *fname, const char *content);
void updateFirmware(const char *url, const char *currentVersion);
bool lightSleepInterruptable(time_t cycleBegin, time_t periodSecs, int miniPeriodMsec, bool (*interrupt)(), void (*heartbeat)());
bool lightSleepNotInterruptable(time_t cycleBegin, time_t periodSecs, void (*heartbeat)());

/* 
 * Deep sleep, lowest power consumption sleep followed by a reset. 
 * Instead of the duration of the sleep, this method receives the beginning of the 
 * cycle and the period until next cycle, and calculates the time to sleep to keep the right frequency.
 * @param cycleBegin when the cycle started (can use now() to start from now)
 * @param periodSecs what is the period between cycles
*/
void deepSleepNotInterruptable(time_t cycleBegin, time_t periodSecs);

void startup(
  const char* project,
  const char* version,
  const char* deviceId,
  int (*failuresInPast)(),
  void (*reportFailureLogs)(),
  void (*cleanFailures)(),
  void (*safepoint)()
) {
  int failures = failuresInPast();
  log(CLASS_BOARDS, User, "d=%s v=%s p=%s f=%d", deviceId, version, project, failures);
  if (failures > 0) {
    reportFailureLogs();
    cleanFailures();
    if (failures > SAFEPOINT_THRESHOLD_FAILURES) {
      safepoint();
    }
  }
}


#endif // BOARDS_INC
