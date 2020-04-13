#ifndef ESP_INC
#define ESP_INC

#define CLASS_ESP "ES"
#define NRO_ATTEMPTS 5

WifiNetwork detectWifi(const char *ssid, const char *ssidb) {
  for (int a = 0; a < NRO_ATTEMPTS; a++) {
    log(CLASS_ESP, Debug, "Wifi attempt %d", a);
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
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

#endif // ESP_INC
