/*
 * File:   TimerTest.c
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

// TMR2 buffer
#define REFRESHRATE 49  
// 1 / (Fosc/4) / (Pre-scale * Post-scaler) = 256us
// (256 * TMR2_max_val) - (1/80 * 10^6) = 53036 us
// 53036 / 256 = 207
// 256 - 207 = 49


struct _InterruptFlags {
    unsigned char RefreshADC : 1;
    unsigned char : 7;
} InterruptFlags;

void __interrupt() ROT_ISR(void) {
    if (TMR2IF && TMR2IE) {
        TMR2 = REFRESHRATE;
        InterruptFlags.RefreshADC = 1; // markeer dat de ADC moet worden herladen
        TMR2IF = 0; // reset flag
    }
}

void main(void) {
    TRISA = 0; // set PORTA to output
    TRISE = 0b00000100; // set RE2 to input
    ADCON0 = 0b00011101; // ADCS = fosc/2 (00), CHS = channel 7 (0111), GO = OFF (0), ADON = ON (1)
    ADCON1 = 0b10000000; // ADFM = right justified (1), Unimplemented(0), VCFG1 = VSS (0), VCFG0 = VDD (0), Unimplemented*3(0)

    T2CON = 0b01111011; // Prescaler: 1:16<1:0>, post-scaler 1:16<3:6> together: 1:256

    GIE = 1; // enable global interrupts
    PEIE = 1; // enable peripheral interrupts
    TMR2IE = 1; // enable Timer 2 Overflow Interrupt

    TMR2 = REFRESHRATE; // zet buffer
    TMR2ON = 1; // Zet timer 2 aan

    while (1) {
        if (InterruptFlags.RefreshADC) {
            // LED should be ON or OFF, no flickering
            PORTA ^= 1;
            InterruptFlags.RefreshADC = 0; // reset software flag
        }
    }

    return;
}
