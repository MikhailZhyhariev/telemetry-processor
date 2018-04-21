/* Name: telemetry.c
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
 * Transmitting a n-byte data
 * @param data
 * @param bytes - number of bytes of the register
 */
void Telemetry_nthBytesTransmit(s32 data, u8 bytes) {
    data = BMP180_checkSign(data);
    for (u8 i = 0; i < bytes; i++) {
        USART_Transmit((data >> (8 * (bytes - i - 1))) & 0xFF);
    }
}

/**
 * Receiving two bytes using UART interface
 * @return    received two-bytes data
 */
int Telemetry_twoBytesReceive(void) {
    int data = USART_Receive() << 8;
    data |= USART_Receive();
    return data;
}

/**
 * Transmitting an array of two-byte digits using UART interface
 * @param arr - an array of two-byte digits
 * @param len - length of array
 */
void Telemetry_arrayTransmit(int* arr, unsigned char len) {
    for (unsigned char i = 0; i < len; i++) {
        int data = arr[i];
        // If the data variable is less than zero, invert it and transmitting the "minus" identifier
        if (data < 0) {
            Telemetry_twoBytesTransmit(MINUS);
            data = -(data);
        } else {
            // Else transmitting "plus" identifier
            Telemetry_twoBytesTransmit(PLUS);
        }

        // Transmitting data
        Telemetry_twoBytesTransmit(data);
    }
}

/**
 * Transmitting Telemetry data
 * @param id   data type identifier
 * @param data - two-byte value for Transmitting
 */
void Telemetry_dataTransmit(int id, int* data) {
    // Transmitting "start" identifier
    Telemetry_twoBytesTransmit(START);

    // Data array length
    unsigned char len = 3;

    switch (id) {
        // If it data from accelerometer transmitting "accel" identifier
        case ACCEL:
            Telemetry_twoBytesTransmit(ACCEL);
            break;

        // If it data from gyroscope transmitting "gyro" identifier
        case GYRO:
            Telemetry_twoBytesTransmit(GYRO);
            break;

        // If it data from temperature transmitting "temp" identifier
        case TEMP:
            // Set length is one because the temperature is one two-bytes digit
            len = 1;
            Telemetry_twoBytesTransmit(TEMP);
            break;
    }

    // Transmitting data array
    Telemetry_arrayTransmit(data, len);
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
