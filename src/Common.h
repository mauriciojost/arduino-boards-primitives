#ifndef COMMON_INC

#define COMMON_INC

#include <main4ino/Table.h>
#include <main4ino/ParamStream.h>
#include <main4ino/Buffer.h>
#include <main4ino/HttpCodes.h>

enum WifiNetwork { WifiNoNetwork = 0, WifiMainNetwork, WifiBackupNetwork };

WifiNetwork detectWifi(const char *ssid, const char *ssidb);
bool initializeWifi(const char *ssid, const char *pass, const char *ssidb, const char *passb, bool skipIfConnected, int retries);
void stopWifi();
int httpGet(const char *url, ParamStream *response, Table *headers);
int httpPost(const char *url, const char *body, ParamStream *response, Table *headers);
bool readFile(const char *fname, Buffer *content);
bool writeFile(const char *fname, const char *content);
void updateFirmwareVersion(const char *url, const char *projVersion);
bool lightSleepInterruptable(time_t cycleBegin, time_t periodSecs, int miniPeriodMsec, bool (*interrupt)(), void (*heartbeat)());
bool lightSleepNotInterruptable(time_t cycleBegin, time_t periodSecs, void (*heartbeat)());
void deepSleepNotInterruptable(time_t cycleBegin, time_t periodSecs);
void deepSleepNotInterruptableSecs(time_t cycleBegin, time_t periodSecs);

#endif // COMMON_INC
