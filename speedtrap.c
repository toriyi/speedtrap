#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include <avr/interrupt.h>
#include "encoder.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h> 
#include <stdio.h>
#include "speedtrap.h"
#include "serial.h"

//define all the variables
volatile unsigned int time = 0;
volatile unsigned char state = 0; 
volatile unsigned char bits, a, b;
volatile unsigned int speed_all; 
volatile unsigned int speed_dec;
volatile unsigned int speed_scale_the_time;
volatile unsigned int mm;
volatile unsigned int mm_scaled;
char ostr[20];

int main(void){
	DDRB |= (1 << PB5); //LED
	DDRB |= (1 << PB4); //buzzer
	DDRC &= ~((1 << PC2) | (1 << PC3)); //Initialize PC2 and PC3 for input
	PORTB |= (1 << PB2) | (1 << PB3); // Pull-up resistors for phototransistors
	PORTC |= (1 << PC2) | (1 << PC3); 
	
	lcd_init();  // Initialize the LCD
	serial_init();	//initialize serial
	init_timer1(250); //initialize TIMER1 

	PCICR |= (1 << PCINT1); //Enable pin change interrupts
	PCMSK1 |= ((1 << PCINT10) | (1 << PCINT11));


	//clears the screen
	lcd_writecommand(1);

	unsigned char bits = PINC;
    unsigned char a = bits & (1 << PC2);
    unsigned char b = bits & (1 << PC3);    
	init_encoder(); //enable encoder
	max = eeprom_read_byte((void*) 200);
	//print the max count 
	snprintf(ostr, 20, "max %2d", max);
	lcd_moveto(1,0);
	lcd_stringout(ostr);
	//set global interrupt 
	UCSR0B |= (1 << RXCIE0); 
	sei();

	while(1)
	{			
		//retrieved
		if(stop_flag)
		{
			stop_flag = 0; 
			int got_speed;
			sscanf((char*)buffer, "%d", &got_speed); //get the received speed
			if(got_speed > max*10) //if above speed limit, play sound 
			{
				play_note(400);
			}
			lcd_moveto(1, 8);
			snprintf(buffer, 5, "%d.%d", got_speed/10, got_speed%10);
			lcd_stringout(buffer);
		}
		
		if(state == 1) 
		{
			snprintf(ostr, 20, "max:%2d", max);
			lcd_moveto(1, 0);
			lcd_stringout(ostr);    
			PORTB &= ~(1 << PB5); //turn off LED 
			lcd_writecommand(1);
			snprintf(ostr, 20, "max:%2d", max);
			lcd_moveto(1, 0);
			lcd_stringout(ostr);    			
			lcd_moveto(0, 0);
			char time_print[30];
			snprintf(time_print, 30, "%4dms= ", time);
			lcd_stringout(time_print);						
			speed_scale_the_time = 43180/time; 
			mm_scaled = 431800/time;
			speed_all = speed_scale_the_time/10; //the number
			speed_dec = speed_scale_the_time%10; //the number in decimal
			mm = mm_scaled/10;			
			lcd_moveto(0, 7);
			char print_the_speed[30];
			snprintf(print_the_speed, 30, "%4d.%d", speed_all, speed_dec);
			lcd_stringout(print_the_speed);			
			//the transmission 
			char send_speed[6] = {0,0,0,0,0,0};
			snprintf(send_speed, 6, "@%d$", mm);
			int i = 0;
			//sending the speed
			while(send_speed[i])
			{
				serial_txchar(send_speed[i]);
				i++;
			}
			state = 0;
		} 
		else if(state == 2) 
		{
			PORTB |= (1 << PB5);
		}		
		snprintf(ostr, 20, "max: %2d", max);
		lcd_moveto(1, 0);
		lcd_stringout(ostr);    
		
	}
	return 0;
	}

ISR(PCINT1_vect){ 
	//phototransistors check states
    unsigned char bits = PINC;
    unsigned char a = bits & (1 << PC2);
    unsigned char b = bits & (1 << PC3);

	if (a == 0) {
		if(state == 0){
			state = 2;
		}
	}
	else if (b == 0) {
		if(state == 2){
			state = 1;
		}
	}
}


ISR(TIMER1_COMPA_vect){ 
	//in process of running
	if (state == 0)
		time = 0;
	if(state == 2){
		time++;
	}
	PORTB ^= (1 << PB5);
}


//enable timer interrupt
void init_timer1(unsigned short m)
{
TCCR1B |= (1 << WGM12);
TIMSK1 |= (1 << OCIE1A);
OCR1A = m;
TCCR1B |= (1 << CS11) | (1 << CS10);
}


/*
  Play a tone at the frequency specified for one second
*/
void play_note(unsigned short freq)
{
    unsigned long period;

    period = 1000000 / freq;      // Period of note in microseconds

    while (freq--) {
	// Make PB4 high
       PORTB |= (1 << PB4);
	// Use variable_delay_us to delay for half the period
       variable_delay_us(period/2);
	// Make PB4 low
       PORTB &= ~(1 << PB4);
	// Delay for half the period again
       variable_delay_us(period/2);
    }
}


/*
    variable_delay_us - Delay a variable number of microseconds
*/
void variable_delay_us(int delay)
{
    int i = (delay + 5) / 10;

    while (i--)
        _delay_us(10);
}

