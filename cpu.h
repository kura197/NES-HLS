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
        uint8_t ACC;
        uint8_t X;
        uint8_t Y;
        uint8_t SP;
        //uint8_t Stack[0x100];
        //uint8_t CFlag, ZFlag, IFlag, DFlag, BFlag, VFlag, NFlag;
        //bool CFlag, ZFlag, IFlag, DFlag, BFlag, VFlag, NFlag;
        uint1 CFlag, ZFlag, IFlag, DFlag, BFlag, VFlag, NFlag;
        //int rest;
        uint1 reset_line;
        //bool irq_line;
        uint1 nmi_line;
        //bool log;
        //uint16_t addr;
        //bool op;
        uint8_t IR;
        uint16_t Stack_PC;
        uint8_t Stack_Flags;

        //uint16_t res_vec;
        //uint16_t irq_vec;
        //uint16_t nmi_vec;

        hls_register uint8_t cache[4];
        hls_register uint16_t cache_addr;
        //uint1 Valid[4];
        uint8_t Valid[4];

        struct ADDRESS adr;
        uint1 op_adc, op_sbc, op_cmp, op_and, op_ora, op_eor, op_bit;
        uint1 op_load, op_store, op_mov, op_asl, op_lsr, op_rol, op_ror, op_bra_false;
        uint1 op_inc, op_dec, op_bra, op_jmp, op_jsr, op_rts, op_rti, op_push, op_pop;
        uint1 acc, x, y;
        hls_register uint16_t addr;

    public:
        bool state;

        //CPU(uint8_t* PROM);
        void dump_regs(uint8_t insn);
        //uint8_t read_mem8(uint16_t addr);
        uint16_t read_mem16(uint16_t addr, uint8_t* WRAM, uint8_t* PPU_RAM);
        //void write_mem8(uint16_t addr, uint8_t data);
        void write_mem16(uint16_t addr, uint16_t data, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM);
        uint8_t read_prom(uint16_t addr, uint8_t* PROM);
        uint16_t read_prom16(uint16_t addr, uint8_t* PROM);
        void norm_write8(uint16_t addr, uint8_t data, uint8_t* WRAM);
        uint8_t norm_read8(uint16_t addr, uint8_t* WRAM);
        uint16_t norm_read16(uint16_t addr, uint8_t* WRAM);
        uint8_t read_mem8(uint16_t addr, uint8_t* WRAM, uint32_t* PROM);

        void load_key(uint8_t key){Input_Key(key);};
        void exec_DMA(uint8_t* SP_RAM, uint8_t* WRAM);
        void set_nmi();
        //void set_irq(bool signal);
        void set_reset();
        //void reset(uint8_t* WRAM, uint8_t* PPU_RAM);
        void set_mode_false(struct ADDRESS* adr);
        void exec(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint32_t* PROM, struct SPREG* spreg, uint8_t* Stack, uint8_t* CROM);
        void exec_irq(int cause, uint16_t nmi_vec, uint16_t res_vec, uint16_t irq_vec);
        void execution(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint32_t* PROM, struct SPREG* spreg, uint8_t* Stack, uint8_t* CROM);

        uint16_t addressing(struct ADDRESS adr, uint8_t* WRAM, uint32_t* PROM);

        //uint16_t _imm(uint16_t opr_pc, uint8_t* WRAM)  
        //    {return PC++;};
        //uint16_t _abs(uint16_t opr_pc, uint8_t* WRAM)  
        //    {PC+=2; return norm_read16(opr_pc, WRAM);};
        //uint16_t _abxi(uint16_t opr_pc, uint8_t* WRAM) 
        //    {PC+=2; return norm_read16(norm_read16(opr_pc, WRAM)+X, WRAM);};
        //uint16_t _abx(uint16_t opr_pc, uint8_t* WRAM)  
        //    {PC+=2; return norm_read16(opr_pc, WRAM)+X;};
        //uint16_t _aby(uint16_t opr_pc, uint8_t* WRAM) 
        //    {PC+=2; return norm_read16(opr_pc, WRAM)+Y;};
        //uint16_t _absi(uint16_t opr_pc, uint8_t* WRAM) 
        //    {PC+=2; return norm_read16(norm_read16(opr_pc, WRAM), WRAM);};
        //uint16_t _zp(uint16_t opr_pc, uint8_t* WRAM)   
        //    {return norm_read8(PC++, WRAM);};
        //uint16_t _zpxi(uint16_t opr_pc, uint8_t* WRAM) 
        //    {return norm_read16((uint8_t)(norm_read8(PC++, WRAM)+X), WRAM);};
        //uint16_t _zpx(uint16_t opr_pc, uint8_t* WRAM)  
        //    {return (uint8_t)(norm_read8(PC++, WRAM)+X);};
        //uint16_t _zpy(uint16_t opr_pc, uint8_t* WRAM)  
        //    {return (uint8_t)(norm_read8(PC++, WRAM)+Y);};
        //uint16_t _zpiy(uint16_t opr_pc, uint8_t* WRAM) 
        //    {return norm_read16(norm_read8(PC++, WRAM), WRAM)+Y;};

        //uint16_t _imm(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
        //    {return PC++;};
        //uint16_t _abs(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
        //    {PC+=2; return read_prom16(opr_pc, PROM);};
        //uint16_t _abx(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
        //    {PC+=2; return read_prom16(opr_pc, PROM)+X;};
        //uint16_t _aby(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM) 
        //    {PC+=2; return read_prom16(opr_pc, PROM)+Y;};
        //uint16_t _absi(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM) 
        //    {PC+=2; return norm_read16(read_prom16(opr_pc, PROM), WRAM, PROM);};
        //uint16_t _zp(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)   
        //    {return read_prom(PC++, PROM);};
        //uint16_t _zpxi(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM) 
        //    {return norm_read16((uint8_t)(read_prom(PC++, PROM)+X), WRAM, PROM);};
        //uint16_t _zpx(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
        //    {return (uint8_t)(read_prom(PC++, PROM)+X);};
        //uint16_t _zpy(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
        //    {return (uint8_t)(read_prom(PC++, PROM)+Y);};
        //uint16_t _zpiy(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM) 
        //    {return norm_read16(read_prom(PC++, PROM), WRAM, PROM)+Y;};

        void push8(uint8_t data, uint8_t* Stack);
        void push16(uint16_t data, uint8_t* Stack);
        uint8_t pop8(uint8_t* Stack);
        uint16_t pop16(uint8_t* Stack);

        uint8_t read_prom_ex8(uint16_t addr, uint32_t* PROM);
        uint16_t read_prom_ex16(uint16_t addr, uint32_t* PROM);
        uint32_t read_prom_ex32(uint16_t addr, uint32_t* PROM);
        void cache_update(uint16_t addr, uint32_t* PROM);
        void cache_false();

        uint16_t get_PC();
        uint16_t get_IR();
        uint32_t get_cache();
        uint8_t get_SP();
        uint8_t get_ACC();
        uint8_t get_X();
        uint8_t get_Y();

};

#endif
