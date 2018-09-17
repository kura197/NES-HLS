#ifndef PPU_H_
#define PPU_H_

#include "nes.h"

class NES;

class PPU{
    private:
        NES *nes;
        //uint8_t VRAM[3*256*240];
        uint8_t* VRAM;
        uint8_t* CROM;
        //uint8_t VRAM_gray[256*240];
        bool BG_Valid[256];
        uint8_t *PPU_RAM;
        uint8_t *SP_RAM;
        bool en_gray;

    public:
        PPU(NES *n, uint8_t* vram, uint8_t* crom);
        ~PPU();
        uint8_t read_mem8(uint16_t addr);
        void render(uint8_t line);
        void bg_render(uint8_t line);
        void sp_render(uint8_t line);
        void store_vram(uint8_t line, uint8_t x, uint8_t color, bool sprite);
        void make_bmp(ofstream *bmp);
        uint8_t* get_VRAM(){ return VRAM; };
        //uint8_t* get_VRAM_gray(){ return VRAM_gray; };
        void enable_gray(){ en_gray = true; };
};

#endif

