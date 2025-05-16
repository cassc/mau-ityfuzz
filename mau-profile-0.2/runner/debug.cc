#include <stdint.h>
#include <assert.h>
#include <iostream>

#include "debug.h"

uint32_t bytesFromHex_bigEndian(uint8_t bytes[], const uint8_t *rawdata,
                                const size_t size) {
  const char *hexdata = (char *)rawdata;
  uint32_t offset = 0;
  if (hexdata[0] == '0' && (hexdata[1] == 'x' || hexdata[1] == 'X')) offset = 2;

  for (uint32_t i = 0; i < size; i++) {
    std::sscanf(hexdata + offset + i * 2, "%02hhx", &bytes[size - 1 - i]);
  }
  return (size - offset) / 2;
}

uint32_t bytesFromHex(uint8_t bytes[], const std::string& rawdata) {
  assert(rawdata.length() % 2 == 0);
  uint32_t bytesLen = (uint32_t)rawdata.length() / 2;
  uint32_t offset = 0;
  if (rawdata[0] == '0' && (rawdata[1] == 'x' || rawdata[1] == 'X'))
    offset = 2;
  // printf("offset = %d\n", offset);

  std::string strByte;
  for (uint32_t i = 0; i < bytesLen; i++) {
    strByte = rawdata.substr(offset + i * 2, 2);
    std::sscanf(strByte.c_str(), "%02hhx", &bytes[i]);
    // sscanf_s(strByte.c_str(), "%x", &bytes[i]);
  }
  return bytesLen - (offset == 0 ? 0 : 1);
}

void printbytes(uint8_t bytes[], uint32_t _size) {
  for (uint32_t i = 0; i < _size; ++i) {
    // if (i == 8 || i == 12 || i == 16 || (((i - 16) % 32) == 0) ) printf(" ");
    printf("%02hhx", bytes[i]);
  }
  printf("\n");
}

void printbytes(char msg[], uint8_t bytes[], uint32_t _size) {
  char hexmap[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  
  char *buf = (char*)malloc(_size * 2 + 1);

  for (uint32_t i = 0; i < _size; ++i) {
    uint8_t byte = bytes[i];
    buf[2*i + 1]  = hexmap[(byte & 0xf)];
    byte >>= 4;
    buf[2*i]      = hexmap[byte & 0xf];
  }
  buf[_size*2] = '\x00';
  printf("%s hex bytes: %s\n", msg, buf);
  free(buf);
}

std::string BytesToHex(const uint8_t *Data, const size_t Len) {  
  char Out[1001];
  assert(Len % 2 == 0 || Len < 1000);
  for (auto i = 0; i < Len; i++) {
    std::sprintf((char*)&Out[2 * i], "%02hhx", (char)Data[i]);
  }
  return std::string(Out);
}