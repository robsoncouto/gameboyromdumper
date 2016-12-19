#define F_CPU 16000000UL
#include<avr/io.h>
#include<util/delay.h>
#include <avr/interrupt.h>
#include "constants.h"
#include "serial/uart.h"
#define UART_BAUD_RATE      38400


uint8_t MBC=0;

void programMode(void) {
	DDRC=0xFF;
}
void readMode(void) {
	DDRC=0x00;
}

void Go2ADR(uint16_t Adr){
	ADRL=Adr&0x00FF;
	ADRH=Adr>>8;
}
void WriteByte(uint16_t Adr,uint8_t data){

	DDRC=0xFF;
	Go2ADR(Adr);
	DATAOUT=data;
	ControlPort|=(1<<RD);
	ControlPort&=~(1<<WR);
	_delay_us(2);
	ControlPort|=(1<<WR);

}
uint8_t readByte(uint16_t Adr){

	uint8_t b;
	//DATAOUT=0xFF;////////////////////////////////////////Noobada
	DDRC=0x00;
	Go2ADR(Adr);
	//ControlPort&=~(1<<MREQ);
	ControlPort|=(1<<WR);
	ControlPort&=~(1<<RD);
	_delay_us(2);
	b=DATAIN;
	ControlPort|=(1<<RD);
	//ControlPort|=(1<<MREQ);
	return b;
}

void init(void){
	DDRA=0xFF;
	DDRB=0xFF;
	DDRC=0x00;
	ControlDDR=(1<<WR)|(1<<RD)|(1<<LED)|(1<<RST)|(1<<MREQ);
	ControlPort=(1<<WR)|(1<<RD)|(0<<LED)|(1<<RST)|(1 <<MREQ);
	uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );
	sei();
	uint8_t data =readByte(0x0147);
	switch (data){
		case 0x01:MBC=1;break;
		case 0x02:MBC=1;break;
		case 0x03:MBC=1;break;
		case 0x19:MBC=5;break;
		case 0x1A:MBC=5;break;
		case 0x1B:MBC=5;break;
		case 0x1C:MBC=5;break;
		case 0x1D:MBC=5;break;
		case 0x1E:MBC=5;break;
		case 0xFC:MBC=3;break;
	}
	if (MBC==1)	{
		if (readByte(0x0149)==0x03){
			WriteByte(0x7000,(0x01));//32k ram mode
		}else{
			WriteByte(0x7000,(0x00));//8k  ram mode
		}
	}


	//debug
	uart_puts("MBC:");
	uart_putc(MBC+0x30);
	uart_putc('\n');

}
void readBank(uint16_t bank ){
	if (MBC==5){
		if (bank==0)
		{
			for (uint16_t i=0;i<=0x3FFF;i++){//16k
				uart_putc(readByte(i));
			}
		}else{
			WriteByte(0x2000,(bank&0x00FF));
			WriteByte(0x3000,(bank>>8));
			for (uint16_t i=0x4000;i<=0x7FFF;i++){//16k
				uart_putc(readByte(i));
			}
		}
	}
	if (MBC==1){
		if (readByte(0x0149)==0x03){//32k RAM? Mode 2
			if (bank==0){
				for (uint16_t i=0;i<=16383;i++){//16k
					uart_putc(readByte(i));
				}
			}else{
				WriteByte(0x3000,bank);
				for (uint16_t i=0x4000;i<=0x7FFF;i++){//16k
					uart_putc(readByte(i));
				}
			}
		}else{//Mode 1 2MB ROM 8kb RAM
			if (bank==0){
				for (uint16_t i=0x000;i<=0x3FFF;i++){//16k
					uart_putc(readByte(i));
				}
			}else{
				WriteByte(0x3000,bank&0x001F);
				WriteByte(0x4000,bank>>5);
				//uart_putc(bank&0x00FF);
				for (uint16_t i=16384;i<=32767;i++){//16k
					uart_putc(readByte(i));
				}
			}
		}
	}
	if (MBC==3){
		if (bank==0){
			for (uint16_t i=0;i<=0x3FFF;i++){//16k
				uart_putc(readByte(i));
			}
			}else{
			WriteByte(0x2000,(bank&0x00FF));
			for (uint16_t i=0x4000;i<=0x7FFF;i++){//16k
				uart_putc(readByte(i));
			}
		}
	}
}
void readRAM(void){
	//enable the RAM chip
	if (MBC==1){
		if (readByte(0x0149)==0x03){//32k RAM? Mode 2
			WriteByte(0x1000,0x0A);//enable RAM writing
			for (uint8_t i=0;i<4;i++){
				WriteByte(0x4000,i);//select bank
				ControlPort&=~(1<<MREQ);
				for (uint16_t j=0xA000;j<=0xBFFF;j++){
					uart_putc(readByte(j));
				}
				ControlPort|=(1<<MREQ);
			}
			WriteByte(0x1000,0x00);//disable RAM writing
		}else{//Mode 1 2MB ROM 8kb RAM
			WriteByte(0x1000,0x0A);
			ControlPort&=~(1<<MREQ);
			for (uint16_t j=0xA000;j<=0xBFFF;j++){
				uart_putc(readByte(j));
			}
			ControlPort|=(1<<MREQ);
		}
		WriteByte(0x1000,0x00);//disable RAM writing
	}
	if (MBC==5){
		WriteByte(0x0000, 0x0A);//enable RAM writing
		for (uint8_t i=0;i<32;i++){
			WriteByte(0x4000,i);//select bank
			ControlPort&=~(1<<MREQ);
			for (uint16_t j=0xA000;j<=0xBFFF;j++){
				uart_putc(readByte(j));
			}
			ControlPort|=(1<<MREQ);
		}
		WriteByte(0x1000,0x00);//disable RAM writing
	}
	if (MBC==3){
		//WriteByte(0x0000, 0x0A);//enable RAM writing
		for (uint8_t i=0;i<0x0F;i++){
			WriteByte(0x4000,i);//select bank
			ControlPort&=~(1<<MREQ);//inside the loop
			for (uint16_t j=0xA000;j<=0xBFFF;j++){
				uart_putc(readByte(j));
			}
			ControlPort|=(1<<MREQ);
		}
		//WriteByte(0x0000,0x00);//disable RAM writing
	}
}

