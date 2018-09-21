#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>
#include <fstream>
#include "ram.h"
#include "instr.h"


#define IRQ 0
#define NMI 1
#define RESET 2

using namespace std;


class CPU : RAM{
    private:
        uint16_t PC;
        uint8_t ACC;
        uint8_t X;
        uint8_t Y;
        uint8_t SP;
        uint8_t CFlag, ZFlag, IFlag, DFlag, BFlag, VFlag, NFlag;
        //bool CFlag, ZFlag, IFlag, DFlag, BFlag, VFlag, NFlag;
        //int rest;
        bool reset_line;
        //bool irq_line;
        bool nmi_line;
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

        void norm_write8(uint16_t addr, uint8_t data, uint8_t* WRAM);
        uint8_t norm_read8(uint16_t addr, uint8_t* WRAM);
        uint16_t norm_read16(uint16_t addr, uint8_t* WRAM);

        void set_nmi();
        //void set_irq(bool signal);
        void set_reset();
        //void reset(uint8_t* WRAM, uint8_t* PPU_RAM);
        struct SCROLL exec(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM);
        void exec_irq(int cause, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM);
        void exec_addressing(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM);
        void exec_op(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM);
        void set_mode_false();
        void set_op_false();
        void execution(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM);

        uint16_t _imm(uint16_t opr_pc, uint8_t* WRAM)  
            {return PC++;};
        uint16_t _abs(uint16_t opr_pc, uint8_t* WRAM)  
            {PC+=2; return norm_read16(opr_pc, WRAM);};
        uint16_t _abxi(uint16_t opr_pc, uint8_t* WRAM) 
            {PC+=2; return norm_read16(norm_read16(opr_pc, WRAM)+X, WRAM);};
        uint16_t _abx(uint16_t opr_pc, uint8_t* WRAM)  
            {PC+=2; return norm_read16(opr_pc, WRAM)+X;};
        uint16_t _aby(uint16_t opr_pc, uint8_t* WRAM) 
            {PC+=2; return norm_read16(opr_pc, WRAM)+Y;};
        uint16_t _absi(uint16_t opr_pc, uint8_t* WRAM) 
            {PC+=2; return norm_read16(norm_read16(opr_pc, WRAM), WRAM);};
        uint16_t _zp(uint16_t opr_pc, uint8_t* WRAM)   
            {return norm_read8(PC++, WRAM);};
        uint16_t _zpxi(uint16_t opr_pc, uint8_t* WRAM) 
            {return norm_read16((uint8_t)(norm_read8(PC++, WRAM)+X), WRAM);};
        uint16_t _zpx(uint16_t opr_pc, uint8_t* WRAM)  
            {return (uint8_t)(norm_read8(PC++, WRAM)+X);};
        uint16_t _zpy(uint16_t opr_pc, uint8_t* WRAM)  
            {return (uint8_t)(norm_read8(PC++, WRAM)+Y);};
        uint16_t _zpiy(uint16_t opr_pc, uint8_t* WRAM) 
            {return norm_read16(norm_read8(PC++, WRAM), WRAM)+Y;};

        void push8(uint8_t data, uint8_t* WRAM);
        void push16(uint16_t data, uint8_t* WRAM);
        uint8_t pop8(uint8_t* WRAM);
        uint16_t pop16(uint8_t* WRAM);

};

#endif
