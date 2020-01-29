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
// TMR2 buffer = 0
#define IRtime 71		// 422us voor 455Khz, 480us voor 400Khz, TMR2 is on 1:24 so every 1 Ccycle : 6us

#define rottime 2400    // 9000-15000ms = full, 2400ms for every Area on 12000ms
#define Q7      PORTCbits.RC1
#define Q8      PORTCbits.RC0
#define potCW   Q7 = 1; Q8 = 0; // active low
#define potCCW  Q7 = 0; Q8 = 1; // active low
#define potREM  Q7 = 1; Q8 = 1; // een van de twee? kijk welke sneller is
#define potREM2 Q7 = 0; Q8 = 0; // waarom? niels zegt handig

// !!!!!!!!!!!!!! VARIABLES !!!!!!!!!!!!!! //
int AreaSize = ADCMAX / AREAS; // bereken areasize 1 keer.
char prev_ADCvalue = 0; // houdt bij wat de vorige waarde van de ADC meeting was
char ADC_draaiwaarde = 0;
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
    bits8 motorrotating     : 1;
	bits8					: 5;
} InterruptFlags;

typedef union _irsignal{
	struct __splitdatapacket__ {
        // data byte 1;
        // 0b.... HHHH SSDD DDDD
        // 0b0000 HHHH SSDD DDDD
        //          0123 45
        //          DDDD DDSS HHHH
        // PORTA 76 5432 10
		bits16 data 	: 6;
        bits16 S2       : 1;
        bits16 S1       : 1;
        bits16 H        : 1;
		bits16 Header 	: 3;
        bits16          : 4;
	}spdp;
	struct __packetbits__ {
		bits16 D6       : 1;
		bits16 D5       : 1;
		bits16 D4       : 1;
		bits16 D3       : 1;
		bits16 D2       : 1;
		bits16 D1       : 1;
		bits16 S2  	   	: 1;
		bits16 S1		: 1;
		bits16 H		: 1;
		bits16 			: 7;
	}pbits;
	bits16 datapacket;					// 16 bits
} irsignal;

irsignal packet[] = {{0},{0}};

signed char IRbitCounter    = 11;
char SelectedChannel =  1;


// !!!!!!!!!!!!!! FUNCTION DEFINITONS !!!!!!!!!!!!!! //

// adc
// functie om de ADC waarde (0-1023) om te zetten naar een waarde van (0-5)
// hier is jitter al bij weg gewerkt.
char getAreaVal(char prev_ADCvalue);
int getADCval(void);
// rotary
void update_SelectedChannel(void);


void __interrupt() ISR(void){
    if((INTF && INTE)){
        INTF = 0;
        OPTION_REGbits.INTEDG ^= 1; // toggle edge detector
        TMR2ON ^= 1;
        if(TMR2ON == 0){
            // kijken hoe lang?
            if(TMR2 > IRtime){
                // dan is 1
                packet[InterruptFlags.IRpacketRecieved].datapacket |= (bits16)(1<<IRbitCounter);
                IRbitCounter--;
            }
            else{
                // dan is 0
                // packet.datapacket |= 0<<IRbitCounter; 
                // niet nodig, want 0 bitshift wordt 0, plek blijft leeg.
                IRbitCounter--;
            }
            TMR2 = 0;
        }
        if(IRbitCounter < 0){
            // whole packet ja!
            InterruptFlags.IRpacketRecieved++;
            IRbitCounter = 11;
        }
    }
} 

void main(void){
    picinit();
    
    while(1){
        // HHH H S1S2 DDDDDD
        if(InterruptFlags.IRpacketRecieved == 2){   
            if(packet[0].datapacket == packet[1].datapacket){
            // goed ontvangen yo!
                if(packet[0].spdp.Header == 0b101){
                    // correct type of signal found;
                    //PORTAbits.RA0 = packet[0].spdp.H;
                    //PORTAbits.RA1 = packet[0].spdp.S1;
                    //PORTAbits.RA2 = packet[0].spdp.S2;
                    //__delay_ms(100);
                    //PORTA = ~0;

                    //            0123 45
                    //            DDDD DDSS HHHH
                    // PORTA 7654 3210
                    if(packet[0].spdp.S1){      
                        switch(packet[0].spdp.data){
                            case 0b001000:  // K9
                                // INPUT4
                                PORTA = ~(0b1000);
                                break;
                            case 0b010000:  // K8
                                // INPUT1
                                PORTA = ~(0b0100);
                                break;
                            case 0b100000:  // K7
                                // INPUT3
                                PORTA = ~(0b0001);
                                break;
                            default:
                                // dit zou niet moeten?
                                PORTA = ~(0b1001);
                                break;
                        }
                    }
                    if(packet[0].spdp.H){
                        switch(packet[0].spdp.data){
                            case 0b001000:  // K3
                                // INPUT2
                                PORTA = ~(0b1000);
                                break;
                            case 0b010000:  // K2
                                // VOL-
                                PORTA = ~(0b0100);
                                break;
                            case 0b100000:  // K1
                                // VOL+
                                PORTA = ~(0b0001);
                                break;
                            default:
                                // dit zou niet moeten?
                                PORTA = ~(0b0110);
                                break;
                        }
                    }
                }
            }
            InterruptFlags.IRpacketRecieved = 0;
            packet[0].datapacket = 0;
            packet[1].datapacket = 0;
        }
    }

	return;
}

void picinit(){
    TRISA = 0;              // LED output
    TRISBbits.TRISB0 = 1;   // IR input
    TRISCbits.TRISC0 = 0;   // Q8 output
    TRISCbits.TRISC1 = 0;   // Q7 output
    ANSEL = 0;
    ANSELHbits.ANS12 = 0;   // digital input IR

    OPTION_REGbits.INTEDG = 0;  // edge detection INT pin on rising

    T2CONbits.TOUTPS = 0b0101;  // 1:6 postscaler
    T2CONbits.T2CKPS = 0b01;    // 1:4 prescaler
    TMR2             = 0;       // reset TMR2
    
    PORTA = ~(0);
    
    INTCONbits.INTE = 1;    // enable INT interrupts
    INTCONbits.INTF = 0;    // reset INT flag
    INTCONbits.PEIE = 1;    // enable peripheral interrupts
    INTCONbits.GIE  = 1;    // enable global interrupts
}