#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>
#include <fstream>
#include "ram.h"
#include "instr.h"
#include <HLS/ac_int.h>
#include <HLS/hls.h>

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
        uint8_t Stack[0x100];
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
        //uint8_t IR;

    public:
        void dump_regs(uint8_t insn);
        //uint8_t read_mem8(uint16_t addr);
        uint16_t read_mem16(uint16_t addr, uint8_t* WRAM, uint8_t* PPU_RAM);
        //void write_mem8(uint16_t addr, uint8_t data);
        void write_mem16(uint16_t addr, uint16_t data, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM);
        uint8_t read_prom(uint16_t addr, uint8_t* PROM);
        uint16_t read_prom16(uint16_t addr, uint8_t* PROM);
        void norm_write8(uint16_t addr, uint8_t data, uint8_t* WRAM);
        uint8_t norm_read8(uint16_t addr, uint8_t* WRAM, uint8_t* PROM);
        uint16_t norm_read16(uint16_t addr, uint8_t* WRAM, uint8_t* PROM);
        uint8_t read_mem8(uint16_t addr, uint8_t* WRAM, uint8_t* PROM);
        uint16_t addressing(uint16_t opr_pc, struct ADDRESS adr, uint8_t* WRAM, uint8_t* PROM);

        void load_key(uint8_t key){Input_Key(key);};
        void exec_DMA(uint8_t* SP_RAM, uint8_t* WRAM);
        void set_nmi();
        //void set_irq(bool signal);
        void set_reset();
        //void reset(uint8_t* WRAM, uint8_t* PPU_RAM);
        void set_mode_false(struct ADDRESS* adr);
        void exec(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* PROM, struct SPREG* spreg);
        void exec_irq(int cause, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* PROM);
        void execution(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* PROM, struct SPREG* spreg);

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

        uint16_t _imm(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
            {return PC++;};
        uint16_t _abs(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
            {PC+=2; return read_prom16(opr_pc, PROM);};
        uint16_t _abx(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
            {PC+=2; return read_prom16(opr_pc, PROM)+X;};
        uint16_t _aby(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM) 
            {PC+=2; return read_prom16(opr_pc, PROM)+Y;};
        uint16_t _absi(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM) 
            {PC+=2; return norm_read16(read_prom16(opr_pc, PROM), WRAM, PROM);};
        uint16_t _zp(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)   
            {return read_prom(PC++, PROM);};
        uint16_t _zpxi(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM) 
            {return norm_read16((uint8_t)(read_prom(PC++, PROM)+X), WRAM, PROM);};
        uint16_t _zpx(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
            {return (uint8_t)(read_prom(PC++, PROM)+X);};
        uint16_t _zpy(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
            {return (uint8_t)(read_prom(PC++, PROM)+Y);};
        uint16_t _zpiy(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM) 
            {return norm_read16(read_prom(PC++, PROM), WRAM, PROM)+Y;};

        //uint16_t _imm(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
        //    {return PC++;};
        //uint16_t _abs(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
        //    {PC+=2; return norm_read16(opr_pc, WRAM, PROM);};
        //uint16_t _abx(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
        //    {PC+=2; return norm_read16(opr_pc, WRAM, PROM)+X;};
        //uint16_t _aby(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM) 
        //    {PC+=2; return norm_read16(opr_pc, WRAM, PROM)+Y;};
        //uint16_t _absi(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM) 
        //    {PC+=2; return norm_read16(norm_read16(opr_pc, WRAM, PROM), WRAM, PROM);};
        //uint16_t _zp(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)   
        //    {return norm_read8(PC++, WRAM, PROM);};
        //uint16_t _zpxi(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM) 
        //    {return norm_read16((uint8_t)(norm_read8(PC++, WRAM, PROM)+X), WRAM, PROM);};
        //uint16_t _zpx(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
        //    {return (uint8_t)(norm_read8(PC++, WRAM, PROM)+X);};
        //uint16_t _zpy(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM)  
        //    {return (uint8_t)(norm_read8(PC++, WRAM, PROM)+Y);};
        //uint16_t _zpiy(uint16_t opr_pc, uint8_t* WRAM, uint8_t* PROM) 
        //    {return norm_read16(norm_read8(PC++, WRAM, PROM), WRAM, PROM)+Y;};
        
        void push8(uint8_t data, uint8_t* WRAM);
        void push16(uint16_t data, uint8_t* WRAM);
        uint8_t pop8(uint8_t* WRAM);
        uint16_t pop16(uint8_t* WRAM);

};

#endif
