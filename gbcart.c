#define F_CPU 16000000UL
#define UART_BAUD_RATE 38400
#define ROM 0
#define RAM 1


#include<util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "constants.h"
#include "serial/uart.h"

uint8_t MBC=1,romsize=0,ramsize=0;//size in banks


int (*serialAvailable)(void);
unsigned int (*serialRead)(void);


void programMode(void) {
	DATADDR=0xFF;
}
void readMode(void) {
	DATADDR=0x00;
}

void Go2ADR(uint16_t Adr){
	ADRL=Adr&0x00FF;
	ADRH=Adr>>8;
}
void WriteByte(uint16_t Adr,uint8_t data){
	DATADDR=0xFF;
	Go2ADR(Adr);
	DATAOUT=data;
	ControlPort|=(1<<RD);
	ControlPort&=~(1<<WR);
	_delay_us(5);
	ControlPort|=(1<<WR);
}

uint8_t readByte(uint16_t Adr){
	uint8_t b;
	DATADDR=0x00; //FIXME move to programMode(), so it is executed only once intead of every read
								//then check every call to readByte()
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
	ADRHDDR=0xFF;
	ADRLDDR=0xFF;
	DATADDR=0x00;
	ControlDDR=(1<<WR)|(1<<RD)|(1<<LED)|(1<<RST)|(1<<MREQ)|(1<<TX)|(0<<RX)|(1<<SND);
	ControlPort=(1<<WR)|(1<<RD)|(0<<LED)|(1<<RST)|(1 <<MREQ)|(1<<SND);
	uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU));
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
		case 0x0F:MBC=3;break;
		case 0x10:MBC=3;break;
		case 0x11:MBC=3;break;
		case 0x12:MBC=3;break;
		case 0x13:MBC=3;break;
	}
	//romsize
	data =readByte(0x0148);
	switch (data){
		case 0x00:romsize=2;break;//32kB
		case 0x01:romsize=4;break;
		case 0x02:romsize=8;break;
		case 0x03:romsize=16;break;
		case 0x04:romsize=32;break;
		case 0x05:romsize=64;break;
		case 0x06:romsize=128;break;
		case 0x52:romsize=72;break;
		case 0x53:romsize=80;break;
		case 0x54:romsize=96;break;
	}
	//ramsize
	data =readByte(0x0149);
	switch (data){
		case 0x00:ramsize=0;break;
		case 0x01:ramsize=1;break;
		case 0x02:ramsize=1;break;
		case 0x03:ramsize=4;break;
		case 0x04:ramsize=16;break;
	}
	if (MBC==1)	{
		if (readByte(0x0149)==0x03){
			WriteByte(0x7000,(0x01));//32k ram mode
		}else{
			WriteByte(0x7000,(0x00));//8k  ram mode
		}
	}
	//debug
	MBC=1;
	romsize=32;
	ramsize=1;
	char debug[5];
	uart_puts("MBC:");
	uart_putc(MBC+0x30);
	uart_putc('\n');
	uart_puts("romsize:");
	itoa(romsize,debug,10);
	uart_puts(debug);
	uart_putc('\n');
	uart_puts("ramsize:");
	itoa(ramsize,debug,10);
	uart_puts(debug);
	uart_putc('\n');
	//serialAvailable=uart_available;
	//serialRead=uart_getc;
}


void selectbank(uint8_t location, uint16_t bank){
	if(location==ROM){
		if (MBC==5){
			if (bank>0){
				WriteByte(0x2000,(bank&0x00FF));
				WriteByte(0x3000,(bank>>8));
			}
		}
		if (MBC==1){
			if (ramsize==4){//32k RAM? Mode 2 FIXME
				if (bank>0){
					WriteByte(0x3000,bank);
				}
			}else{//Mode 1 2MB ROM 8kb RAM
				if (bank>0){
					WriteByte(0x3000,bank&0x001F);
					WriteByte(0x4000,bank>>5);
				}
			}
		}
		//Working
		if (MBC==3){
			if (bank>0){
				WriteByte(0x2000,bank);
			}
	  }
	}
	if(location==RAM){
		if (MBC==1){
			if (readByte(0x0149)==0x03){//32k RAM? Mode 2
				WriteByte(0x4000,bank);//select bank
			}
		}
		if (MBC==5){
			WriteByte(0x4000,bank);//select bank
		}
		//working
		if (MBC==3){
			WriteByte(0x4000,bank);//select bank
		}
	}
}

void readBank(uint8_t location, uint16_t bank){
	readMode();
	if (location==ROM){
    if (bank==0){
    	for (uint16_t i=0;i<=0x3FFF;i++){//16k
    		uart_putc(readByte(i));
    	}
    }else{
    	for (uint16_t i=0x4000;i<=0x7FFF;i++){//16k
    		uart_putc(readByte(i));
      }
    }
  }
  if(location==RAM){
    ControlPort&=~(1<<MREQ);
		WriteByte(0x1000,0x0A);//enable RAM writing
    for (uint16_t j=0xA000;j<=0xBFFF;j++){//8k
    	uart_putc(readByte(j));
    }
		WriteByte(0x1000,0x00);//disable RAM writing
    ControlPort|=(1<<MREQ);
  }
}

