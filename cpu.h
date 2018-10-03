#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>
#include <fstream>
#include "ram.h"
#include "instr.h"
#include "HLS/hls.h"
#include "HLS/ac_int.h"

#define IRQ 0
#define NMI 1
#define RESET 2

using namespace std;

struct ADDRESS{
    bool imm, zp, zpx, zpy, abs, abx, aby, zpxi, zpiy, absi, imp;
};

class CPU : RAM{
    private:
        uint16_t PC;
        uint8 ACC;
        uint8 X;
        uint8 Y;
        uint8 SP;
        uint1 CFlag, ZFlag, IFlag, DFlag, BFlag, VFlag, NFlag;
        uint1 reset_line;
        uint1 nmi_line;
        uint16_t Stack_PC;
        uint8_t Stack_Flags;


        uint16_t cache_addr;
        uint32 cache;
        uint4 V;
        uint1 PC_update;

        hls_register uint1 SP_wide;
        

    public:

        void dump_regs(uint8_t insn);
        uint16_t read_mem16(uint16_t addr, uint8_t* WRAM, uint8_t* PPU_RAM);
        void write_mem16(uint16_t addr, uint16_t data, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM);
        uint8_t read_prom(uint16 addr, uint8_t* PROM);
        uint16_t read_prom16(uint16_t addr, uint8_t* PROM);
        void norm_write8(uint16_t addr, uint8_t data, uint8_t* WRAM);
        uint8_t norm_read8(uint16 addr, uint8_t* WRAM);
        uint16_t norm_read16(uint16_t addr, uint8_t* WRAM);
        uint8_t read_mem8(uint16_t addr, uint8_t* WRAM, uint8_t* PROM);

        void load_key(uint8_t key){Input_Key(key);};
        void exec_DMA(uint8_t* SP_RAM, uint8_t* WRAM);
        void set_nmi();
        //void set_irq(bool signal);
        void set_reset();
        //void reset(uint8_t* WRAM, uint8_t* PPU_RAM);
        void set_mode_false(struct ADDRESS* adr);
        void exec(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* PROM, struct SPREG* spreg, uint16_t* Stack, uint8_t* CROM);
        void exec_irq(int cause, uint16_t nmi_vec, uint16_t res_vec, uint16_t irq_vec);
        void execution(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* PROM, struct SPREG* spreg, uint16_t* Stack, uint8_t* CROM);

        uint16_t addressing(struct ADDRESS adr, uint8_t* WRAM, uint8_t* PROM);


        void push8(uint8_t data, uint8_t* Stack);
        void push16(uint16_t data, uint8_t* Stack);
        uint8_t pop8(uint8_t* Stack);
        uint16_t pop16(uint8_t* Stack);
        
        void push_ex8(uint8_t data, uint16_t* Stack);
        void push_ex16(uint16_t data, uint16_t* Stack);
        uint8_t pop_ex8(uint16_t* Stack);
        uint16_t pop_ex16(uint16_t* Stack);


        uint8_t read_prom_ex8(uint16 addr, uint32_t* PROM);
        uint16_t read_prom_ex16(uint16 addr, uint32_t* PROM);
        uint32_t read_prom_ex32(uint16 addr, uint32_t* PROM);
        void cache_update(uint16_t addr, uint32_t* PROM);
        void cache_false();

        uint16_t get_PC();
        uint16_t get_IR();
        uint32_t get_cache();
        uint8_t get_SP();
        uint8_t get_ACC();
        uint8_t get_X();
        uint8_t get_Y();
        uint16_t get_addr();

        uint8_t bindFlags();

};

#endif
