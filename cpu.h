#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>
#include <fstream>
#include "nes.h"
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
        int rest;
        bool reset_line;
        bool irq_line;
        bool nmi_line;
        bool log;

    public:
        void dump_regs(uint8_t insn);
        //uint8_t read_mem8(uint16_t addr);
        uint16_t read_mem16(uint16_t addr);
        //void write_mem8(uint16_t addr, uint8_t data);
        void write_mem16(uint16_t addr, uint16_t data);
        //void set_nmi(bool signal);
        void set_irq(bool signal);
        void set_reset(bool signal);
        void reset();
        void exec(int clk);
        void exec_irq(int cause);

};

#endif
