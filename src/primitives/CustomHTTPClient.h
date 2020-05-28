#ifndef CUSTOM_HTTP_CLIENT_INC
#define CUSTOM_HTTP_CLIENT_INC

#define CLASS_CUSTOM_HTTP "CH"

class CustomHTTPClient : public HTTPClient {

public:

  int sendRequestChunked(const char * type, Stream * stream) {

    if(!stream) {
      clear();
      return returnError(HTTPC_ERROR_NO_STREAM);
    }

    // connect to server
    if(!connect()) {
      clear();
      return returnError(HTTPC_ERROR_CONNECTION_REFUSED);
    }

    addHeader("Transfer-Encoding", String("chunked"));
    addHeader("Connection", String("close"));

    // send Header
    if(!sendHeader(type)) {
      clear();
      return returnError(HTTPC_ERROR_SEND_HEADER_FAILED);
    }

    int buff_size = HTTP_TCP_BUFFER_SIZE;

    // create buffer for read
    uint8_t * payloadChunk = (uint8_t *) malloc(buff_size);
    uint8_t * lenHex = (uint8_t *) malloc(8 + 1);

    if(payloadChunk) {
      // read all data from stream and send it to server
      while(connected() && (stream->available() > -1)) {

        memset(payloadChunk, 0, buff_size);

        // get available data size
        int sizeAvailable = stream->available();

        if(sizeAvailable) { // size cannot be -1: if != 0 then read bytes, otherwise wait until new data arrives
          log(CLASS_CUSTOM_HTTP, Debug, "Avlble: '%d'", sizeAvailable);

          int readBytes = sizeAvailable;

          // not read more the buffer can handle
          if(readBytes > buff_size) {
            readBytes = buff_size;
          }

          // read data
          int payloadBytesRead = stream->readBytes(payloadChunk, readBytes);

          // write to Stream the length
          int l = sprintf((char*)lenHex, "%X", payloadBytesRead);
          // FIXME l could be negative
          _client->write((const uint8_t *) lenHex, l);
          _client->write((const uint8_t *) "\r\n", 2);

          // write to Stream the chunked body
          int payloadBytesWrite = _client->write((const uint8_t *) payloadChunk, payloadBytesRead);
          _client->write((const uint8_t *) "\r\n", 2);

          log(CLASS_CUSTOM_HTTP, Fine, "Write: lhex='%s' cont='%s'", lenHex, payloadChunk);

          // are all Bytes a writen to stream ?
          if(payloadBytesWrite != payloadBytesRead) {
            log(CLASS_CUSTOM_HTTP, Warn, "Short write %d/%d", payloadBytesRead, payloadBytesWrite);
          }

          // check for write error
          if(_client->getWriteError()) {
            log(CLASS_CUSTOM_HTTP, Warn, "Write error %d", _client->getWriteError());
            free(payloadChunk);
            free(lenHex);
            clear();
            return returnError(HTTPC_ERROR_SEND_PAYLOAD_FAILED);
          }

          delay(0);
        } else {
          delay(1);
        }
      }

      _client->write((const uint8_t *) "0\r\n\r\n", 5); // close

      free(payloadChunk);
      free(lenHex);

    } else {
      log(CLASS_CUSTOM_HTTP, Warn, "Not enough RAM / %d", HTTP_TCP_BUFFER_SIZE);
      clear();
      return returnError(HTTPC_ERROR_TOO_LESS_RAM);
    }

    // handle Server Response (Header)
    clear();
    return returnError(handleHeaderResponse());
  }

};


#endif // CUSTOM_HTTP_CLIENT_INC
