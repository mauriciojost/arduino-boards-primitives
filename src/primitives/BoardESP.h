#ifndef ESP_INC
#define ESP_INC

#define CLASS_ESP "ES"
#define NRO_ATTEMPTS 5
#define WAIT_BEFORE_HTTP_MS 100

#ifndef URL_PRINT_MAX_LENGTH
#define URL_PRINT_MAX_LENGTH 20
#endif // URL_PRINT_MAX_LENGTH

CustomHTTPClient httpClient;
std::function<void ()> httpClientEnd = []() { httpClient.end();};

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


HttpResponse httpMethod(HttpMethod method, const char *url, Stream *body, Table *headers, const char *fingerprint) {
  int errorCode;
  if (fingerprint == NULL) {
    httpClient.begin(url);
  } else {
    httpClient.begin(url, fingerprint);
  }
  log(CLASS_ESPX, Debug, "> %s:..%s", HTTP_METHOD_STR(method), tailStr(url, URL_PRINT_MAX_LENGTH));
  for (KV kv = headers->next(KV()); kv.isValid(); kv = headers->next(kv)) {
    httpClient.addHeader(kv.getKey(), kv.getValue());
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

  log(CLASS_ESPX, Debug, "< %d", errorCode);
  delay(WAIT_BEFORE_HTTP_MS);
  return HttpResponse(errorCode, httpClient.getStreamPtr(), httpClientEnd);
}



#endif // ESP_INC
