/* Name: telemetry_processor.c
 * Author: Zhyhariev Mikhail
 * License: MIT
 */

#include <stdlib.h>
#include "telemetry_processor.h"

/**
 * Transmitting RAW n-bytes data
 * @param data - n-bytes data
 */
void _Telemetry_transmitRawData(u32 data, u8 bytes) {
    // Transmitting raw data
    for (u8 i = 0; i < bytes; i++) {
        Telemetry_transmitData((data >> (8 * (bytes - i - 1))) & 0xFF);
    }
}

/**
 * Receiving RAW n-bytes data
 * @param  bytes - number of bytes the data
 * @return       RAW n-bytes data
 */
u32 _Telemetry_receiveRawData(u8 bytes) {
    // Receiving the data
    u32 data = 0;
    for (u8 i = 0; i < bytes; i++) {
        data += Telemetry_receiveData() << (8 * (bytes - i - 1));
    }
    return data;
}

/**
 * Checks sign of the data. If data is negative, inverts it and transmitting sign identifier
 * @param data
 * @return      positive data
 */
s32 Telemetry_checkSign(s32 data) {
    if (data < 0) {
        _Telemetry_transmitRawData(MINUS, TWO_BYTE);
        data = -(data);
    } else {
        _Telemetry_transmitRawData(PLUS, TWO_BYTE);
    }
    return data;
}

/**
 * Transmitting the n-byte data using UART
 * @param data
 * @param bytes - number of bytes of the register
 */
void Telemetry_nthBytesTransmit(s32 data) {
    // Check sign of the data
    data = Telemetry_checkSign(data);

    // Transmitting the data
    _Telemetry_transmitRawData(data, FOUR_BYTE);
}

/**
 * Receiving n-bytes using UART interface
 * @return  n-bytes data
 */
s32 Telemetry_nthBytesReceive(u8 bytes) {
    // Receiving sign of the data
    u16 sign = _Telemetry_receiveRawData(TWO_BYTE);

    // Receiving the data
    s32 data = _Telemetry_receiveRawData(bytes);

    // Invert the data if a sign is "minus"
    if (sign == MINUS) data = -(data);
    return data;
}

/**
 * Transmitting the number having the data type "float"
 * @param  data
 */
void Telemetry_transmitFloat(float data) {
    u8* ptr = (u8 *)&data;
    for (u8 i = 0; i < sizeof(float); i++) {
        Telemetry_transmitData(*(ptr++));
    }
}

/**
 * Receiving the number having the data type "float"
 * @return  number that having type "float"
 */
float Telemetry_receiveFloat(void) {
    return (float)_Telemetry_receiveRawData(sizeof(float));
}

/**
 * Transmitting an array of n-bytes digits using UART interface
 * @param arr - an array of n-bytes digits
 * @param len - length of array
 */
void Telemetry_transmitArray(s32* arr) {
    /*
    * Counting a array length using "sizeof" function.
    *
    * Example:
    * s32 arr[3] = {1, 2, 3}
    * array item have "s32" type. sizeof(s32) = 4 (byte)
    *
    * sizeof(arr) = sizeof(s32 arr[length]) =
    * sizeof(s32 * length) = 4 * length = 12 (byte);
    *
    * length = 12 / 4 = 3
    */
    u8 len = sizeof(arr) / sizeof(s32);
    Telemetry_transmitData(len);

    for (u8 i = 0; i < len; i++) {
        // Create the temporary variable, so as not to change the values of the array
        s32 data = arr[i];

        // Check sign of the data
        data = Telemetry_checkSign(data);

        // Transmitting data
        Telemetry_nthBytesTransmit(data);
    }
}

/**
 * Transmitting an array of n-bytes digits using UART interface
 * @return     an array of n-bytes digits
 */
s32* Telemetry_receiveArray(void) {
    // Receiving an array length
    u8 len = Telemetry_receiveData();

    // Allocating memory to the data
    s32 *arr = (s32 *)malloc(len * sizeof(s32));

    for (u8 i = 0; i < len; i++) {
        // Receiving an array item
        arr[i] = Telemetry_nthBytesReceive(sizeof(s32));
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
telemetry_item* Telemetry_getItems(u8 count, u8* ids, getter* functions, u8* types) {
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
void Telemetry_dataTransmit(u8 type, void* data) {
    // Transmitting "start" identifier
    _Telemetry_transmitRawData(START, TWO_BYTE);

    // Transmitting data type identifier
    Telemetry_transmitData(type);

    // Check data type and use a special function if a type is "float" or "array"
    switch (type) {
        case FLOAT:
            // Transmitting data that having "float" type and return from the function
            float* d = (float *)data;
            Telemetry_transmitFloat(*d);
            return;

        case ARRAY:
            // Transmitting data that having "array" type and return from the function
            s32* arr = (s32 *)data;
            Telemetry_transmitArray(arr);
            return;
    }

    // Transmitting data
    Telemetry_nthBytesTransmit(*data);
}

/**
 * Listening the Rx wire and transmitting data on request
 * @param items - telemetry items structure
 * @param count - number of telemetry items
 * @param del   - delay
 */
void Telemetry_streamData(telemetry_item* items, u8 count) {
    // // Receiving data identifier
    u8 id = Telemetry_receiveData();

    for (u8 i = 0; i < count; i++) {
        // Find telemetry item with right identifier
        if (items[i].id == id) {
            // Getting data to transmit
            void* ptr = items[i].func();

            // Transmitting the data
            Telemetry_dataTransmit(items[i].type, &data);
            break;
        }
    }
}

/**
 * Getting telemetry data after transmitting identifier
 * @param id    - data identifier
 */
s32 Telemetry_getData(u8 id) {
    // Transmitting data identifier
    Telemetry_transmitData(id);

    // If the identifier "start" is not received - do nothing
    if (_Telemetry_receiveRawData(TWO_BYTE) != START) return NULL;

    // Receiving data type identifier
    u8 type = Telemetry_receiveData();

    switch (type) {
        case ARRAY:
            s32* data = Telemetry_receiveArray();
            break;

        case FLOAT:
            float data = Telemetry_receiveFloat();
            break;

        default:
            s32 data = Telemetry_nthBytesReceive(type);
    }

    // Receiving data
    return data;
}
