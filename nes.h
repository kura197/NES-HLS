#ifndef NES_H_
#define NES_H_

#include "cpu.h"
#include "ram.h"
#include "ppu.h"
#include <stdint.h>
#include <fstream>

class CPU;
class RAM;
class PPU;

using namespace std;

class NES{
    private:
        int log;

    public:
        RAM *ram;
        CPU *cpu; 
        PPU *ppu;
        void load_ROM(ifstream *rom);
        void dump_WRAM(uint16_t start_addr, uint16_t size);
        void dump_PPURAM(uint16_t start_addr, uint16_t size);
        void nes_reset();
        void exec_frame();
        void SP_hit_reset();
        uint8_t* get_VRAM();
        //uint8_t* get_VRAM_gray();
        void enable_gray();
        void load_submodule(RAM *r, CPU *c, PPU *p);
};

#endif
