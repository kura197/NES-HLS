#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
//#include "nes.h"
#include "ram.h"
#include "cpu.h"
#include "ppu.h"
#include "HLS/hls.h"
#include "HLS/ac_int.h"
#include "HLS/stdio.h"

using namespace std;


void load_ROM(ifstream *rom, uint8_t* PROM, uint8_t* CROM);
void set_vram(uint8_t* COLOR, uint8_t* VRAM);
void make_bmp(uint8_t* VRAM, int index);

uint8_t _PROM[0x8000];
uint8_t _CROM[0x2000];
void load_test_ROM(ifstream *rom);

void test_load(uint8_t* WRAM, uint8_t* PPU_RAM){
    //for(uint32_t addr = 0x8000; addr <= 0xFFFF; addr++){
        //WRAM[addr] = PROM[addr - 0x8000];
    for(uint32_t addr = 0x0000; addr <= 0x7FFF; addr++){
        WRAM[addr] = _PROM[addr];
    }

    for(uint32_t addr = 0x00; addr <= 0x1FFF; addr++){
        PPU_RAM[addr] = _CROM[addr];
    }
}

//hls_avalon_slave_component
//struct SCROLL exec_cpu(
//              //ihc::mm_master<uint8_t, ihc::aspace<5>, ihc::awidth<15>, ihc::dwidth<8*8>, ihc::align<8> >& PROM,
//              ihc::mm_master<uint8_t, ihc::aspace<1>, ihc::awidth<16>, ihc::dwidth<8*8>, ihc::align<8> >& WRAM,
//              ihc::mm_master<uint8_t, ihc::aspace<2>, ihc::awidth<14>, ihc::dwidth<8> >& PPU_RAM,
//              ihc::mm_master<uint8_t, ihc::aspace<3>, ihc::awidth<8>, ihc::dwidth<8> >& SP_RAM,
//              //hls_avalon_slave_memory_argument(0x8000*sizeof(uint8_t)) uint8_t *WRAM, 
//              //hls_avalon_slave_memory_argument(0x4000*sizeof(uint8_t)) uint8_t *PPU_RAM, 
//              //hls_avalon_slave_memory_argument(0x100*sizeof(uint8_t)) uint8_t *SP_RAM, 
//              bool res, bool nmi){
//    static CPU cpu;
//    //hls_init_on_powerup static uint8_t WRAM[0x10000];
//    struct SCROLL scr;
//    if(res) cpu.set_reset();
//    if(nmi) cpu.set_nmi();
//    //scr = cpu.exec(WRAM, PPU_RAM, SP_RAM);
//    return scr;
//}

//bool exec_ppu(ihc::mm_master<uint8_t, ihc::aspace<1>, ihc::awidth<16>, ihc::dwidth<8> >& WRAM,
//bool exec_ppu(ihc::mm_master<uint8_t, ihc::aspace<1>, ihc::awidth<16>, ihc::dwidth<8*8>, ihc::align<8> >& WRAM,
//              //ihc::mm_master<uint8_t, ihc::aspace<1>, ihc::awidth<15>, ihc::dwidth<8*8>, ihc::align<8> >& WRAM,
//              ihc::mm_master<uint8_t, ihc::aspace<2>, ihc::awidth<14>, ihc::dwidth<8> >& PPU_RAM,
//              ihc::mm_master<uint8_t, ihc::aspace<3>, ihc::awidth<8>, ihc::dwidth<8> >& SP_RAM,
//              ihc::mm_master<uint8_t, ihc::aspace<4>, ihc::awidth<16>, ihc::dwidth<8> >& VRAM,
//              uint8_t BG_offset_x, uint8_t BG_offset_y)
//{
//    static PPU ppu;
//    bool nmi = ppu.render(WRAM, PPU_RAM, SP_RAM, VRAM, BG_offset_x, BG_offset_y);
//    return nmi;
//}

