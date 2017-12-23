#include <avr/interrupt.h>
#include "serial.h"

#define FOSC 16000000           // Clock frequency
#define BAUD 9600               // Baud rate used
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register

void serial_init(void)
{
	UBRR0 = MYUBRR; // Set baud rate
	UCSR0B |= (1 << TXEN0 | 1 << RXEN0 | 1 << RXCIE0); // Enable RX and TX
	UCSR0C = (3 << UCSZ00); // Async., no parity
	DDRD |= (1 << PD1); //set PD1 to output for RX/TX
	DDRC |= (1 << PC5); //Set enable bit to output
}


char serial_rxchar(){

	while (!(UCSR0A & (1 << RXC0))) {

	}
	return UDR0;
}

// Use to send each character in message
void serial_txchar(char ch){
    // Wait for transmitter data register empty
    while ((UCSR0A & (1<<UDRE0)) == 0);
    UDR0 = ch;
}

//use interrupt to handle recieved character
ISR(USART_RX_vect){
	char cha = serial_rxchar();
	//the begin char
	if(cha == '@'){
		start_flag = 1;
		stop_flag = 0;
		int i = 0;
		while(i < 5){
			buffer[i] = 0;
			i++;
		}
		buf_char = 0;
	}
	else if(start_flag == 1){
		if((cha >= '0') && (cha <= '9')){
			buffer[buf_char] = cha;
			buf_char++;
		}
		else if((cha == '$') && (buf_char > 0)){
			stop_flag = 1;
		}
	}
}
