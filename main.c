/*
 * GBCartReader.c
 *
 * Created: 29/10/2015 20:00:15
 *  Author: Roco
 */
#define F_CPU 16000000UL

#include<avr/io.h>
#include<util/delay.h>
#include <avr/interrupt.h>
#include "serial/uart.h"
#include "constants.h"
#include "gbcart.h"

uint8_t a=0,num=0;
uint16_t adress;

int main(void){
	init();
	_delay_ms(2000);
	while(1){
		if (uart_available()>2){
			while (uart_available()>0){
				a=uart_getc();
				if (a=='a'){
					a=uart_getc();
					if (a=='b'){
						a=uart_getc();
						switch (a){
							case 'c':
								for(uint16_t  i=0;i<1024;i++){
									readBank(i);
								}
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
								//Prints the ram through serial
								readRAM();
							break;
							case 'k':
								//Not done yet .... work in progress
								writeRAM();
							break;
							case 'e':
							break;
							//puppet mode, work in progress
							case 'f':
							//read bytes from location
								while(uart_available()<3);
								adress = uart_getc();
								adress |= (uart_getc()<<8);
								num=uart_getc();
								//uart_puts("Byte: \n");
								while (num>0)
								{
									uart_putc(readByte(adress++));
									num--;
								}
								uart_putc('\n');
							case 'g':
								while(uart_available()<3);
								adress = uart_getc();
								adress |= (uart_getc()<<8);
								num=uart_getc();
								//uart_puts("Byte: \n");
								WriteByte(adress,num);
								uart_putc(readByte(adress));
								uart_putc('\n');
							break;
						}
					}
				}
			}

		}
	}
}