hls_avalon_slave_component
component void exec_nes(
            //ihc::mm_master<uint8_t, ihc::aspace<4>, ihc::awidth<16>, ihc::dwidth<8> >& VRAM,
            hls_avalon_slave_memory_argument(256*240*sizeof(uint8_t)) uint8_t *VRAM, 
            //hls_avalon_slave_memory_argument(256*240*sizeof(uint6)) uint8_t *VRAM, 
            uint8_t key, bool res
        ){
    static CPU cpu;
    static PPU ppu;
    hls_init_on_powerup static uint8_t PROM[0x8000];
    hls_init_on_powerup static uint8_t PPU_RAM[0x4000];
    hls_init_on_powerup static uint8_t WRAM[0x800];
    hls_init_on_powerup static uint8_t SP_RAM[0x100];
    //hls_register uint8_t Stack[0x40];
    static uint8_t Stack[0x100];

    //static bool init;
    //if(!init) test_load(PROM, PPU_RAM);
    //init = true;

    static struct SPREG spreg;
    static bool nmi;

    //if(res) cpu.set_reset();
    //if(nmi) cpu.set_nmi();
    
    uint8_t irq_num;
    if(nmi) irq_num = NMI;
    else if(res) irq_num = RESET;
    if(res | nmi) cpu.exec_irq(irq_num, WRAM, PPU_RAM, SP_RAM, PROM, Stack);

    cpu.load_key(key);
    for(int c = 0; c < 40; c++) {
        cpu.exec(WRAM, PPU_RAM, SP_RAM, PROM, &spreg, Stack);
    }
    //printf("sphit:%d\n", spreg.SPhit);
    nmi = ppu.render(PPU_RAM, SP_RAM, VRAM, &spreg);
}

//component int test(int arg){
//    int ret;
//    switch(arg){
//        case 0:
//            ret = 1;
//            break;
//        default:
//            ret = 0;
//            printf("Nooo\n");
//            break;
//    }
//    return ret;
//}

int main(int argc, char* argv[]){
    if(argc == 1){
        cout<< "usage: nes [ROM] [-k KEY] [-g] [-s] [-p port] [-f frame] [-b] [-i interval] [-l]" << endl;
        return -1;
    }
    ifstream ROM(argv[1],ios::in|ios::binary);
    if(!ROM){
        cout << "cannot open ROM file." << endl;
        return 1;
    }
    bool en_key = false;
    bool en_gray = false;
    bool en_socket = false;
    bool en_bmp = false;
    bool en_log = false;
    uint16_t port = 7000;
    int frame = 1000;
    int interval = 8;
    char key_file[64];
    int opt;
    while((opt = getopt(argc, argv, "k:gsp:f:bi:l")) != -1){
        switch(opt){
            case 'k':
                en_key = true;
                strcpy(key_file, optarg);
                break;
            case 'g':
                en_gray = true;
                break;
            case 's':
                en_socket = true;
                break;
            case 'p':
                en_socket = true;
                port = atoi(optarg);
                break;
            case 'f':
                frame = atoi(optarg);
                break;
            case 'b':
                en_bmp = true;
                break;
            case 'i':
                interval = atoi(optarg);
                break;
            case 'l':
                en_log = true;
                break;
            default:
                cout<< "usage: nes [ROM] [-k KEY] [-g] [-s] [-p port] [-f frame] [-b] [-i interval] [-l]" << endl;
                return -1;
                break;
        }
    }

    ifstream KEY;
    if(en_key){
        KEY.open(key_file);
        if(!KEY){
            cout << "cannot open KEY file." << endl;
            return 1;
        }
    }

    //NES nes;
    //nes.load_ROM(&ROM);
    uint8_t COLOR[256*240];
    uint8_t VRAM[3*256*240];
    uint8_t WRAM[0x10000];
    uint8_t PROM[0x8000];
    uint8_t PPU_RAM[0x4000];
    uint8_t SP_RAM[0x100];
    //load_ROM(&ROM, WRAM, PPU_RAM);
    load_test_ROM(&ROM);
    ROM.close();
    //ihc::mm_master<uint8_t, ihc::aspace<1>, ihc::awidth<16>, ihc::dwidth<8> > mm_WRAM(WRAM, sizeof(uint8_t)*0x10000);
    ihc::mm_master<uint8_t, ihc::aspace<1>, ihc::awidth<16>, ihc::dwidth<8*8>, ihc::align<8> > mm_WRAM(WRAM, sizeof(uint8_t)*0x10000);
    ihc::mm_master<uint8_t, ihc::aspace<2>, ihc::awidth<14>, ihc::dwidth<8> > mm_PPU_RAM(PPU_RAM, sizeof(uint8_t)*0x4000);
    ihc::mm_master<uint8_t, ihc::aspace<3>, ihc::awidth<8>, ihc::dwidth<8> > mm_SP_RAM(SP_RAM, sizeof(uint8_t)*0x100);
    ihc::mm_master<uint8_t, ihc::aspace<4>, ihc::awidth<16>, ihc::dwidth<8> > mm_COLOR(COLOR, sizeof(uint8_t)*(256*240));
    ihc::mm_master<uint8_t, ihc::aspace<5>, ihc::awidth<15>, ihc::dwidth<8*8>, ihc::align<8> > mm_PROM(PROM, sizeof(uint8_t)*0x8000);

    int index = 0;
    int f = 0;
    uint8_t key = 0;
    struct SPREG spreg;
    bool nmi = false;
    //scr = exec_cpu(mm_WRAM, mm_PPU_RAM, mm_SP_RAM, true, false);
    //scr = exec_cpu(WRAM, PPU_RAM, SP_RAM, true, false);
    exec_nes(COLOR, 0x0, true);
    while(f++ < frame){
        for(int l = 0; l < 256; l++){
            exec_nes(COLOR, 0x0, false);
        }
        //for(int l = 0; l < 256; l++){
        //    for(int c = 0; c < 40; c++) {
        //        scr = exec_cpu(mm_WRAM, mm_PPU_RAM, mm_SP_RAM, false, nmi);
        //        //scr = exec_cpu(WRAM, PPU_RAM, SP_RAM, false, nmi);
        //        if(nmi) nmi = false;
        //    }
        //    nmi = exec_ppu(mm_WRAM, mm_PPU_RAM, mm_SP_RAM, mm_COLOR, scr.BGoffset_X, scr.BGoffset_Y);
        //}

        if(en_bmp && f % interval == 0){
            set_vram(COLOR, VRAM);
            make_bmp(VRAM, index++);
        }
    }

    cout << "Finish" << endl;
    return 0;
}

