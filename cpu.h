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

class NES;

class CPU{
    private:
        NES *nes;
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
        uint8_t read_mem8(uint16_t addr);
        uint16_t read_mem16(uint16_t addr);
        void write_mem8(uint16_t addr, uint8_t data);
        void write_mem16(uint16_t addr, uint16_t data);
        void set_nmi(bool signal);
        void set_irq(bool signal);
        void set_reset(bool signal);
        void reset();
        void exec(int clk);
        void exec_irq(int cause);
        //uint16_t _absi(uint16_t opr_pc);
        //uint16_t _zpxi();
        //uint16_t _zpiy();
        uint16_t read_PC() { return PC; };
        uint8_t read_ACC() { return ACC; };
        uint8_t read_X()  { return X; };
        uint8_t read_Y()  { return Y; };
        uint8_t read_SP() { return SP; };
        uint8_t read_Flag() { return _bindFlags(); };
        void write_PC(uint16_t data) { PC = data; };
        void write_ACC(uint8_t data) { ACC = data; };
        void write_X(uint8_t data) { X = data; };
        void write_Y(uint8_t data) { Y = data; };
        void write_SP(uint8_t data) { SP = data; };
        void write_Flag(uint8_t data) { _unbindFlags(data); };
        void load_nes(NES *n) { nes = n; };

};

#endif
