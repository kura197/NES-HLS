#ifndef PPU_H_
#define PPU_H_

#include <stdint.h>
#include "ram.h"
#include "HLS/hls.h"
#include "HLS/ac_int.h"

class PPU{
    private:
        //NES *nes;
        //uint8_t VRAM[3*256*240];
        //uint8_t* VRAM;
        //uint8_t* CROM;
        //uint8_t VRAM_gray[256*240];
        //uint8_t BG_Valid[32];
        ac_int<1, false> BG_Valid[256];
        //uint8_t *PPU_RAM;
        //uint8_t *SP_RAM;
        //bool en_gray;
        hls_register uint8_t line;

    public:
        void BG_Valid_set(uint8_t x, uint8_t bit, bool valid);
        bool BG_Valid_check(uint8_t x);
        bool render(uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* VRAM, struct SPREG* spreg, uint8_t* CROM);
        //void bg_render(uint8_t line, uint8_t ctrlreg1, uint8_t ctrlreg2, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* VRAM, uint8_t BG_offset_x, uint8_t BG_offset_y);
        //void sp_render(uint8_t line, uint8_t ctrlreg1, uint8_t ctrlreg2, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* VRAM);
        //void store_vram(uint8_t line, uint8_t x, uint8_t color, bool sprite, uint8_t* WRAM, uint8_t* VRAM, uint8_t ctrlreg2);
        void bg_render(uint8_t line, struct SPREG* spreg, uint8_t* PPU_RAM, uint8_t* VRAM, uint8_t* CROM);
        void sp_render(uint8_t line, struct SPREG* spreg, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* VRAM, uint8_t* CROM);
        void store_vram(uint8_t line, uint8_t x, uint8_t color, bool sprite, uint8_t* VRAM, struct SPREG* spreg);
        void set_bit(uint8_t* WRAM, uint16_t addr, uint8_t bit);
        void clr_bit(uint8_t* WRAM, uint16_t addr, uint8_t bit);
        uint8_t read_PPURAM(uint16_t addr, uint8_t* PPU_RAM);

};



#endif

