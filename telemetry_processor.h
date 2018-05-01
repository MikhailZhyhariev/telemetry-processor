/* Name: telemetry_processor.h
 * Author: Zhyhariev Mikhail
 * License: MIT
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H

// Platforms settings
#define ATMEGA      0
#define ORANGE      1
// Choose your platform ORANGE, ATMEGA, etc.
#define PLATFORM    ATMEGA
// Сonnection of libraries depending on the platform
#if PLATFORM == ORANGE
    #include <stdio.h>
    #include <stdlib.h>
    #include <wiringPi.h>
    #include <wiringSerial.h>
#elif PLATFORM == ATMEGA
    #include "uart/uart.h"
#endif

#include <stdint.h>

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
#if PLATFORM == ATMEGA
    #define Telemetry_transmitData(x)       (USART_Transmit(x))
    #define Telemetry_receiveData()         (USART_Receive())
// For use on orangePi (rapberryPi)
#elif PLATFORM == ORANGE
    // UART settings
    #define BAUD                            9600
    #define INTERFACE                       "/dev/ttyS0"
    // File descriptor
    static int fd;
    // Initialisation serial interface for orangePi
    #define Telemetry_init()                (fd = serialOpen(INTERFACE, BAUD))
    #define Telemetry_transmitData(data)    (serialPutchar(fd, data))
    #define Telemetry_receiveData()         (serialGetchar(fd))
#endif

// Pointer to a callback function
typedef void* (*getter)(void);

// Telemetry items structure
typedef struct {
    // Identifier of data
    u8 id;

    // Callback functions that used to get data
    getter func;

    // Variable type which return callback functions
    u8 type;
} telemetry_item;

// Types of a data
#define ONE_BYTE      1
#define TWO_BYTE      2
#define FOUR_BYTE     4
#define ARRAY         5
#define FLOAT         6

#define START         33000

// Sign identifiers
#define MINUS         33001
#define PLUS          33002

/**
 * FUNCTIONS
 */

 /**
  * Transmitting RAW n-bytes data
  * @param data - n-bytes data
  */
void _Telemetry_transmitRawData(u32 data, u8 bytes);

/**
 * Receiving RAW n-bytes data
 * @param  bytes - number of bytes the data
 * @return       RAW n-bytes data
 */
u32 _Telemetry_receiveRawData(u8 bytes);

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
s32 Telemetry_nthBytesReceive(u8 bytes);

/**
 * Transmitting the number having the data type "float"
 * @param  data
 */
void Telemetry_transmitFloat(float* data);

/**
 * Receiving the number having the data type "float"
 * @return  number that having type "float"
 */
float* Telemetry_receiveFloat(void);

/**
 * Transmitting an array of n-bytes digits using UART interface
 * @param arr - an array of n-bytes digits
 * @param len - length of array
 */
void Telemetry_arrayTransmit(s32* arr);

/**
 * Transmitting an array of n-bytes digits using UART interface
 * @param  len - length of array
 * @return     an array of n-bytes digits
 */
s32* Telemetry_receiveArray(void);

/**
 * Create telemetry items
 * @param  count     - number of telemetry items
 * @param  ids       - identifiers of telemetry items
 * @param  functions - callbacks of telemetry items
 * @param  types     - variables types which return by callback functions
 * @return           telemetry items structure
 */
telemetry_item* Telemetry_getItems(u8 count, u8* ids, getter* functions, u8* types);


/**
 * Transmitting Telemetry data
 * @param type  - data type identifier
 * @param data  - n-bytes values for transmitting
 */
void Telemetry_dataTransmit(u8 type, void* data);

/**
 * Listening to the Rx wire and transmitting data on request
 * @param items - telemetry items structure
 * @param count - number of telemetry items
 */
void Telemetry_streamData(telemetry_item* items, u8 count);

/**
 * Getting telemetry data after transmitting identifier
 * @param id    - data identifier
 */
void* Telemetry_getData(u8 id);

#endif
