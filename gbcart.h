#ifndef GBCART_H
#define GBCART_H
#define ROM 0
#define RAM 1


/*funtion pointers, I dont want to include the uart library in main, so I
 use pointers to call it from gbcart */
extern int (*serialAvailable)(void);
extern unsigned int (*serialRead)(void);

void Go2ADR(uint16_t Adr);

uint8_t readByte(uint16_t Adr);

void WriteByte(uint16_t Adr,uint8_t data);

void init(void);

void selectbank(uint16_t bank);
void readBank(uint16_t bank);

void readROM(void);
void readRAM(void);

void writeRAM(void);

void cartInfo(void);

void writeBlock(uint8_t location, uint8_t blockH,uint8_t blockL);

uint8_t dataAvailable(void);

void programByte(uint32_t, uint8_t);
void eraseflash(void);

#endif
