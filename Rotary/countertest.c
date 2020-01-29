/*
 * File:   countertest.c
 * Author: mcimr
 *
 * Created on November 11, 2019, 12:22 PM
 */
#define _XTAL_FREQ 4000000


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_CLKOUT// Oscillator Selection bits (INTOSC oscillator: CLKOUT function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR21V   // Brown-out Reset Selection bit (Brown-out Reset set to 2.1V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

char C = 0;         // C represents the rotation of the rotary encoder
// change edge dependent on starting value of RB4. edge is the inverse of the starting position of RB4.
char edge = 0;      // keeps track of rising edge or falling edge;
char counter = 1;   // cycles between 1,2,4 and 8.

void update_counter(void);

void main(void) {
    TRISA = 0;              // set PORTA to output
    PORTA = 0;
    
	while(1){
        C = 0b00001111;
        /*
         * expected result:
         * counter goes 4 times left; (cw)
         * then counter goed 4 times right; (ccw)
		 * If the LEDs start with a ccw rotation, 
		 * change if(C) in update_counter() to !C
         */
        
        for(int i = 0; i < 8; i++){
            update_counter();
            PORTA = ~counter;
            C = (char)(C>>1);
            __delay_ms(200);
        }
	}
}

void update_counter(void){
    // if C is 1 then it is clockwise, else it it counterclockwise
	if(C){  // clockwise
		if(counter == 8){
			counter = 1;
		}
		else{
			counter<<=1;
		}
	}
	else{   // counterclockwise
		if(counter == 1){
			counter = 8;
		}
		else{
			counter>>=1;
		}
	}
}