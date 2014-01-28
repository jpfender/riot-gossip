/*
 * base64.c
 *
 *  Created on: 18.04.2013
 *      Author: Ron Wenzel
 */

#include "base64.h"

char decode_char(char c);

static char encode_char[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
		'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
		'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',
		'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
		'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

char decode_char(char c) {
	if ('A' <= c && c <= 'Z')
		return c - 0x41;
	if ('a' <= c && c <= 'z')
		return c - 0x61 + 26;
	if ('0' <= c && c <= '9')
		return c - 0x30 + 52;
	if (c == '+')
		return 62;
	if (c == '/')
		return 63;
	return 0;
}

char *base64_encode(char *out, const unsigned char *data, uint16_t len) {
	uint16_t i = 0, j = 0;
	uint16_t o_len = BASE64_ENCODE_LENGTH(len);
		while (i < len) {

		uint32_t bits = ((i < len ? data[i++] : 0) << 16)
				+ ((i < len ? data[i++] : 0) << 8)
				+ (i < len ? data[i++] : 0);

		out[j++] = encode_char[(bits >> 18) & 0x3F];
		out[j++] = encode_char[(bits >> 12) & 0x3F];
		out[j++] = encode_char[(bits >> 6) & 0x3F];
		out[j++] = encode_char[(bits >> 0) & 0x3F];
	}

	j = o_len -  ((3 - (len % 3)) % 3);
	while (j < o_len)
		out[j++] = '=';

	return out;
}

unsigned char *base64_decode(unsigned char *out, const char *data,
		uint16_t len) {

	if (len % 4 != 0)
		return 0;

	int i, j;
	for (i = 0, j = 0; i < len;) {
		uint32_t bits = (decode_char(data[i++]) << 18)
				+ (decode_char(data[i++]) << 12)
				+ (decode_char(data[i++]) << 6)
				+ (decode_char(data[i++]));

		out[j++] = (bits >> 2 * 8) & 0xFF;
		out[j++] = (bits >> 1 * 8) & 0xFF;
		out[j++] = (bits >> 0 * 8) & 0xFF;
	}
	return out;
}
