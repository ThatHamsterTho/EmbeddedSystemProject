/*
 * File:   main.c
 * Author: mcimr
 *
 * Created on December 8, 2019, 1:46 PM
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


// !!!!!!!!!!!!!! DEFINES !!!!!!!!!!!!!! //

/* AREAS is de waarde die aangeeft hoeveel waardes moeten kunnen worden gerepresenteerd. waarde bereik: 1-x, waarin x aantal LEDs zijn.
 * DEADZONE is het gebied waarbij waardes moeten worden genegeerd rondom randwaarden,
 * de randwaarden worden bepaald via AREAS en ADCMAX
 * ADCMAX geeft aan hoeveel waardes mogelijk zijn in de ADC module
 * MAXSTATES geeft aan hoeveel waarden maximaal moeten kunnen worden opgenomen.
 * PRECISION wordt gebruikt voor het afwachten van de ADCmodule, dit getal kan van 2-10 worden gezet.
 * Des te lager het getal des te sneller het programma, ookal heeft dit geen erg in menselijk waarnemen.
 */

#define AREAS 5
#define DEADZONE 20
#define ADCMAX 1024
#define MAXSTATES 10

// bereken precision vanuit de formule: ceil(log2(AREAS))
#define PRECISION 4
// TMR2 buffer
#define REFRESHRATE 49  
// 1 / (Fosc/4) / (Pre-scale * Post-scaler) = 256us
// (256 * TMR2_max_val) - (1/80 * 10^6) = 53036 us
// 53036 / 256 = 207
// 256 - 207 = 49
#define IRtime 71		// 422us voor 455Khz, 480us voor 400Khz, TMR2 is on 1:24 so every 1 Ccycle : 6us

#define rottime 1000    // 1000ms for every Area.


// !!!!!!!!!!!!!! VARIABLES !!!!!!!!!!!!!! //

void picinit(void);

typedef unsigned char bits8;
typedef unsigned int  bits16;

struct rotarytracker {
    bits8 a     : 1;
    bits8 b     : 1;
    bits8 C     : 1;
    bits8 aOld  : 1;
    bits8 bOld  : 1;
    bits8 cOld  : 1;
    bits8 rotate_counter : 2;
} rottracker;

struct _InterruptFlags {
	bits8 IRpacketRecieved	: 2;
    bits8 RefreshADC        : 1;
    bits8 rotarychanged     : 1;
	bits8					: 6;
} InterruptFlags;

char SelectedChannel = 1;


// !!!!!!!!!!!!!! FUNCTION DEFINITONS !!!!!!!!!!!!!! //

// rotary
void update_SelectedChannel(void);


void __interrupt() ISR(void){
    if(RBIF && RBIE && !INTF){
        RBIF = 0;
        rottracker.a = (!PORTBbits.RB4) & 1;
        rottracker.b = (!PORTBbits.RB5) & 1;
        if(rottracker.a != rottracker.aOld){
            rottracker.aOld = rottracker.a;
            if(rottracker.b != rottracker.bOld){
                rottracker.bOld = rottracker.b;
                rottracker.C = (rottracker.a==rottracker.b) ? 0 : 1;
                if(rottracker.C != rottracker.cOld){
                    update_SelectedChannel();
                    rottracker.rotate_counter = 0;
                    InterruptFlags.rotarychanged = 1;
                }
                else if(rottracker.rotate_counter >= 2){                
                    update_SelectedChannel();
                    rottracker.rotate_counter = 0;
                    InterruptFlags.rotarychanged = 1;
                }
                rottracker.cOld = rottracker.C;
                rottracker.rotate_counter++;
            }
        }
    }
} 

void main(void){
    picinit();
    
    while(1){
        if(InterruptFlags.rotarychanged){
            PORTA = ~SelectedChannel;
            InterruptFlags.rotarychanged = 0;
        }
    }

	return;
}

void picinit(){
    TRISA = 0;              // LED output
    TRISBbits.TRISB4 = 1;   // rotary input A
    TRISBbits.TRISB5 = 1;   // rotary input B
    
    ANSELHbits.ANS11 = 0;   // turn off analog read of RB4
    ANSELHbits.ANS13 = 0;   // turn off analog read of RB5
    
    IOCBbits.IOCB4  = 1;    // enable interrupts on RB4
    IOCBbits.IOCB5  = 1;    // enable interrupts on RB5
    INTCONbits.RBIE = 1;    // enable interrupt-on-change port B
    INTCONbits.GIE  = 1;    // enable global interrupts
}

void update_SelectedChannel(void){
    // if C is 1 then it is clockwise, else it it counterclockwise
	if(rottracker.C){  // clockwise
		if(SelectedChannel == 8){
			SelectedChannel = 1;
		}
		else{
			SelectedChannel<<=1;
		}
	}
	else{   // counterclockwise
		if(SelectedChannel == 1){
			SelectedChannel = 8;
		}
		else{
			SelectedChannel>>=1;
		}
	}
}