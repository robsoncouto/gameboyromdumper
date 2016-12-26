/*
 * GBCartReader.c
 *
 * Created: 29/10/2015 20:00:15
 *  Author: Roco
 */
#define F_CPU 16000000UL
#define UART_BAUD_RATE 38400


#include<avr/io.h>
#include<util/delay.h>
#include "serial/uart.h"
#include "gbcart.h"

uint8_t inByte=0,num=0,addrH=0,addrL=0;
uint16_t address;


int main(void){
	init();
	_delay_ms(2000);
	while(1){
		PORTB^=(1<<7);
		//uart_puts("Restart complete");
		if (uart_available()>2){
			while (uart_available()>0){
				inByte=uart_getc();
				if (inByte=='a'){
					inByte=uart_getc();
					if (inByte=='b'){
						inByte=uart_getc();
						switch (inByte){
							case 'c':
								readROM();
							break;
							case 'd':
								//prints the rom header information
								init();
								cartInfo();
							break;
							case 'r':
								// just runs the init() function again, this is important to set the right MBC
								init();
								uart_puts("Restart complete");
							break;
							case 'y':
								//Prints ram through serial
								readRAM();
							break;
							case 'k':
								//Not done yet .... work in progress
								while(uart_available()<2);
          			addrH=uart_getc();
								addrL=uart_getc();
          			writeBlock(RAM,addrH,addrL);
							break;
							case 'b':
								while(uart_available()<2);
								addrH=uart_getc();
								addrL=uart_getc();
								writeBlock(ROM,addrH,addrL);
							break;
							//puppet mode, work in progress
							case 'f':
							//read bytes from location
								while(serialAvailable()<3);
								address = serialRead();
								address |= (serialRead()<<8);
								num=serialRead();
								//uart_puts("Byte: \n");
								while (num>0)
								{
									uart_putc(readByte(address++));
									num--;
								}
								uart_putc('\n');
							case 'g':
								while(serialAvailable()<3);
								address = serialRead();
								address |= (serialRead()<<8);
								num=serialRead();
								//uart_puts("Byte: \n");
								WriteByte(address,num);
								uart_putc(readByte(address));
								uart_putc('\n');
							break;
						}
					}
				}
			}

		}
	}
}
