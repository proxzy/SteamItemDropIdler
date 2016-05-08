#ifndef _TOKEN_GENERATOR_H_
#define _TOKEN_GENERATOR_H_

#include <stdint.h>

int getSharedSecret(const char *accountName, uint8_t *secret);
void get2FACode(const uint8_t *secret, char *code);

#endif //_TOKEN_GENERATOR_H_
