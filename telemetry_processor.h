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
    u32 id;

    // Callback functions that used to get data
    getter func;

    // Variable type which return callback functions
    u8 type;
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
#define LONG    2
#define ARRAY   3
#define DOUBLE  4

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
 * Transmitting the n-byte data using UART
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
 * Transmitting the number having the data type "double"
 * @param  data
 */
void Telemetry_transmitDouble(double data);

/**
 * Transmitting an array of n-bytes digits using UART interface
 * @param arr - an array of n-bytes digits
 * @param len - length of array
 */
void Telemetry_arrayTransmit(s32* arr, u8 len);

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

/**
 * Transmitting Telemetry data
 * @param type - data type identifier
 * @param data - n-bytes values for transmitting
 */
void Telemetry_dataTransmit(u8 type, s32* data);

/**
 * Listening to the Rx wire and transmitting data on request
 * @param items - telemetry items structure
 * @param count - number of telemetry items
 */
void Telemetry_streamData(telemetry_items* items, u8 count);

#endif
