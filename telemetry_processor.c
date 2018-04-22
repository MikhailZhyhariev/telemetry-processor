/* Name: telemetry_processor.c
 * Author: Zhyhariev Mikhail
 * License: MIT
 */

#include <stdlib.h>
#include "uart/uart.h"

/**
 * Checks sign of the data. If data is negative, inverts it and transmitting sign identifier
 * @param data
 * @return      positive data
 */
s32 Telemetry_checkSign(s32 data) {
    if (data < 0) {
        USART_Transmit((MINUS >> 8) & 0xFF);
        USART_Transmit(MINUS & 0xFF);
        data = -(data);
    } else {
        USART_Transmit((PLUS >> 8) & 0xFF);
        USART_Transmit(PLUS & 0xFF);
    }
    return data;
}

/**
 * Transmitting the n-byte data using UART
 * @param data
 * @param bytes - number of bytes of the register
 */
void Telemetry_nthBytesTransmit(s32 data, u8 bytes) {
    // Check sign of the data
    data = Telemetry_checkSign(data);

    // Transmitting number of bytes
    USART_Transmit(bytes);

    // Transmitting the data
    for (u8 i = 0; i < bytes; i++) {
        USART_Transmit((data >> (8 * (bytes - i - 1))) & 0xFF);
    }
}

/**
 * Receiving n-bytes using UART interface
 * @return  n-bytes data
 */
s32 Telemetry_nthBytesReceive(void) {
    // Receiving sign of the data
    u16 sign = USART_Receive();

    // Receiving number of bytes
    u16 bytes = USART_Receive();

    // Receiving the data
    s32 data = 0;
    for (u8 i = 0; i < bytes; i++) {
        data += USART_Receive() << (8 * (bytes - i - 1));
    }
    return data;
}

/**
 * Transmitting the number having the data type "double"
 * @param  data
 */
void Telemetry_transmitDouble(double data) {
    u8* ptr = (u8 *)&data;
    for (u8 i = 0; i < sizeof(double); i++) {
        USART_Transmit(*(ptr++));
    }
}

/**
 * Transmitting an array of n-bytes digits using UART interface
 * @param arr - an array of n-bytes digits
 * @param len - length of array
 */
void Telemetry_transmitArray(s32* arr, u8 len) {
    for (u8 i = 0; i < len; i++) {
        // Create the temporary variable, so as not to change the values of the array
        s32 data = arr[i];

        // Check sign of the data
        data = Telemetry_checkSign(data);

        // Transmitting data
        Telemetry_nthBytesTransmit(data, sizeof(data));
    }
}

/**
 * Transmitting an array of n-bytes digits using UART interface
 * @param  len - length of array
 * @return     an array of n-bytes digits
 */
u32* Telemetry_receiveArray(u8 len) {
    u32 *arr = (u32 *)malloc(len * sizeof(u32));

    for (u8 i = 0; i < len; i++) {
        u32 sign = Telemetry_nthBytesReceive(2);

        arr[i] = Telemetry_nthBytesReceive(2);
        if (sign == MINUS) arr[i] *= -1;
    }
    return arr;
}

/**
 * Create telemetry items
 * @param  count     - number of telemetry items
 * @param  ids       - identifiers of telemetry items
 * @param  functions - callback functions of telemetry items
 * @param  types     - variables types which return callback functions
 * @return           telemetry items structure
 */
telemetry_item* getItems(unsigned char count, int* ids, getter* functions, unsigned char* types) {
    telemetry_item* items = (telemetry_item *)malloc(sizeof(telemetry_item) * count);

    for (unsigned char i = 0; i < count; i++) {
        items[i].id = ids[i];
        items[i].func = functions[i];
        items[i].type = types[i];
    }
    return items;
}

/**
 * Transmitting Telemetry data
 * @param type - data type identifier
 * @param data - n-bytes values for transmitting
 */
void Telemetry_dataTransmit(u8 type, s32* data) {
    // Transmitting "start" identifier
    Telemetry_nthBytesTransmit(START, 2);

    // Transmitting data using the function according to data type
    switch (type) {
        case CHAR:
            Telemetry_nthBytesTransmit(data, 1);
            break;

        case INT:
            Telemetry_nthBytesTransmit(data, 2);
            break;

        case LONG:
            Telemetry_nthBytesTransmit(data, 4);
            break;

        case DOUBLE:
            Telemetry_transmitDouble(data);
            break;

        case ARRAY:
            /*
            Transmitting data array. Counting a array length using "sizeof" function.
            Example:
            s32 data[3] = {1, 2, 3}
            data array item have "s32" type. sizeof(s32) = 4 (byte)
            sizeof(data) = sizeof(s32 data[length]) = sizeof(s32 * length) = 4 * length = 12 (byte);
            length = 12 / 4 = 3
             */
            Telemetry_transmitArray(data, sizeof(data) / sizeof(s32));
            break;
    }
}

/**
 * Listening the Rx wire and transmitting data on request
 * @param items - telemetry items structure
 * @param count - number of telemetry items
 */
void Telemetry_streamData(telemetry_items* items, u8 count) {
    // Receiving data identifier
    s32 id = Telemetry_nthBytesReceive();

    for (u8 i = 0; i < count; i++) {
        // Find telemetry item with right identifier
        if (items[i].id == id) {
            // Getting data to transmit
            s32* data = items[i].func();

            // Transmitting the data
            Telemetry_dataTransmit(items[i].type, data);
        }
    }
}

/**
 * Transmitting data according to received id
 * @return data according to received id
 */
int* Telemetry_dataReceive() {
    // Creating a data array
    int* data = NULL;

    // Receiving two-bytes id
    int id = Telemetry_twoBytesReceive();
    switch (id) {
        case ACCEL:
            // Getting accelerometer data
            data = Telemetry_getAccel();
            // Transmitting data
            Telemetry_dataTransmit(id, data);
            break;

        case GYRO:
            // Getting gyroscope data
            data = Telemetry_getGyro();
            // Transmitting data
            Telemetry_dataTransmit(id, data);
            break;

        case TEMP:
            // Getting temperature data
            data = (int *)Telemetry_getTemp();
            // Transmitting data
            Telemetry_dataTransmit(id, data);
            break;
    }

    return data;
}
