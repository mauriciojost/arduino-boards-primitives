#ifndef BOARDS_INC

#define BOARDS_INC

#include <main4ino/Table.h>
#include <main4ino/ParamStream.h>
#include <main4ino/Buffer.h>
#include <main4ino/HttpCodes.h>
#include <main4ino/HttpMethods.h>

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

bool initializeWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfConnected, int retries);
void stopWifi();
int httpMethod(HttpMethod method, const char *url, const char *body, ParamStream *response, Table *headers, const char *fingerprint);
bool readFile(const char *fname, Buffer *content);
bool writeFile(const char *fname, const char *content);
void updateFirmware(const char *url, const char *currentVersion);
bool lightSleepInterruptable(time_t cycleBegin, time_t periodSecs, int miniPeriodMsec, bool (*interrupt)(), void (*heartbeat)());
bool lightSleepNotInterruptable(time_t cycleBegin, time_t periodSecs, void (*heartbeat)());
void deepSleepNotInterruptable(time_t cycleBegin, time_t periodSecs);
void deepSleepNotInterruptableSecs(time_t cycleBegin, time_t periodSecs);

void updateFirmwareFromMain4ino(const char* session, const char *device, const char *project, const char* platform, const char *targetVersion, const char* currentVersion) {
  Buffer aux(UPDATE_FIRMWARE_URL_MAX_LENGTH);
  aux.fill(FIRMWARE_UPDATE_URL, session, device, project, platform, targetVersion);
  updateFirmware(aux.getBuffer(), currentVersion);
}

#endif // BOARDS_INC
