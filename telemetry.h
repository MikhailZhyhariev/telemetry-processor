#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdlib.h>

// Signed custom variables types
typedef short s16;
typedef int32_t s32;

// Unsigned custom variables types
typedef unsigned char u8;
typedef unsigned short u16;
typedef uint32_t u32;

// Sign identifiers
#define MINUS 33001
#define PLUS  33002

/**
 * Checks sign of the data. If data is negative, inverts it and transmitting sign identifier
 * @param data
 * @return      positive data
 */
s32 Telemetry_checkSign(s32 data);

/**
 * Transmitting a n-byte data
 * @param data
 * @param bytes - number of bytes of the register
 */
void Telemetry_nthBytesTransmit(s32 data, u8 bytes);

#endif