void cartInfo(void){

	//WriteByte(0x7000,0x00);//8k ram mode
	//WriteByte(0x3000,0x01);
	//WriteByte(0x4000,0x00);

	//cartridge connection check
	if (readByte(0x0104)==0xCE){
		if (readByte(0x0105)==0xED){
			if (readByte(0x0106)==0x66){
				if (readByte(0x0107)==0x66){uart_puts("Cartridge Connected \n");}
				if (readByte(0x0104)==readByte(0x4104)){uart_puts("But it is a faulty  connection ,please try to insert again \n");}
			}
		}
	}else{uart_puts("Please check the cartridge connection\n");return;}
	//title of the game
	uart_puts("Title of the game in ASCII:");
	for (int i=0x0134;i<=0x0142;i++)
	{
		if(readByte(i)<128) {uart_putc(readByte(i));}//some fake cartridges using values outside ASCII range.
	}uart_putc('\n');

	uart_puts("Gameboy/Gameboy Color?");
	uint8_t data =readByte(0x0143);
	if (data==0x80){
		uart_puts("	Cartridge with color function\n");
	}else{
		uart_puts("	Cartridge with no color function\n");
	}
	uart_puts("Super Gameboy functions?");
	data =readByte(0x0146);
	if (data==0x03){
		uart_puts("	Cartridge with SGB functions\n");
	}else{
		uart_puts("	Cartridge without SGB functions\n");
	}
	uart_puts("Cartridge Type:");
	data =readByte(0x0147);
	switch (data){
		case 0x00:uart_puts("	ROM ONLY\n");break;
		case 0x01:uart_puts("	ROM+MBC1\n");break;
		case 0x02:uart_puts("	ROM+MBC1+RAM\n");break;
		case 0x03:uart_puts("	ROM+MBC1+RAM+BATTERY\n");break;
		case 0x05:uart_puts("	ROM+MBC2\n");break;
		case 0x06:uart_puts("	ROM+MBC2+BATTERY\n");break;
		case 0x08:uart_puts("	ROM+RAM\n");break;
		case 0x09:uart_puts("	ROM+RAM+BATTERY\n");break;
		case 0x0B:uart_puts("	ROM+MMM01\n");break;
		case 0x0C:uart_puts("	ROM+MMM01+SRAM\n");break;
		case 0x0D:uart_puts("	ROM+MMM01+SRAM+BATT\n");break;
		case 0x0F:uart_puts("	ROM+MBC3+TIMER+BATT\n");break;
		case 0x10:uart_puts("	ROM+MBC3+TIMER+RAM+BATT\n");break;
		case 0x11:uart_puts("	ROM+MBC3\n");break;
		case 0x12:uart_puts("	ROM+MBC3+RAM\n");break;
		case 0x13:uart_puts("	ROM+MBC3+RAM+BATT\n");break;
		case 0x19:uart_puts("	ROM+MBC5\n");break;
		case 0x1A:uart_puts("	ROM+MBC5+RAM\n");break;
		case 0x1B:uart_puts("	ROM+MBC5+RAM+BATT\n");break;
		case 0x1C:uart_puts("	ROM+MBC5+RUMBLE\n");break;
		case 0x1D:uart_puts("	ROM+MBC5+RUMBLE+SRAM\n");break;
		case 0x1E:uart_puts("	ROM+MBC5+RUMBLE+SRAM+BATT\n");break;
		case 0x1F:uart_puts("	Pocket Camera\n");break;
		case 0xFC:uart_puts("	Game Boy Camera\n");break;
		case 0xFD:uart_puts("	Bandai TAMA5\n");break;
		case 0xFE:uart_puts("	Hudson HuC-3\n");break;
		case 0xFF:uart_puts("	Hudson HuC-1\n");break;
	}
	uart_puts("ROM size:");
	data =readByte(0x0148);
	switch (data){
		case 0x00:uart_puts("	256Kbit = 32KByte = 2 banks \n");break;
		case 0x01:uart_puts("	512Kbit = 64KByte = 4 banks \n");break;
		case 0x02:uart_puts("	1Mbit = 128KByte = 8 banks \n");break;
		case 0x03:uart_puts("	2Mbit = 256KByte = 16 banks \n");break;
		case 0x04:uart_puts("	4Mbit = 512KByte = 32 banks \n");break;
		case 0x05:uart_puts("	8Mbit = 1MByte = 64 banks \n");break;
		case 0x06:uart_puts("	16Mbit = 2MByte = 128 banks \n");break;
		case 0x52:uart_puts("	9Mbit = 1.1MByte = 72 banks \n");break;
		case 0x53:uart_puts("	10Mbit = 1.2MByte = 80 banks \n");break;
		case 0x54:uart_puts("	12Mbit = 1.5MByte = 96 banks \n");break;
	}
	uart_puts("RAM size:");
	data =readByte(0x0149);
	switch (data){
		case 0x00:uart_puts("	None");break;
		case 0x01:uart_puts("	16kBit = 2kB = 1 bank \n");break;
		case 0x02:uart_puts("	64kBit = 8kB = 1 bank \n");break;
		case 0x03:uart_puts("	256kBit = 32kB = 4 banks \n");break;
		case 0x04:uart_puts("	1MBit =128kB =16 banks \n");break;
	}
}

