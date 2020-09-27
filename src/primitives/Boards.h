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

#ifndef UPDATE_FIRMWARE_URL_MAX_LENGTH
#define UPDATE_FIRMWARE_URL_MAX_LENGTH 512
#endif // UPDATE_FIRMWARE_URL_MAX_LENGTH

// convention for firmware file name: firmware-<version>.<platform>.bin
// to replace: base + project + version + platform
#define FIRMWARE_UPDATE_URL MAIN4INOSERVER_API_HOST_BASE "/api/v1/session/%s/devices/%s/firmware/firmwares/%s/%s/content?version=%s"

#ifndef FACTOR_USEC_TO_SEC_DEEP_SLEEP
#define FACTOR_USEC_TO_SEC_DEEP_SLEEP 1000000L
#endif // FACTOR_USEC_TO_SEC_DEEP_SLEEP

#ifndef ROLLBACK_THRESHOLD_FAILURES
#define ROLLBACK_THRESHOLD_FAILURES 2
#endif // ROLLBACK_THRESHOLD_FAILURES

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

void updateFirmwareFromMain4ino(const char* session, const char *device, const char *project, const char* platform, const char *targetVersion, const char* currentVersion) {
#ifndef UPDATE_FIRMWARE_MAIN4INO_DISABLED
  Buffer aux(UPDATE_FIRMWARE_URL_MAX_LENGTH);
  aux.fill(FIRMWARE_UPDATE_URL, session, device, project, platform, targetVersion);
  updateFirmware(aux.getBuffer(), currentVersion);
#else // UPDATE_FIRMWARE_MAIN4INO_DISABLED
  log(CLASS_BOARDS, Warn, "Update disabled");
#endif // UPDATE_FIRMWARE_MAIN4INO_DISABLED
}

void startup(
  const char* project,
  const char* version,
  const char* deviceId,
  int (*failuresInPast)(),
  void (*reportFailureLogs)(),
  void (*cleanFailures)(),
  void (*rollback)()
) {
  log(CLASS_BOARDS, User, "*%s", project);
  log(CLASS_BOARDS, User, "*%s", version);
  log(CLASS_BOARDS, User, "*%s", deviceId);

  int failures = failuresInPast();
  if (failures > 0) {
    reportFailureLogs();
    cleanFailures();
    if (failures > ROLLBACK_THRESHOLD_FAILURES) {
      rollback();
    }
  }
}


#endif // BOARDS_INC
