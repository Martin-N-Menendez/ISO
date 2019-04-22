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

void float_to_string( float f )
{
	int n,div;
	uint32_t i;
	int integer_size,decimal_size;
	int integer_part,decimal_part;
	char c[8];

	integer_size = calculate_integer(f);
	integer_part = (int)f;

	decimal_size = calculate_decimal(f,integer_size);
	int multi = pot(decimal_size+1);
	decimal_part = (int)(f*multi)-((int)f)*multi;

	div = pot(integer_size);

	for ( i = 0 ; i < (integer_size) ; i ++)
	{
		n = integer_part / div;
		c[i] = n + '0';
		integer_part -= n*div;
		div /= 10;
	}
	c[integer_size] = '.';

	//UART_USB_String( c );

	div = pot(decimal_size);

	i = integer_size+1;
	for ( i = integer_size+1 ; i < (integer_size+decimal_size+1) ; i ++)
	{
		n = decimal_part / div;
		c[i] = n + '0';
		decimal_part -= n*div;
		div /= 10;
	}
	c[i] = '\0';
	UART_USB_String( c );
}


int calculate_integer(float f)
{
	int i = 0;

	int num = (int)f;

	while ((num % 10) > 0)
	{
		num /= 10;
		i++;
	}

	return i;
}

int calculate_decimal(float f,int size)
{
	int i = 1;

	int multi = pot(size);

	int num = (int)(f*multi)-((int)f)*multi;

	while ((num / 10) > 0)
	{
		num /= 10;
		i++;
	}

	return i;
}

int pot(int decimal_size)
{
	int power = 1;
	int i;


	for (i = 1 ; i < decimal_size ; i++)
	{
		power *= 10;
	}

	return power;
}
