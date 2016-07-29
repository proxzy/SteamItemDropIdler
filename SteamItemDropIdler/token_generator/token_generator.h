#ifndef _TOKEN_GENERATOR_H_
#define _TOKEN_GENERATOR_H_

#include <stdint.h>

int getSharedSecret(const char *accountName, uint8_t *secret, uint32_t bufferSize);
int get2FACode(const uint8_t *secret, char *code, uint32_t bufferSize);

#endif //_TOKEN_GENERATOR_H_
