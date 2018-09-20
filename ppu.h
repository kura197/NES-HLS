#ifndef PPU_H_
#define PPU_H_

#include <stdint.h>

class PPU{
    private:
        //NES *nes;
        //uint8_t VRAM[3*256*240];
        //uint8_t* VRAM;
        //uint8_t* CROM;
        //uint8_t VRAM_gray[256*240];
        bool BG_Valid[256];
        //uint8_t *PPU_RAM;
        //uint8_t *SP_RAM;
        //bool en_gray;
        uint8_t line;

    public:
        bool render(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* VRAM, uint8_t BG_offset_x, uint8_t BG_offset_y);
        void bg_render(uint8_t line, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* VRAM, uint8_t BG_offset_x, uint8_t BG_offset_y);
        void sp_render(uint8_t line, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* VRAM);
        void store_vram(uint8_t line, uint8_t x, uint8_t color, bool sprite, uint8_t* WRAM, uint8_t* VRAM);
        void set_bit(uint8_t* WRAM, uint16_t addr, uint8_t bit);
        void clr_bit(uint8_t* WRAM, uint16_t addr, uint8_t bit);
};



#endif

