#ifndef UART_H_
#define UART_H_

#include "sapi.h"

#define UART_config(uart,baudRate)     			printConfigUart(&(debugPrint),(uart),(baudRate))
#define UART_print_string(string)				printlnString((debugPrint),(string))

#define BAUD_RATE 		115200

/* 10 caracteres maximo! */
#define MSG_WELCOME "Iniciando"


typedef uartMap_t print_t;

void UART_String( print_t printer, char* string );
void UART_USB_String( char* string );
void float_to_string( float f );
int calculate_integer(float f);
int calculate_decimal(float f,int size);
#endif

