/* Name: telemetry_processor.c
 * Author: Zhyhariev Mikhail
 * License: MIT
 */

#include <stdlib.h>
#include <math.h>
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
void Telemetry_nthBytesTransmit(s32 data, u8 bytes) {
    // Check sign of the data
    u32 d = Telemetry_checkSign(data);

    // Transmitting the data
    _Telemetry_transmitRawData(d, bytes);
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
void Telemetry_transmitFloat(float* data) {
    u32* ptr = (u32 *)data;
    _Telemetry_transmitRawData(*ptr, sizeof(float));
}

/**
 * Receiving the number having the data type "float"
 * @return  number that having type "float"
 */
float* Telemetry_receiveFloat(void) {
    // Receive four-byte raw digit
    s32 rec = _Telemetry_receiveRawData(sizeof(float));
    // Getting sign from recevived raw digit
    u8 sign = rec >> 31 & 0xFF;
    // Getting exponenta from recevived raw digit
    u8 exponenta = rec >> 23 & 0xFF;
    // Getting mantissa from recevived raw digit
    u32 mantissa = rec & 0x7FFFFF;

    // Calculating float digit and return it
    float a = mantissa / pow(2, 23);
    float *result = (float *)malloc(sizeof(float));
    *result = pow(-1, sign) * (1 + a) * pow(2, exponenta - 127);
    return result;
}

/**
 * Transmitting an array of n-bytes digits using UART interface
 * @param arr  - an array of n-bytes digits
 * @param type - an array items type
 * @param len  - length of array
 */
void Telemetry_transmitArray(s32* arr, u8 type, u8 len) {
    // Transmitting an array length
    Telemetry_transmitData(len);

    // Transmitting an array type
    Telemetry_transmitData(type);

    // Transmitting data
    for (u8 i = 0; i < len; i++) {
        Telemetry_nthBytesTransmit(arr[i], type);
    }
}

/**
 * Transmitting an array of n-bytes digits using UART interface
 * @return     an array of n-bytes digits
 */
s32* Telemetry_receiveArray(void) {
    // Receiving an array length
    u8 len = Telemetry_receiveData();

    // Receiving an array type
    u8 type = Telemetry_receiveData();

    // Allocating memory to the data
    s32 *arr = (s32 *)malloc(len * sizeof(s32));

    for (u8 i = 0; i < len; i++) {
        // Receiving an array item
        arr[i] = Telemetry_nthBytesReceive(type);
    }
    return arr;
}

/**
 * Create telemetry items
 * @param  count     - number of telemetry items
 * @param  ids       - identifiers of telemetry items
 * @param  functions - callbacks of telemetry items
 * @param  types     - variables types which return by callback functions
 * @param  arr_len   - an arrays length
 * @param  arr_type  - an arrays types
 * @return           telemetry items structure
 */
telemetry_item* Telemetry_getItems(u8 count, u8* ids, getter* functions, u8* types, u8* arr_len, u8* arr_type) {
    telemetry_item* items = (telemetry_item *)malloc(sizeof(telemetry_item) * count);

    u8 a = 0;
    for (unsigned char i = 0; i < count; i++) {
        items[i].id = ids[i];
        items[i].func = functions[i];
        items[i].type = types[i];

        if (types[i] == ARRAY) {
            items[i].array.length = arr_len[a];
            items[i].array.type = arr_type[a++];
        }
    }
    return items;
}

/**
 * Transmitting Telemetry data
 * @param data  - n-bytes values for transmitting
 * @param type  - data type identifier
 * @param array - an array information
 */
void Telemetry_dataTransmit(telemetry_item* item) {
    // Transmitting "start" identifier
    _Telemetry_transmitRawData(START, TWO_BYTE);

    // Transmitting data type identifier
    Telemetry_transmitData(item->type);

    // Check data type and use a special function if a type is "float" or "array"
    switch (item->type) {
        case FLOAT: {
            // Transmitting data that having "float" type and return from the function
            float_point func = (float_point)item->func;
            float d = func();

            Telemetry_transmitFloat(&d);
            return;
        }

        case ARRAY: {
            // Transmitting data that having "array" type and return from the function
            fixed_array func = (fixed_array)item->func;
            s32* arr = func();

            Telemetry_transmitArray(arr, item->array.type, item->array.length);
            free(arr);
            return;
        }
    }

    // Transmitting data
    fixed_point func = (fixed_point)item->func;
    int d = func();
    Telemetry_nthBytesTransmit(d, item->type);
}

/**
 * Listening the Rx wire and transmitting data on request
 * @param items - telemetry items structure
 * @param count - number of telemetry items
 */
u8 Telemetry_streamData(telemetry_item* items, u8 count) {
    // // Receiving data identifier
    u8 id = Telemetry_receiveData();

    for (u8 i = 0; i < count; i++) {
        // Find telemetry item with right identifier
        if (items[i].id == id) {
            // Transmitting the data
            Telemetry_dataTransmit(&items[i]);
        }
    }

    return id;
}

/**
 * Getting telemetry data after transmitting identifier
 * @param id    - data identifier
 */
void* Telemetry_getData(u8 id) {
    // Transmitting data identifier
    Telemetry_transmitData(id);

    // If the identifier "start" is not received - do nothing
    if (_Telemetry_receiveRawData(TWO_BYTE) != START) return NULL;

    // Receiving data type identifier
    u8 type = Telemetry_receiveData();

    switch (type) {
        case ARRAY: {
            s32* data = Telemetry_receiveArray();
            return data;
        }

        case FLOAT: {
            float* data = Telemetry_receiveFloat();
            return data;
        }

        default: {
            s32* data = (s32 *)malloc(sizeof(s32));
            *data = Telemetry_nthBytesReceive(type);
            return data;
        }
    }
}
