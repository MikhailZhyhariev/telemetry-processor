/* Name: telemetry_processor.h
 * Author: Zhyhariev Mikhail
 * License: MIT
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H


/**
 * Choose your platform ORANGE, ATMEGA, etc.
 * (comment extra line)
 */
#define PLATFORM ORANGE
// Сonnection of libraries depending on the platform
#if PLATFORM == ORANGE
    #include <stdio.h>
    #include <wiringPi.h>
    #include <wiringSerial.h>
#elif PLATFORM == ATMEGA
    #include "uart/uart.h"
#endif

#include <stdint.h>

// Pointer to a callback function
typedef void* (*getter)(void);

// Signed custom variables types
typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;

// Unsigned custom variables types
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;

/**
 * The name of functions for transmitting and receiving data.
 * Here you can use any data transmission/receiving functions regardless of the data transmission protocol.
 * "transmitData" function should have type "u8" argument value - data
 * "receiveData" function should have type "u8" returning value.
 * Uncomment the desired function.
 */
// For use on ATMEGA Controllers
// #define Telemetry_transmitData(x)  (USART_Transmit(x))
// #define Telemetry_receiveData()    (USART_Receive())

// For use on orangePi (rapberryPi)
#define BAUD        9600
#define INTERFACE   "/dev/ttyS0"
const int fd;
#define Telemetry_init()                (fd = serialOpen(BAUD, INTERFACE))
#define Telemetry_transmitData(data)    (serialPutchar(fd, data))
#define Telemetry_receiveData()         (serialGetchar(fd))

/**
 * The name of delay function.
 * "delay"      - for use on orangePi (rapberryPi)
 * "_delay_ms"  - for use on ATMEGA Controllers
 * Uncomment the desired function.
 */
// #define Telemetry_delay(ms)     (_delay_ms(ms))
#define Telemetry_delay(ms)     (delay(ms))

// Telemetry items structure
typedef struct {
    // Identifier of data
    s32 id;

    // Callback functions that used to get data
    getter func;

    // Variable type which return callback functions
    u8 type;
} telemetry_item;

// Types of a data
#define CHAR    0
#define INT     1
#define LONG    2
#define ARRAY   3
#define DOUBLE  4

#define START   33000

// Sign identifiers
#define MINUS   33001
#define PLUS    33002


/**
 * FUNCTIONS
 */

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
 * @param type  - data type identifier
 * @param data  - n-bytes values for transmitting
 * @param delay
 */
 void Telemetry_dataTransmit(u8 type, s32* data);

/**
 * Listening to the Rx wire and transmitting data on request
 * @param items - telemetry items structure
 * @param count - number of telemetry items
 * @param del   - delay
 */
void Telemetry_streamData(telemetry_item* items, u8 count, u8 del);

/**
 * Getting telemetry data after transmitting identifier
 * @param id    - data identifier
 * @param items - telemetry items structure
 * @param count - number of telemetry items
 */
void* Telemetry_getData(s32 id, telemetry_item* items, u8 count);

#endif
