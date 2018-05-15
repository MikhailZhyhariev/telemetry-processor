Description
------------

Telemetry_processor is a library for simple data transfer between devices on
different data protocols.
Supported devices:
-   AVR microcontrollers;
-   orangePi;
-   raspberryPi;
-   etc...
Supported protocols:
-   UART;
-   I2C;
-   SPI.

Initialize and usage
--------------------

To initialize, you must use the `Temeletry_init` and `Telemetry_getItems`
functions, which accepts:
-   amount of elements;
-   array of identifiers;
-   an array describing the data type;
-   array of callback functions;
-   An array describing the data type of the arrays;
-   array with lengths of arrays.

Example code initialization module:
```
// Identifiers
#define EXAMPLE_1           1
#define EXAMPLE_2           2
#define EXAMPLE_3           3
#define EXAMPLE_4           4

// Amount of stream elements
#define COUNT               4

int main(void) {
    // This function initialize data transfer protocol
    Telemetry_init();

    // An array of callback functions
    // You should use an explicit cast (getter)
    getter functions[COUNT] = {
        (getter)function_1,
        (getter)function_2,
        (getter)function_3,
        (getter)function_4
       };

       // An array of data types
       u8 types[COUNT] = {
           ONE_BYTE,
           TWO_BYTE,
           ARRAY,
           FLOAT,
       };

       // Identifiers
       u8 ids[COUNT] = {
           EXAMPLE_1,
           EXAMPLE_2,
           EXAMPLE_3,
           EXAMPLE_4,
        };

       // Information about array (type and length)
       u8 arr_type[] = {TWO_BYTE};
       u8 arr_len[] = {3};

       // Initialize stream items
       telemetry_item* items = Telemetry_getItems(COUNT, ids, functions, types, arr_len, arr_type);
}
```

If you don't have an `ARRAY` type data you can use `NULL` instead of `arr_len`
and `arr_type` arrays.
`Telemetry_getItems` function return an array of the `telemetry_item` structure.
To transfer data, you need to use the `Telemetry_streamData` function, which
takes a number of elements and an array of `telemetry_item` structures, and
returns the received identifier.
On the receiving side, you need to use the function `Telemetry_getData`, which
takes an identifier, and returns a `telemetry_item` structure that contains the
received data.
Example.
Source code on transmitting side:
```
unsigned char id = Telemetry_streamData(items, COUNT);
switch (id) {
    case EXAMPLE_1:
        // do something...
        break;
    case EXAMPLE_2:
        // do something...
        break;
        // etc...
}
```

Source code on receiving side:
```
#define EXAMPLE_1           1
#define EXAMPLE_2           2
#define EXAMPLE_3           3
#define EXAMPLE_4           4

telemetry_item* item = Telemetry_getData(EXAMPLE_1);

// Received data
item->data
// Received type
item->type

// If the data type is ARRAY
// Received an array length
item->array.length
// Received an array items type
item->array.type
```

Data types
----------

Available type of a data:
-   `ONE_BYTE`
-   `TWO_BYTE`
-   `FOUR_BYTE`
-   `ARRAY`
-   `FLOAT`

Data transmit/receive protocol
------------------------------
You can use any data transmission/receiving functions regardless of the data
transmission protocol.
In `init` function, you can initialize protocol and put some settings.
`transmitData` function should have type «`unsigned char`» argument value - data
`receiveData`  function should have type «`unsigned char`» returning value.
Example for AVR microcontrollers:
```
#define Telemetry_init()                (USART_Init(MYUBRR))
#define Telemetry_transmitData(x)       (USART_Transmit(x))
#define Telemetry_receiveData()         (USART_Receive())
```

Example for OrangePi:
```
// UART settings
#define BAUD                            4800
#define INTERFACE                       "/dev/ttyS0"
// File descriptor
static int fd;
// Initialization serial interface for orangePi
#define Telemetry_init()                (fd = serialOpen(INTERFACE, BAUD))
#define Telemetry_transmitData(data)    (serialPutchar(fd, data))
#define Telemetry_receiveData()         (serialGetchar(fd))
```

In `telemetry_processor.h` file you also can specify the path to the data
transmit/receive protocol library.
