#ifndef Stream_h
#define Stream_h

#ifdef X86_64

typedef unsigned char uint8_t;
typedef unsigned long size_t;

class Stream {
private:
  int idx;
  char buff[16 * 1024];

public:

  Stream(const char *c);

  int read();

  void setTimeout(unsigned long timeout);

  void fillUntil(const char *str, const char *until);
  const char* content();

  size_t readBytes(char *buffer, size_t length);
  size_t readBytes(uint8_t *buffer, size_t length) {
    return readBytes((char *) buffer, length);
  }
  size_t readBytesUntil(char terminator, char *buffer, size_t length); // as readBytes with terminator character
  size_t readBytesUntil(char terminator, uint8_t *buffer, size_t length) {
    return readBytesUntil(terminator, (char *) buffer, length);
  }

};

#endif // X86_64

#endif
