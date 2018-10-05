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

        hls_register uint1 SP_wide;

    public:

        void dump_regs(uint8_t insn);

        uint8_t read_prom8(uint16 addr, uint8_t* PROM);
        uint16_t read_prom16(uint16_t addr, uint8_t* PROM);
        uint8_t read_wram8(uint16 addr, uint8_t* WRAM);
        uint16_t read_wram16(uint16_t addr, uint8_t* WRAM);
        uint8_t read_mem8(uint16_t addr, uint8_t* WRAM, uint8_t* PROM);

        void load_key(uint8_t key){Input_Key(key);};
        void exec_DMA(uint8_t* SP_RAM, uint8_t* WRAM);
        void set_nmi();
        void set_reset();
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
        uint16_t get_PC();
        uint8_t bindFlags();

};

#endif
