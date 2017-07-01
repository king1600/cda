#pragma once
#include "events.hh"

namespace io {

  /**
   * Deoce a base64 string to a unsigned char array
   * @param {const char*} src the char array to decode
   * @param {size_t} len the size of the char array
   * @return {unsigned char*} the encoded base64 string
   * @see https://github.com/littlstar/b64.c
   */
  unsigned char* b64_decode(const char *src, size_t len);

  /**
   * Encode a char array to base64
   * @param {const unsigned char*} src the char array to encode
   * @param {size_t} len the size of the char array
   * @return {char*} the encoded base64 string
   * @see https://github.com/littlstar/b64.c
   */
  char *b64_encode(const unsigned char *src, size_t len);

}