void load_ROM(ifstream *rom, uint8_t* WRAM, uint8_t* PPU_RAM){
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
    uint8_t *prom_ptr = (prom_size == 1) ? WRAM + 0xC000 : WRAM + 0x8000;
    //uint8_t *prom_ptr = (prom_size == 1) ? WRAM + 0x4000 : WRAM;
    for(int i=0;i<psize;i++)
        rom->read((char*)(prom_ptr+i), sizeof(uint8_t));
    
    for(int i=0;i<csize;i++)
        rom->read((char*)(PPU_RAM+i), sizeof(uint8_t));
}


void load_test_ROM(ifstream *rom){
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
    uint8_t *prom_ptr = (prom_size == 1) ? _PROM + 0x4000 : _PROM;
    for(int i=0;i<psize;i++)
        rom->read((char*)(prom_ptr+i), sizeof(uint8_t));
    
    for(int i=0;i<csize;i++)
        rom->read((char*)(_CROM+i), sizeof(uint8_t));
}

#define _rgb(r, g, b) (red = r, green = g, blue = b)
void set_vram(uint8_t* COLOR, uint8_t* VRAM){
    for(int i = 0; i < 240*256; i++){
        //BGR
        uint8_t blue, green, red; 
        switch(COLOR[i]){
            case 0x00: _rgb(0x75,0x75,0x75); break;
            case 0x01: _rgb(0x27,0x1B,0x8F); break;
            case 0x02: _rgb(0x00,0x00,0xAB); break;
            case 0x03: _rgb(0x47,0x00,0x9F); break;
            case 0x04: _rgb(0x8F,0x00,0x77); break;
            case 0x05: _rgb(0xAB,0x00,0x13); break;
            case 0x06: _rgb(0xA7,0x00,0x00); break;
            case 0x07: _rgb(0x7F,0x0B,0x00); break;
            case 0x08: _rgb(0x43,0x2F,0x00); break;
            case 0x09: _rgb(0x00,0x47,0x00); break;
            case 0x0a: _rgb(0x00,0x51,0x00); break;
            case 0x0b: _rgb(0x00,0x3F,0x17); break;
            case 0x0c: _rgb(0x1B,0x3F,0x5F); break;
            case 0x0d: _rgb(0x00,0x00,0x00); break;
            case 0x0e: _rgb(0x00,0x00,0x00); break;
            case 0x0f: _rgb(0x00,0x00,0x00); break;

            case 0x10: _rgb(0xBC,0xBC,0xBC); break;
            case 0x11: _rgb(0x00,0x73,0xEF); break;
            case 0x12: _rgb(0x23,0x3B,0xEF); break;
            case 0x13: _rgb(0x83,0x00,0xF3); break;
            case 0x14: _rgb(0xBF,0x00,0xBF); break;
            case 0x15: _rgb(0xE7,0x00,0x5B); break;
            case 0x16: _rgb(0xDB,0x2B,0x00); break;
            case 0x17: _rgb(0xCB,0x4F,0x0F); break;
            case 0x18: _rgb(0x8B,0x73,0x00); break;
            case 0x19: _rgb(0x00,0x97,0x00); break;
            case 0x1a: _rgb(0x00,0xAB,0x00); break;
            case 0x1b: _rgb(0x00,0x93,0x3B); break;
            case 0x1c: _rgb(0x00,0x83,0x8B); break;
            case 0x1d: _rgb(0x00,0x00,0x00); break;
            case 0x1e: _rgb(0x00,0x00,0x00); break;
            case 0x1f: _rgb(0x00,0x00,0x00); break;

            case 0x20: _rgb(0xFF,0xFF,0xFF); break;
            case 0x21: _rgb(0x3F,0xBF,0xFF); break;
            case 0x22: _rgb(0x5F,0x73,0xFF); break;
            case 0x23: _rgb(0xA7,0x8B,0xFD); break;
            case 0x24: _rgb(0xF7,0x7B,0xFF); break;
            case 0x25: _rgb(0xFF,0x77,0xB7); break;
            case 0x26: _rgb(0xFF,0x77,0x63); break;
            case 0x27: _rgb(0xFF,0x9B,0x3B); break;
            case 0x28: _rgb(0xF3,0xBF,0x3F); break;
            case 0x29: _rgb(0x83,0xD3,0x13); break;
            case 0x2a: _rgb(0x4F,0xDF,0x4B); break;
            case 0x2b: _rgb(0x58,0xF8,0x98); break;
            case 0x2c: _rgb(0x00,0xEB,0xDB); break;
            case 0x2d: _rgb(0x75,0x75,0x75); break;
            case 0x2e: _rgb(0x00,0x00,0x00); break;
            case 0x2f: _rgb(0x00,0x00,0x00); break;

            case 0x30: _rgb(0xFF,0xFF,0xFF); break;
            case 0x31: _rgb(0xAB,0xE7,0xFF); break;
            case 0x32: _rgb(0xC7,0xD7,0xFF); break;
            case 0x33: _rgb(0xD7,0xCB,0xFF); break;
            case 0x34: _rgb(0xFF,0xC7,0xFF); break;
            case 0x35: _rgb(0xFF,0xC7,0xDB); break;
            case 0x36: _rgb(0xFF,0xBF,0xB3); break;
            case 0x37: _rgb(0xFF,0xDB,0xAB); break;
            case 0x38: _rgb(0xFF,0xE7,0xA3); break;
            case 0x39: _rgb(0xE3,0xFF,0xA3); break;
            case 0x3a: _rgb(0xAB,0xF3,0xBF); break;
            case 0x3b: _rgb(0xB3,0xFF,0xCF); break;
            case 0x3c: _rgb(0x9F,0xFF,0xF3); break;
            case 0x3d: _rgb(0xBC,0xBC,0xBC); break;
            case 0x3e: _rgb(0x00,0x00,0x00); break;
            case 0x3f: _rgb(0x00,0x00,0x00); break;

            default: _rgb(0,0,0); break;
        }
        VRAM[3*i] = blue;
        VRAM[3*i + 1] = green;
        VRAM[3*i + 2] = red;
    }
}

