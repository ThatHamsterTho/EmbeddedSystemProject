/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef SPI_HEADER
#define	SPI_HEADER

#include <xc.h> // include processor files - each processor file is guarded.  

typedef unsigned char bits8;
typedef unsigned int  bits16;

typedef enum __characterSPI__{
	char0,
	char1,
	char2,
	char3,
	char4,
	char5,
	char6,
	char7,
	char8,
	char9,
	charColon,
	charDot,
	charSpace,
	charI,
	charN,
	charP,
	charU,
	charT,
	charV,
	charO,
	charL
} SPIcharacter;
typedef enum __DISPLAY__{
    Display1,
    Display2
} DISPLAY;

const bits8 SPIColumnCode[][5] = {
	{0x3E, 0x51, 0x49, 0x45, 0x3E},
	{0x00, 0x42, 0x7F, 0x40, 0x00},
	{0x62, 0x51, 0x49, 0x49, 0x46},
	{0x22, 0x41, 0x49, 0x49, 0x36},
	{0x18, 0x14, 0x12, 0x7F, 0x10},
	{0x27, 0x45, 0x45, 0x45, 0x39},
	{0x3C, 0x4A, 0x49, 0x49, 0x30},
	{0x01, 0x71, 0x09, 0x05, 0x03},
	{0x36, 0x49, 0x49, 0x49, 0x36},
	{0x06, 0x49, 0x49, 0x29, 0x1E},
	{0x00, 0x36, 0x36, 0x00, 0x00},
	{0x00, 0x30, 0x30, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x41, 0x7F, 0x41, 0x00},
	{0x7F, 0x04, 0x08, 0x10, 0x7F},
	{0x7F, 0x09, 0x09, 0x09, 0x06},
	{0x3F, 0x40, 0x40, 0x40, 0x3F},
	{0x01, 0x01, 0x7F, 0x01, 0x01},
	{0x07, 0x18, 0x60, 0x18, 0x07},
	{0x3E, 0x41, 0x41, 0x41, 0x3E},
	{0x7F, 0x40, 0x40, 0x40, 0x40}
};

char divu10(char n);
void sendByte(char chr);
void sendCharacter(SPIcharacter chr);
void sendInput(char input);
void sendVol(char vol);


#endif