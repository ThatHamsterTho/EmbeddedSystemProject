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
#define IRtime 71		// 422us voor 455Khz, 480us voor 400Khz, TMR2 is on 1:24 so every 1 Ccycle : 6us


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

char SelectedChannel = 1;


// !!!!!!!!!!!!!! FUNCTION DEFINITONS !!!!!!!!!!!!!! //

// adc
// functie om de ADC waarde (0-1023) om te zetten naar een waarde van (0-5)
// hier is jitter al bij weg gewerkt.
char getAreaVal(char prev_ADCvalue);
int getADCval(void);
// rotary
void update_SelectedChannel(void);


void __interrupt() ISR(void){
    if(RBIF && RBIE && !PORTBbits.RB0){
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

    if (T0IF && T0IE) {
        InterruptFlags.RefreshADC = 1; // markeer dat de ADC moet worden herladen
        TMR0IF = 0; // reset flag
    }
} 

void main(void){
    picinit();
    
    while(1){
        if (InterruptFlags.RefreshADC) {
            // call ADC reader
            char LEDS = getAreaVal(prev_ADCvalue);
            if(LEDS != prev_ADCvalue){
                prev_ADCvalue = LEDS;
                if(!InterruptFlags.motorrotating){
                    ADC_draaiwaarde = LEDS;
                }
                if (LEDS != 0) {
                    PORTA = ~(2 * LEDS - 1);
                } else {
                    // LEDS is 0
                    PORTA = ~(0);
                }
            }
            InterruptFlags.RefreshADC = 0; // reset software flag
        }
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
    TRISEbits.TRISE2 = 1;   // ADC input
    
    
    ANSELHbits.ANS11 = 0;   // turn off analog read of RB4
    ANSELHbits.ANS13 = 0;   // turn off analog read of RB5
    
    T2CONbits.TOUTPS = 0b0101;  // 1:6 postscaler
    T2CONbits.T2CKPS = 0b01;    // 1:4 prescaler
    TMR2             = 0;       // reset TMR2
    
    OPTION_REGbits.T0CS = 0;    // internal clock
    OPTION_REGbits.PSA = 0;     // TIMER0 gets prescaler assigned
    OPTION_REGbits.PS  = 0b100; // TIMER0 prescaler 1:256
    
    ADCON0bits.ADCS = 0b00;     // fosc/2
    ADCON0bits.CHS  = 0b0111;   // Channel 7, RE2
    ADCON0bits.ADON = 1;        // ADC module on
    
    ADCON1bits.ADFM = 1;        // right justify
    
    T0IE            = 1;    // enable Timer 0 Overflow Interrupt
    T0IF            = 0;    // reset Timer0 flag
    IOCBbits.IOCB0  = 1;    // enable interrupts on RB0
    IOCBbits.IOCB4  = 1;    // enable interrupts on RB4
    IOCBbits.IOCB5  = 1;    // enable interrupts on RB5
    INTCONbits.RBIE = 1;    // enable interrupt-on-change port B
    INTCONbits.PEIE = 1;    // enable peripheral interrupts
    INTCONbits.GIE  = 1;    // enable global interrupts
}

char getAreaVal(char prev_ADCvalue) {
    // lees potmeter.
    int ADCval = getADCval();

    // wanneer ADCval lager is dan AreaSize EN AreaSize-DEADZONE wordt de 0 waarde teruggegeven.
    // De DEADZONE substractie is om jitter tegen te gaan, ADCval kan lager zijn dan AreaSize maar 
    // niet lager dan de DEADZONE rondom AreaSize
    for (char i = 1; i < AREAS; i++) {
        if ((ADCval < (AreaSize - DEADZONE)) && (ADCval < (AreaSize))) {
            return 0;
        }
        // in between threshold+deadzone and (threshold+1)-deadzone. thus it does not return
        // if the value is in between deadzones.
        if ((ADCval > (AreaSize * i) + DEADZONE) && (ADCval < (AreaSize * (i + 1) - DEADZONE))) {
            return 1 << (i - 1); // i-1 zodat wanneer i = 1, 0 wordt terug gegeven. 0 representeert de eerste LED
        }
    }
    return prev_ADCvalue;
}

int getADCval(void) {
    // klok is op 4MHz en ADCON1 is ingesteld op Fosc/8 dus Tad time is 2.0 us.
    __delay_us(5); // wait for ADC charging cap to settle
    GO = 1; // start ADC conversion.

    // Volle ADC conversie kost 11.5 Tad periods.
    // 1.5 hiervan is voor de periode van Tcy - Tad1.
    // wacht 3us (1.5*Tad) voor periode Tcy-Tad1.
    __delay_us(3);

    // PRECISION correspondeert tot de hoeveelheid Tad perioden die moeten worden uitgevoerd.
    __delay_us(2 * PRECISION);
    GO = 0; // termineer de ADC operatie.

    // Mask het ADC resultaat, zo dat alleen de bits die berekend zijn worden gebruikt,
    // Dit is zodat bits die nooit ingesteld zijn niet mee worden genomen in berekeningen.
    // e.g. precision = 5, dan worden bits 6-11 genegeerd.
    // dit wordt gedaan door een complete mask naar links te shiften met de overgebleven bits. (10-PRECISION).
    int ADRES = (ADRESH << 8) | ADRESL;
    return (ADRES & (0xFFFF << (10 - PRECISION))); // mask bits with precision
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