void make_bmp(uint8_t* VRAM, int index){
    char bmp_file[256];
    sprintf(bmp_file, "./frame/frame_%d.bmp",index);
    ofstream bmp(bmp_file, ios::out|ios::binary);
    if(!bmp){
        cout << "cannot create BMP file." << endl;
        return;
    }

    const uint8_t bfType[] = "BM";
    const uint32_t bfSize = 184346; 
    const uint8_t bfReserved = 0;
    const uint32_t bfOffBits = 0x1A;

    bmp.write((char*)bfType, 2*sizeof(uint8_t));
    bmp.write((char*)&bfSize, sizeof(uint32_t));
    bmp.write((char*)&bfReserved, sizeof(uint8_t));
    bmp.write((char*)&bfReserved, sizeof(uint8_t));
    bmp.write((char*)&bfReserved, sizeof(uint8_t));
    bmp.write((char*)&bfReserved, sizeof(uint8_t));
    bmp.write((char*)&bfOffBits, sizeof(uint32_t));

    const uint32_t bcSize = 12;
    const uint16_t bcWidth = 256;
    const uint16_t bcHeight = 240;
    const uint16_t bcPlanes = 1;
    const uint16_t bcBitCount = 24;

    bmp.write((char*)&bcSize, sizeof(uint32_t));
    bmp.write((char*)&bcWidth, sizeof(uint16_t));
    bmp.write((char*)&bcHeight, sizeof(uint16_t));
    bmp.write((char*)&bcPlanes, sizeof(uint16_t));
    bmp.write((char*)&bcBitCount, sizeof(uint16_t));
    bmp.write((char*)VRAM, 3*256*240*sizeof(uint8_t));

    bmp.close(); 
}

