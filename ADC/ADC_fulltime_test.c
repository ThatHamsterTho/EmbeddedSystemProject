/*
 * File:   ADCtest1.c
 * Author: mcimr
 *
 * Created on November 27, 2019, 5:00 PM
 */

#define _XTAL_FREQ 4000000


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

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
#define PRECISION 3

int AreaSize = ADCMAX / AREAS; // bereken areasize 1 keer.
char prev_value = 0; // houdt bij wat de vorige waarde van de ADC meeting was

// functie om de ADC waarde (0-1023) om te zetten naar een waarde van (0-5)
// hier is jitter al bij weg gewerkt.
char getAreaVal(char prev_value);
int getADCval(void);

void main(void) {
    TRISA = 0; // set PORTA to output
    TRISE = 0b00000100; // set RE2 to input
    ADCON0 = 0b00011101; // ADCS = fosc/2 (00), CHS = channel 7 (0111), GO = OFF (0), ADON = ON (1)
    ADCON1 = 0b10000000; // ADFM = right justified (1), Unimplemented(0), VCFG1 = VSS (0), VCFG0 = VDD (0), Unimplemented*3(0)
    PORTA = 0x0F;
    while(1){
        // call ADC reader
        PORTA = 0x0F ^ !getAreaVal(prev_value);
        
    }
    return;
}

char getAreaVal(char prev_value) {
    // lees potmeter.
    int ADCval = getADCval();
    if(ADCval > 512){
        return 1;
    }
    else{
        return 0;
    }
    return prev_value;
}

int getADCval(void) {
    // klok is op 4MHz en ADCON1 is ingesteld op Fosc/8 dus Tad time is 2.0 us.
    __delay_us(5); // wait for ADC charging cap to settle
    GO = 1; // start ADC conversion.
    while(GO) continue;

    // Mask het ADC resultaat, zo dat alleen de bits die berekend zijn worden gebruikt,
    // Dit is zodat bits die nooit ingesteld zijn niet mee worden genomen in berekeningen.
    // e.g. precision = 5, dan worden bits 6-11 genegeerd.
    // dit wordt gedaan door een complete mask naar links te shiften met de overgebleven bits. (10-PRECISION).
    int ADRES = (ADRESH << 8) | ADRESL;
    return (ADRES & (0xFFFF << (10 - PRECISION))); // mask bits with precision
}
