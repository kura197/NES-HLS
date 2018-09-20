#ifndef WRAM_H_
#define WRAM_H_

#include <stdint.h>
#include <fstream>

using namespace std;

#define WRAM_SIZE 0x8000
#define PPU_RAM_SIZE 0x2000
#define SP_RAM_SIZE 0x100

#define _set(data, bit) ((data) ? 1 << bit : 0)

struct SCROLL{
    uint8_t BGoffset_X;
    uint8_t BGoffset_Y;
};


class RAM{
    private:
        //uint8_t WRAM[WRAM_SIZE];
        //uint8_t* PROM;
        //uint8_t* CROM;
        //uint8_t PPU_RAM[PPU_RAM_SIZE];
        //uint8_t SP_RAM[SP_RAM_SIZE];
        uint8_t spram_buf;

        //PAD
        uint8_t key_input;
        uint8_t pad_input;
        uint8_t pad_reset_state;
        uint8_t pad_read_state;

    public:
        //$2000
        //bool VBlank_NMI;
        //bool SPSize;
        //bool BGPtnAddr;
        //bool SPPtnAddr;
        bool PPUInc;
        //bool NameAddrH;
        //bool NameAddrL;
        //$2001
        //bool BGColor2;
        //bool BGColor1;
        //bool BGColor0;
        //bool EnSP;
        //bool EnBG;
        //bool SPMSK;
        //bool BGMSK;
        //bool DispType;
        //$2002
        //bool VBlank;
        //bool SPhit;
        //bool num_ScanSP;
        //$2003
        uint8_t SPAddr;
        //$2005
        bool BGoffset_sel_X;
        //uint8_t BGoffset_X;
        //uint8_t BGoffset_Y;
        struct SCROLL scr;
        //$2006
        bool PPUAddr_sel_H;
        uint8_t PPUAddr_H;
        uint8_t PPUAddr_L;



        //RAM(NES *n, uint8_t* prom, uint8_t* crom);
        //RAM();
        //void dump_WRAM(uint16_t start_addr, uint16_t size);
        //void dump_PROM(uint16_t start_addr, uint16_t size);
        //void dump_PPURAM(uint16_t start_addr, uint16_t size);
        uint8_t read(uint16_t addr, uint8_t* WRAM, uint8_t* PPU_RAM);
        void write(uint16_t addr, uint8_t data, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM);
        //void set_VBlank(bool vblank, bool nmi);
        //void write_2000(uint8_t data);
        //void write_2001(uint8_t data);
        void write_2003(uint8_t data);
        void write_2004(uint8_t data, uint8_t* SP_RAM);
        void write_2005(uint8_t data);
        void write_2006(uint8_t data);
        void write_2007(uint8_t data, uint8_t* PPU_RAM);
        uint8_t read_2007(uint8_t* PPU_RAM);
        void DMA_start(uint8_t addr_H, uint8_t* WRAM, uint8_t* SP_RAM);
        void reset_pad(uint8_t data);
        uint8_t read_pad_1();
        uint8_t read_pad_2();
        void Input_Key(uint8_t key) {key_input = key;};
        //void frame_end(uint8_t* WRAM);

};


#endif
