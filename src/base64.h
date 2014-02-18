/** \file base64.h
 *  \brief Functions to encode and decode base64
 *
 *  Created on: 18.04.2013 \n
 *      Author: Ron Wenzel
 */

#include <stdint.h>

#define BASE64_ENCODE_LENGTH(len) (4 * (((len) + 2) / 3) + 1)
#define BASE64_DECODE_LENGTH(len) ((len) / 4 * 3)

/** Encodes a set of bytes in base64.
 *  No 0-byte at the end of the output string!
 *  \param out output buffer, size at least BASE64_ENCODE_LENGTH(in_length)
 *  \param data data for encoding
 *  \param len length of data set
 */
char *base64_encode(char *out, const unsigned char *data, uint16_t len);


/** Decodes a set of chars encoded in base64.
 *  \param out output buffer, size at least BASE64_DECODE_LENGTH(in_length)
 *  \param data data for decoding
 *  \param len length of data set
 */
unsigned char *base64_decode(unsigned char *out, const char *data, uint16_t len);

