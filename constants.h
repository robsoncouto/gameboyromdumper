#ifndef CONSTANTS
#define CONSTANTS

#include<avr/io.h>

#define LED 7
#define CLK 7
#define SND 6
#define RST 5
#define WR 4
#define RD 3
#define MREQ 2
#define TX 1
#define RX 0


#define ControlPort PORTD
#define ControlDDR  DDRD

#define DATAOUT PORTC
#define DATAIN PINC
#define DATADDR DDRC

#define ADRH PORTA
#define ADRL PORTB

#define ADRHDDR DDRA
#define ADRLDDR DDRB

#endif
