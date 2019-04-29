#include "uart.h"

void UART_String( print_t printer, char* string ){
   uartWriteString( printer, string );
}

void UART_USB_String( char* string ){
   uartWriteString( UART_USB, string );
}

void UART_LED_status( int i )
{
	char c[2];
	c[0] = (i+1) + '0';
	c[2] = '\0';

	UART_USB_String( "LED " );
	UART_USB_String( c );
	UART_USB_String( " toggle\r\n" );
}
