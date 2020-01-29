/*
 * File:   interrupttest.c
 * Author: mcimr
 *
 * Created on November 11, 2019, 12:44 PM
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

#define DEBOUNCE 20

char C = 0;         // C represents the rotation of the rotary encoder
// change edge dependent on starting value of RB4. edge is the inverse of the starting position of RB4.
char edge = 0;      // keeps track of rising edge or falling edge;
char counter = 1;   // cycles between 1,2,4 and 8.

// this file contains 2 interrupt tests.
// one to test if the interrupt function works overall
// and one that will test if the full interrupt function will work.
// in the first test the LEDs will flicker
// in the second test the LEDs should be cycling

void __interrupt() ROT_ISR(void){
    if(INTCONbits.RBIF && INTCONbits.RBIE) {
        /* expected result:
         * because RB5 and RB4 are both set as interrupt-on-change
         * the PORTA LEDs will turn on once, because RBIF is not reset.
         */

        /*
			Possible states:
			
			edge	RB5		C
			falling	0		1
			rising	0		0
			falling	1		0
			rising	1		1

			table is the result of an xnor operation.
			Edge and RB5 are inverses of this table.
		*/
    

        //PORTA = ~0x0F;
		
		edge = !edge;
        C = !(edge ^ PORTBbits.RB5);
		//update_counter(); // update counter;
		__delay_ms(DEBOUNCE); // debounce time;
        INTCONbits.RBIF = 0;
		
	}
}

void main(void) {
    TRISB = 0b00110000;     // set RB4 and RB5 to input
    ANSELHbits.ANS11 = 0;   // turn off analog read of RB4
    ANSELHbits.ANS13 = 0;   // turn off analog read of RB5
	TRISA = 0;              // set PORTA to output

	edge = PORTBbits.RB4;	// set edge to starting value of RB4

    INTCONbits.GIE = 1;     // enable global interrupts
    INTCONbits.PEIE = 1;    // enable peripheral interrupts
	INTCONbits.RBIE = 1;    // enable interrupt-on-change port B
    IOCB = 0b00010000;      // enable interrupts on RB4
    
    PORTA = ~0;
    
	while(1){
        PORTA = ~C;
	}
}
