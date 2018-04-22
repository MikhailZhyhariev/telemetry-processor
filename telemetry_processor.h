/* Name: telemetry_processor.h
 * Author: Zhyhariev Mikhail
 * License: MIT
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdlib.h>

// Pointer to a callback function
typedef double* (*getter)(void);

// Telemetry items structure
typedef struct {
    // Identifier of data
    int id;

    // Callback functions that used to get data
    getter func;

    // Variable type which return callback functions
    unsigned char type;
} telemetry_item;

// Signed custom variables types
typedef short s16;
typedef int32_t s32;

// Unsigned custom variables types
typedef unsigned char u8;
typedef unsigned short u16;
typedef uint32_t u32;

// Types of a data
#define CHAR    0
#define INT     1
#define ARRAY   2
#define DOUBLE  3

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
 * Transmitting a n-byte data using UART
 * @param data
 * @param bytes - number of bytes of the register
 */
void Telemetry_nthBytesTransmit(s32 data, u8 bytes);

/**
 * Receiving n-bytes using UART interface
 * @return  n-bytes data
 */
s32 Telemetry_nthBytesReceive(void);

/**
 * Transmitting an array of n-bytes digits using UART interface
 * @param arr - an array of n-bytes digits
 * @param len - length of array
 */
void Telemetry_arrayTransmit(u32* arr, u8 len);

/**
 * Transmitting an array of n-bytes digits using UART interface
 * @param  len - length of array
 * @return     an array of n-bytes digits
 */
u32* Telemetry_receiveArray(u8 len);

/**
 * Create telemetry items
 * @param  count     - number of telemetry items
 * @param  ids       - identifiers of telemetry items
 * @param  functions - callbacks of telemetry items
 * @param  types     - variables types which return by callback functions
 * @return           telemetry items structure
 */
telemetry_item* getItems(u8 count, s32* ids, getter* functions, u8* types);

#endif
