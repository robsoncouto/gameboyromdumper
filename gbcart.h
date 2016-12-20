#ifndef GBCART_H
#define GBCART_H

void Go2ADR(uint16_t Adr);

uint8_t readByte(uint16_t Adr);

void WriteByte(uint16_t Adr,uint8_t data);

void init(void);

void selectbank(uint16_t bank);
void readBank(uint16_t bank);


void readRAM(void);

void writeRAM(void);

void cartInfo(void);

void writeBlock(unsigned char blockH,unsigned char blockL);

#endif