void writeBank(uint8_t bank){
	uint16_t count=0;
	//uint8_t data=0;
	while(count<0x4FFF){
		//data=uart_getc();
		count++;

	}
}
void writeRAM(void){
	ControlPort&=~(1<<MREQ);
	if (MBC==5){
		WriteByte(0x1000,0x0A);//enable RAM writing
		for (uint8_t i=0;i<16;i++){
			WriteByte(0x4000,i);//select bank
			for (uint16_t j=0xA000;j<=0xBFFF;j++){
				while (uart_available()==0){};
				WriteByte(j,uart_getc());
			}

		}
		WriteByte(0x1000,0x00);//disable RAM writing
	}
	ControlPort|=(1<<MREQ);
}

void writeBlock(unsigned char blockH,unsigned char blockL){
  uint8_t dataBuffer[128];
  uint16_t address=0;
  uint8_t CHK=blockH,CHKreceived;
  CHK^=blockL;

  address=blockH;
  address=(address<<8)|blockL;
  address*=128;

  for(uint8_t i=0;i<128;i++){
        while(uart_available()==0);
        dataBuffer[i]=uart_getc();
        CHK ^= dataBuffer[i];
  }
  while(uart_available()==0);
  CHKreceived=uart_getc();
  programMode();
  //only program the bytes if the checksum is equal to the one received
  if(CHKreceived==CHK){
    for (int i = 0; i < 128; i++){
      WriteByte(address,dataBuffer[i]);
    }
  uart_putc(CHK);
  }

}
