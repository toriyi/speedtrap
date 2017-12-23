#include <avr/interrupt.h>
#include "encoder.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h> 

void init_encoder(void){
	PORTD |= ((1 << PD2) | (1 << PD3)); //Enable pull up resistors
	PCICR |= (1 << PCIE2);           
	PCMSK2 |= ((1 << PCINT18) | (1 << PCINT19)); 

    bits = PIND;
    a = bits & (1 << PD2);
    b = bits & (1 << PD3);
    if (!b && !a)
		old_state = 0;
    else if (!b && a)
		old_state = 1;
    else if (b && !a)
		old_state = 2;
    else
		old_state = 3;

    new_state = old_state;
}


ISR(PCINT2_vect){
	// Read the input bits and determine A and B

    bits = PIND;
    a = bits & (1 << PD2);
    b = bits & (1 << PD3);

    // Output max to LCD
	if (old_state == 0) {

	    // Handle A and B inputs for state 0
	    if(a != 0){
	        max++;
	   		new_state = 1;
	    }
	    else if(b != 0){
	    	max--;
	    	new_state = 2;
	    }

	}
	else if (old_state == 1) {

	    // Handle A and B inputs for state 1
	    if(a == 0){
	        max--;
	   		new_state = 0;
	    }
	    else if(b != 0){
	    	max++;
	    	new_state = 3;
	    }

	}
	else if (old_state == 2) {

	    // Handle A and B inputs for state 2
	    if(a != 0){
	        max--;
	   		new_state = 3;
	    }
	    else if(b == 0){
	    	max++;
	    	new_state = 0;
	    }

	}
	else {   

	    // Handle A and B inputs for state 3
	    if(a == 0){
	        max++;
	   		new_state = 2;
	    }
	    else if(b == 0){
	    	max--;
	    	new_state = 1;
	    }

	}

	//speed between 1 and 99
	if(max < 1){
		max = 1;
	}
	if(max > 99){
		max = 99;
	}
	//change flag state 
	flag = 1;
	old_state = new_state;
	eeprom_update_byte((void *) 200, max);

}