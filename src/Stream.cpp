#ifdef X86_64

#include <Stream.h>

#include <stdio.h>
#include <string.h>

Stream::Stream(const char* str) {
  strcpy(buff, str);
  idx = 0;
}

int Stream::read() {
  if (idx < strlen(buff)) {
    idx++;
    return buff[idx - 1];
  } else {
    return -1;
  }
}

const char* Stream::content() {
  return buff;
}

void Stream::fillUntil(const char *str, const char *until) {
  strcpy(buff, str);
  const char *oc = strstr(buff, until);
  if (oc != NULL) {
    buff[oc - buff] = 0;
  }
}


void Stream::setTimeout(unsigned long timeout) { }

// read characters from stream into buffer
// terminates if length characters have been read, or timeout (see setTimeout)
// returns the number of characters placed in the buffer
// the buffer is NOT null terminated.
//
size_t Stream::readBytes(char *buffer, size_t length) {
  size_t count = 0;
  while(count < length) {
    int c = read();
    if(c < 0)
      break;
    *buffer++ = (char) c;
    count++;
  }
  return count;
}

// as readBytes with terminator character
// terminates if length characters have been read, timeout, or if the terminator character  detected
// returns the number of characters placed in the buffer (0 means no valid data found)

size_t Stream::readBytesUntil(char terminator, char *buffer, size_t length) {
  if(length < 1)
    return 0;
  size_t index = 0;
  while(index < length) {
    int c = read();
    if(c < 0 || c == terminator)
      break;
    *buffer++ = (char) c;
    index++;
  }
  return index; // return number of characters, not including null terminator
}

#endif // X86_64

