#include <iostream>
#include <fstream>
#include <stdint.h>
#include <cstring>
#include <cstdio>

using namespace std;

#define PROM_WIDTH 32
#define PROM_DEPTH (0x8000 / (PROM_WIDTH / 8))
#define CROM_WIDTH 8
#define CROM_DEPTH (0x2000 / (CROM_WIDTH / 8))

uint8_t PROM[0x8000];
uint8_t CROM[0x2000];

void load_ROM(ifstream *rom);

int main(int argc, char* argv[]){
    if(argc == 1){
        cout<< "usage: nes [ROM]" << endl;
        return -1;
    }
    ifstream ROM(argv[1],ios::in|ios::binary);
    if(!ROM){
        cout << "cannot open ROM file." << endl;
        return 1;
    }

    load_ROM(&ROM);
    ROM.close();

    ofstream prom_mif("PROM.mif",ios::out);

    char str[64];
    sprintf(str, "DEPTH = %d;\n",PROM_DEPTH);
    prom_mif << str;
    sprintf(str, "WIDTH = %d;\n\n", PROM_WIDTH);
    prom_mif << str;
    sprintf(str, "ADDRESS_RADIX = HEX;\n");
    prom_mif << str;
    sprintf(str, "DATA_RADIX = HEX;\n\n");
    prom_mif << str;
    sprintf(str, "CONTENT\n");
    prom_mif << str;
    sprintf(str, "BEGIN\n\n");
    prom_mif << str;

    uint32_t data = 0;
    for(uint32_t addr = 0x0000; addr <= 0x1FFF; addr++){
        for(int shift = 0; shift < 4; shift++){
            //printf("%04x:%02x\n", addr*4+shift, PROM[addr*4+shift]);
            data |= (uint32_t)PROM[addr*4+shift] << (shift*8);
        }
        sprintf(str, "%04x : %08x;\n", addr, data);
        prom_mif << str;
        data = 0;
    }

    sprintf(str, "\nEND;\n");
    prom_mif << str;
    prom_mif.close();


    ofstream crom_mif("CROM.mif",ios::out);
    sprintf(str, "DEPTH = %d;\n",CROM_DEPTH);
    crom_mif << str;
    sprintf(str, "WIDTH = %d;\n\n", CROM_WIDTH);
    crom_mif << str;
    sprintf(str, "ADDRESS_RADIX = HEX;\n");
    crom_mif << str;
    sprintf(str, "DATA_RADIX = HEX;\n\n");
    crom_mif << str;
    sprintf(str, "CONTENT\n");
    crom_mif << str;
    sprintf(str, "BEGIN\n\n");
    crom_mif << str;

    for(uint32_t addr = 0x00; addr <= 0x1FFF; addr++){
        data = CROM[addr];
        sprintf(str, "%04x : %02x;\n", addr, data);
        crom_mif << str;
        data = 0;
    }

    sprintf(str, "\nEND;\n");
    crom_mif << str;
    crom_mif.close();

    printf("nmi vec : %04x\n", (uint16_t)PROM[0x7FFB] << 8 | PROM[0x7FFA]);
    printf("res vec : %04x\n", (uint16_t)PROM[0x7FFD] << 8 | PROM[0x7FFC]);
    printf("irq vec : %04x\n", (uint16_t)PROM[0x7FFF] << 8 | PROM[0x7FFE]);
}


void load_ROM(ifstream *rom){
    uint32_t magic;
    uint8_t prom_size;
    uint8_t crom_size;
    int psize;
    int csize;
    rom->read((char*)&magic, sizeof(uint32_t));
    if(magic != 0x1A53454E){
        cout << "This is not iNES format file." << endl;
    }
    rom->read((char*)&prom_size, sizeof(uint8_t));
    rom->read((char*)&crom_size, sizeof(uint8_t));

    psize = prom_size * 0x4000;
    csize = crom_size * 0x2000;

    rom->seekg(16,ios_base::beg);
    uint8_t *prom_ptr = (prom_size == 1) ? PROM + 0x4000 : PROM;
    for(int i=0;i<psize;i++)
        rom->read((char*)(prom_ptr+i), sizeof(uint8_t));
    
    for(int i=0;i<csize;i++)
        rom->read((char*)(CROM+i), sizeof(uint8_t));
}

