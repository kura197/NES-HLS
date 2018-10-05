#ifndef WRAM_H_
#define WRAM_H_

#include <stdint.h>
#include <fstream>
#include "HLS/hls.h"
#include "HLS/ac_int.h"


using namespace std;

#define WRAM_SIZE 0x8000
#define PPU_RAM_SIZE 0x2000
#define SP_RAM_SIZE 0x100

#define _set(data, bit) ((data) ? 1 << bit : 0)

struct SPREG{
    uint8_t BGoffset_X;
    uint8_t BGoffset_Y;
    uint1 VBlank_NMI;
    uint1 BGPtnAddr;
    uint1 SPPtnAddr;
    uint1 NameAddrH;
    uint1 NameAddrL;
    uint1 EnSP;
    uint1 EnBG;
    uint1 SPMSK;
    uint1 BGMSK;
    uint1 VBlank;
    uint1 SPhit;
    uint1 num_ScanSP;
};


class RAM{
    private:
        uint8_t spram_buf;

        //PAD
        uint8_t key_input;
        uint8 pad_input;
        uint8_t pad_reset_state;
        uint3 pad_read_state;

    public:
        uint1 PPUInc;
        uint8_t SPAddr;
        uint1 BGoffset_sel_X;
        struct SPREG spreg;
        uint1 PPUAddr_sel_H;
        uint16 PPUAddr;

        uint8_t DMAAddrH;
        uint8_t DMAAddrL;
        uint1 DMAExcute;

        uint8_t read(uint16_t addr, uint8_t* WRAM, uint8_t* PPU_RAM, struct SPREG* spreg, uint8_t* CROM);
        void write(uint16_t addr, uint8_t data, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, struct SPREG* spreg, uint8_t* CROM);
        void write_2000(uint8 data, struct SPREG* spreg);
        void write_2001(uint8 data, struct SPREG* spreg);
        void write_2003(uint8_t data);
        void write_2004(uint8_t data, uint8_t* SP_RAM);
        void write_2005(uint8_t data, struct SPREG* spreg);
        void write_2006(uint8 data);
        void write_2007(uint8_t data, uint8_t* PPU_RAM, uint8_t* CROM);
        uint8_t read_2007(uint8_t* PPU_RAM, uint8_t* CROM);
        void DMA_start(uint8_t addr_H, uint8_t* WRAM, uint8_t* SP_RAM);
        void reset_pad(uint8 data);
        uint8_t read_pad_1();
        uint8_t read_pad_2();
        void Input_Key(uint8_t key) {key_input = key;};

};


#endif
