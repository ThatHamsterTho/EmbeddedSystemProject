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

char counter = 1;   // cycles between 1,2,4 and 8.
char C = 0;
char cOld = 0;
char aOld = 0;
char bOld = 0;

char rotate_counter = 0;

void update_counter(void);

void __interrupt() ROT_ISR(void){
	if(RBIF && RBIE) {
        RBIF = 0;
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

        // source: http://www.technoblogy.com/show?1YHJ
        char a = (!PORTBbits.RB4) & 1;
        char b = (!PORTBbits.RB5) & 1;
        if(a != aOld){							// signal A has changed
            aOld = a;
            if(b != bOld){						// on transition A, we compare signal B with cleaned up signal bOld, or C
                bOld = b;						// if it has changed, there was an rotation
                C = (a==b) ? 0 : 1;				// depending on the state of a and b, the rotation is CCW or CW
                if(C != cOld){					// protection from CCW -> CW or CW -> CCW not registering
                    update_counter();
                    rotate_counter = 0;
                }
                else if(rotate_counter >= 2){	// indents send 2 signals per indent 
                    update_counter();
                    rotate_counter = 0;
                }
                cOld = C;						// update cOld
                rotate_counter++;
            }
        }
        
	}
}

void main(void) {
    TRISB = 0b00110000;     // set RB4 and RB5 to input
    ANSELHbits.ANS11 = 0;   // turn off analog read of RB4
    ANSELHbits.ANS13 = 0;   // turn off analog read of RB5
	TRISA = 0;              // set PORTA to output

    INTCONbits.GIE = 1;     // enable global interrupts
	INTCONbits.RBIE = 1;    // enable interrupt-on-change port B
    IOCB = 0b00010000;      // enable interrupts on RB4

	while(1){
        PORTA = ~counter;
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