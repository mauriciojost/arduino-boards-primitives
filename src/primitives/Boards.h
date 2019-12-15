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

#ifndef FIRMWARE_UPDATE_URL
// convention for firmware file name: firmware-<version>.<platform>.bin
// to replace: base + project + version + platform
#define FIRMWARE_UPDATE_URL MAIN4INOSERVER_API_HOST_BASE "/firmwares/%s/%s/content?version=%s"
#endif // FIRMWARE_UPDATE_URL


WifiNetwork detectWifi(const char *ssid, const char *ssidb);
bool initializeWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfConnected, int retries);
void stopWifi();
int httpMethod(HttpMethod method, const char *url, const char *body, ParamStream *response, Table *headers);
bool readFile(const char *fname, Buffer *content);
bool writeFile(const char *fname, const char *content);
void updateFirmware(const char *url, const char *currentVersion);
bool lightSleepInterruptable(time_t cycleBegin, time_t periodSecs, int miniPeriodMsec, bool (*interrupt)(), void (*heartbeat)());
bool lightSleepNotInterruptable(time_t cycleBegin, time_t periodSecs, void (*heartbeat)());
void deepSleepNotInterruptable(time_t cycleBegin, time_t periodSecs);
void deepSleepNotInterruptableSecs(time_t cycleBegin, time_t periodSecs);

void updateFirmwareFromMain4ino(const char *project, const char* platform, const char *targetVersion, const char* currentVersion) {
  Buffer aux(128);
  aux.fill(FIRMWARE_UPDATE_URL, project, platform, targetVersion);
  updateFirmware(aux.getBuffer(), currentVersion);
}

#endif // BOARDS_INC
