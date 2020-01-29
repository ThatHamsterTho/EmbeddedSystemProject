/*
 * File:   main.c
 * Author: mcimr
 *
 * Created on January 4, 2020, 1:48 PM
 */


#define _XTAL_FREQ 4000000
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
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
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include "SPIlib.h"

struct _InterruptFlags {
	bits8 IRpacketRecieved	: 2;
    bits8 RefreshADC        : 1;
    bits8 rotarychanged     : 1;
    bits8 motorrotating     : 1;
    bits8 RefreshSPI        : 1;
	bits8					: 5;
} InterruptFlags;

// !!!!!!!!!!!!!! VARIABLES !!!!!!!!!!!!!! //
char SelectedChannel =  1;
char vol = 40;

// !!!!!!!!!!!!!! FUNCTION DEFINITONS !!!!!!!!!!!!!! //

// init funtioncs
void picinit(void);
void SPIinit(void);
void TMRinit(void);

void __interrupt() ISR(void){
    if (T0IF && T0IE) {
        InterruptFlags.RefreshSPI = 1; // markeer dat de ADC moet worden herladen
        TMR0 = 64;  // reset TMR0 so it runs at 80Hz
        TMR0IF = 0; // reset flag
    }
}


void main(void){
    picinit();
	while(1){       
        if (InterruptFlags.RefreshSPI) {
			sendVol(vol);
            __delay_us(10);
            sendInput(SelectedChannel);
            InterruptFlags.RefreshSPI = 0; // reset software flag
        }
	}
}


void picinit(){
    TRISA = 0;              // LED output
	TRISC = 0;				// motor & Display 1 output
	TRISD = 0;				// display 2 output

	SPIinit();
    TMRinit();
    
    INTCONbits.RBIE = 1;    // enable interrupt-on-change port B
    INTCONbits.PEIE = 1;    // enable peripheral interrupts
    INTCONbits.GIE  = 1;    // enable global interrupts
}


void TMRinit(){
    OPTION_REGbits.T0CS = 0;    // internal clock
    OPTION_REGbits.PSA = 0;     // TIMER0 gets prescaler assigned
    OPTION_REGbits.PS  = 0b101; // TIMER0 prescaler 1:64
    TMR0 = 64;
    OPTION_REGbits.INTEDG = 1;  // edge detection INT pin on rising
    
    T0IE            = 1;    // enable Timer 0 Overflow Interrupt
    T0IF            = 0;    // reset Timer0 flag
}