#ifndef LUA_RESTY_BASE_ENCODING_BASE32_H
#define LUA_RESTY_BASE_ENCODING_BASE32_H

#include <stdint.h>

size_t b32_encode(char *dest, const char *src, size_t len, uint32_t no_padding, uint32_t hex);
size_t b32_decode(char *dest, const char *src, size_t len, uint32_t hex);

#endif // LUA_RESTY_BASE_ENCODING_BASE32_H