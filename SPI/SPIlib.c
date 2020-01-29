/*
 * File:   SPIlib.c
 * Author: mcimr
 *
 * Created on January 4, 2020, 2:08 PM
 */

#define _XTAL_FREQ 4000000

#include <xc.h>
#include "SPIlib.h"

char divu10(char n) {
    char i = 0;
    for(; n > 9; i++){
        n = (char)(n - 10);
    }
    return i;
}

void SPIinit(void){
    TRISC = TRISC & 0b00000011; // input display 1
    TRISD = TRISD & 0b00101011; // input display 2
    
    // set SPI mode
    SSPSTATbits.CKE = 1;
	SSPCONbits.CKP  = 0;
	SSPCONbits.SSPM = 0000;	// SPI master mode Fosc/4
	SSPCONbits.SSPEN = 1;   // enable SPI module

    PORTDbits.RD2 = 0;      // reset J2
    PORTCbits.RC2 = 0;      // reset J1
    PORTCbits.RC2 = 1;      // reset J1
    PORTDbits.RD2 = 1;      // reset J2
    
    PORTDbits.RD7 = 1;      // select control word
    PORTCbits.RC7 = 1;      // select control word
    
    PORTDbits.RD4 = 0;      // select display 2
    PORTCbits.RC4 = 0;      // select display 1
    
    sendByte(0b01001100);   // control word 0
    /*
        d6  : set normal operation
        d5-4: set peak current to default, 9.3mA
        d3-0: set brightness control, 47%
    */
    sendByte(0b10000000);  // control word 1
    /*
        d6-d2: not used
        d1   : oscillator freq 1
        d2   : Dout holds contents of bit D7, default
    */
    
    PORTDbits.RD4 = 1;      // deselect display 2
    PORTCbits.RC4 = 1;      // deselect display 1

    PORTCbits.RC7 = 0;      // Select Display1
    PORTDbits.RD7 = 0;      // Select Display2
}

void sendByte(char chr){
    
    SSPBUF = chr;
    while (!PIR1bits.SSPIF);
    __delay_us(10); // wait for SSPBUF to set to SSPSR and then 8 bit shifts.
    
    PIR1bits.SSPIF = 0;
    SSPCONbits.WCOL = 0;
}

void sendCharacter(SPIcharacter chr){
    for(int i = 0; i < 5; i++){
        SSPBUF = SPIColumnCode[chr][i];
        while (!PIR1bits.SSPIF);
        __delay_us(1);
    }
    PIR1bits.SSPIF = 0;
    SSPCONbits.WCOL = 0;
}

void sendInput(char input){
    PORTDbits.RD4 = 0; // select display 2
    __delay_us(10);    // wait for reaction

    for(char ch = charI; ch < charV; ch++){
        sendCharacter(ch);
    }
    
    sendCharacter(charDot);
    sendCharacter(charSpace);
    sendCharacter((SPIcharacter)input);
    
    __delay_us(10); // wait for reaction
    PORTDbits.RD4 = 1; // deselect display 2
}

void sendVol(char vol){    
    PORTCbits.RC4 = 0;  // select display 1
    __delay_us(10); // wait for reaction
    
    for(char ch = charV; ch < charL+1; ch++){
        sendCharacter(ch);
    }
    sendCharacter(charDot);
    sendCharacter(charSpace);
    sendCharacter(charSpace);
    if(vol > 9){
        char div10 = divu10(vol);
        sendCharacter(div10);
        sendCharacter(vol - ((div10 << 3) + (div10 << 1)));
    }
    else{
        sendCharacter(charSpace);
        sendCharacter(vol);
    }
    
    __delay_us(10); // wait for reaction
    PORTCbits.RC4 = 1;  // deselect display 1
}