void readRAM(void){
	if(ramsize==0)return;
	for (uint8_t bank=0;bank<ramsize;bank++){
			selectbank(RAM,bank);//select bank
			readBank(RAM,bank);
	}
}

void readROM(void){
	for(uint8_t bnk=0;bnk<romsize;bnk++){
		selectbank(ROM,bnk);
		readBank(ROM,bnk);
	}
}

void cartInfo(void){

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

void programByte(uint32_t addr, uint8_t data){
		DATADDR=0xFF;

		ControlPort|=(1<<RD);

		selectbank(ROM,1);
		Go2ADR(0x5555);
		DATAOUT=0xAA;
		_delay_us(4);
		ControlPort&=~(1<<SND);
		_delay_us(60);
		ControlPort|=(1<<SND);

		Go2ADR(0x2AAA);
		DATAOUT=0x55;
		_delay_us(4);
		ControlPort&=~(1<<SND);
		_delay_us(60);
		ControlPort|=(1<<SND);

		selectbank(ROM,1);
		Go2ADR(0x5555);
		DATAOUT=0xA0;
		_delay_us(4);
		ControlPort&=~(1<<SND);
		_delay_us(60);
		ControlPort|=(1<<SND);

		if(addr<16384)Go2ADR(addr);//FIXME
		if(addr>16384) {
			selectbank(ROM,addr/16384);
			Go2ADR(0x4000+addr%16384);//FIXME
		}
		DATAOUT=data;
		_delay_us(4);
		ControlPort&=~(1<<SND);
		_delay_us(60);
		ControlPort|=(1<<SND);
}
void eraseflash(void){
		DATADDR=0xFF;

		ControlPort|=(1<<RD);

		selectbank(ROM,1);
		Go2ADR(0x5555);
		DATAOUT=0xAA;
		_delay_us(4);
		ControlPort&=~(1<<SND);
		_delay_us(60);
		ControlPort|=(1<<SND);

		Go2ADR(0x2AAA);
		DATAOUT=0x55;
		_delay_us(4);
		ControlPort&=~(1<<SND);
		_delay_us(60);
		ControlPort|=(1<<SND);

		selectbank(ROM,1);
		Go2ADR(0x5555);
		DATAOUT=0x80;
		_delay_us(4);
		ControlPort&=~(1<<SND);
		_delay_us(60);
		ControlPort|=(1<<SND);

		selectbank(ROM,1);
		Go2ADR(0x5555);
		DATAOUT=0xAA;
		_delay_us(4);
		ControlPort&=~(1<<SND);
		_delay_us(60);
		ControlPort|=(1<<SND);

		Go2ADR(0x2AAA);
		DATAOUT=0x55;
		_delay_us(4);
		ControlPort&=~(1<<SND);
		_delay_us(60);
		ControlPort|=(1<<SND);

		selectbank(ROM,1);
		Go2ADR(0x5555);
		DATAOUT=0x10;
		_delay_us(4);
		ControlPort&=~(1<<SND);
		_delay_us(60);
		ControlPort|=(1<<SND);

		DATADDR=0x00;
		while(~(DATAIN&(1<<7)));
		if(readByte(0xFF)==0xFF) uart_puts("Finished erasing\n");
}

void writeBlock(uint8_t location,uint8_t blockH,uint8_t blockL){
  uint8_t dataBuffer[128];
  uint16_t address=0;
  uint8_t CHK=0,CHKreceived=0;
	uint32_t addr32=0;
	CHK=blockH;
	CHK^=blockL;

  address=blockH;
  address=(address<<8)|blockL;
  address*=128;
	if (location==ROM)addr32=address;

  for(uint8_t i=0;i<128;i++){
        while(uart_available()==0);
        dataBuffer[i]=uart_getc();
        CHK ^= dataBuffer[i];
  }
	if(location==ROM){
		while(uart_available()==0);
	  MBC=uart_getc();
		CHK^=MBC;
	}

	while(uart_available()==0);
  CHKreceived=uart_getc();
  programMode();

  //only program the bytes if the checksum is equal to the one received
  if(location==RAM){
		selectbank(RAM,address/8192);
		WriteByte(0x1000,0x0A);//enable RAM writing
		ControlPort&=~(1<<MREQ);
		_delay_ms(5);
		if(CHKreceived==CHK){
      for (int i = 0; i < 128; i++){
        WriteByte(0xA000+(address%8192),dataBuffer[i]);
				address=address+1;
      }
    uart_putc(CHK);
    }
		ControlPort|=(1<<MREQ);
		WriteByte(0x1000,0x00);

  }
	/*The address received in the real adrees in the memory.
	However, the bank scheme needs this address to be converted as with RAM.
	And there is still the flash programming algorithm to mind.*/
	if(location==ROM){
		//selectbank(ROM,address/16384);
		_delay_ms(5);
		if(CHKreceived==CHK){
      for (int i = 0; i < 128; i++){
        programByte(addr32,dataBuffer[i]);
				addr32=addr32+1;
      }
    uart_putc(CHK);
    }
  }
}
