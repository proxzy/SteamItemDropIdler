#include <stdio.h>
#include <windows.h>
#include <time.h>
#include "sha1.h"
#include "base64.h"

const char chars[] = {
	'2', '3', '4', '5', '6', '7', '8', '9',
	'B', 'C', 'D', 'F', 'G', 'H', 'J', 'K', 'M',
	'N', 'P', 'Q', 'R', 'T', 'V', 'W', 'X', 'Y'
};

uint32_t swap_uint32(uint32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF); 
	return (val << 16) | (val >> 16);
}

int getSharedSecret(const char *accountName, uint8_t *secret, uint32_t bufferSize)
{
	uint8_t shared_secret[30] = {0};
	char secretFileName[70];
	FILE *pFile;
	int ret;

	strcpy_s(secretFileName, sizeof(secretFileName), accountName);
	strcat_s(secretFileName, sizeof(secretFileName), ".secret");

	pFile = fopen(secretFileName, "r");

	if (!pFile)
	{
		return 1;
	}

	ret = fscanf(pFile, "%28s", shared_secret);
	fclose(pFile);

	if (!ret || shared_secret[27] != '=')
	{
		return 2;
	}

	ret = Base64decode(/*out*/(char *)shared_secret, /*in*/(char *)shared_secret);
	if (ret != HASH_LENGTH)
	{
		return 3;
	}

	if (bufferSize < HASH_LENGTH)
	{
	  return -1;
	}

	memcpy(secret, shared_secret, HASH_LENGTH);

	return 0;
}

int get2FACode(const uint8_t *secret, char *code, uint32_t bufferSize)
{
	sha1nfo s;
	uint8_t *hmac;
	uint32_t timeBuffer[2];
	uint32_t codePoint;
	int i;

	if (bufferSize < 6)
	{
		return -1;
	}

	timeBuffer[0] = 0; // This will stop working in 2038!
	timeBuffer[1] = swap_uint32(time(NULL) / 30);

	sha1_initHmac(&s, secret, 20);
	sha1_write(&s, (char *)timeBuffer, 8);
	hmac = sha1_resultHmac(&s);

	codePoint = swap_uint32(*(uint32_t *)(hmac + (hmac[19] & 0x0F))) & 0x7FFFFFFF;

	for (i = 0; i < 5; ++i)
	{
		code[i] = chars[codePoint % sizeof(chars)];
		codePoint /= sizeof(chars);
	}
	code[5] = '\0';

	return 0;
}
