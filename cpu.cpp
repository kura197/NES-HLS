#include "cpu.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include "instr.h"

//void CPU::dump_regs(uint8_t insn){
//    printf("%04x %02x   A:%02x X:%02x Y:%02x P:%02x SP:%02x\n",
//                    PC, insn, ACC, X, Y, _bindFlags(), SP);
//}

//uint8_t CPU::read_mem8(uint16_t addr, uint8_t* WRAM, uint8_t* PPU_RAM){
//    return read(addr, WRAM, PPU_RAM);
//}

uint16_t CPU::read_mem16(uint16_t addr, uint8_t* WRAM, uint8_t* PPU_RAM){
    uint16_t rddata;
    rddata = read(addr, WRAM, PPU_RAM);
    rddata = rddata | ((uint16_t)read(addr+1, WRAM, PPU_RAM) << 8);
    return rddata;
}

//void CPU::write_mem8(uint16_t addr, uint8_t data){
//    nes->ram->write(addr, data);
//}
void CPU::write_mem16(uint16_t addr, uint16_t data, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM){
    write(addr, (uint8_t)data, WRAM, PPU_RAM, SP_RAM);
    write(addr+1, (uint8_t)(data >> 8), WRAM, PPU_RAM, SP_RAM);
}

//void CPU::set_nmi(bool signal)
//{
//  if (!nmi_line && signal){
//    //if(log) cout << "nmi interrupt occur" << endl;
//    exec_irq(NMI);
//  }
//  nmi_line = signal;
//}
void CPU::set_nmi(bool signal)
{
  nmi_line = signal;
}

//void CPU::set_irq(bool signal)
//{
//  irq_line = signal;
//}
//

void CPU::set_reset(bool signal)
{
  reset_line = signal;
}

//void CPU::reset(uint8_t* WRAM, uint8_t* PPU_RAM){
//    ACC = 0;
//    X = 0;
//    Y = 0;
//    SP = 0xFD;
//    CFlag = 0;
//    ZFlag = 0;
//    IFlag = 1;
//    DFlag = 0;
//    BFlag = 0;
//    VFlag = 0; 
//    NFlag = 0;
//    PC = read_mem16(0xFFFC, WRAM, PPU_RAM);
//    nmi_line = false;
//    irq_line = false;
//    reset_line = false;
//    //printf("PC : %04x\n", PC);
//}

struct SCROLL CPU::exec(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM){
    if(!op){
        if(reset_line) exec_irq(RESET, WRAM, PPU_RAM, SP_RAM); reset_line = false;
        if(nmi_line) exec_irq(NMI, WRAM, PPU_RAM, SP_RAM); nmi_line = false;
        exec_addressing(WRAM, PPU_RAM, SP_RAM);
    }
    //else exec_op(WRAM, PPU_RAM, SP_RAM);
    scr.BGoffset_X = (uint8_t)addr;
    
    //if(reset_line) exec_irq(RESET, WRAM, PPU_RAM, SP_RAM); reset_line = false;
    //if(nmi_line) exec_irq(NMI, WRAM, PPU_RAM, SP_RAM); nmi_line = false;
    //test_exec(WRAM, PPU_RAM, SP_RAM);
    return scr;
}

void CPU::exec_addressing(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM){
    IR = read(PC, WRAM, PPU_RAM);
    //dump_regs(IR);
    PC++;
    uint16_t opr_pc = PC;
    switch(IR){
        /* ALU */
        case 0x69: addr = _imm(opr_pc, WRAM, PPU_RAM);  break;
        case 0x65: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0x75: addr = _zpx(opr_pc, WRAM, PPU_RAM);  break;
        case 0x6D: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;
        case 0x7D: addr = _abx(opr_pc, WRAM, PPU_RAM);  break;
        case 0x79: addr = _aby(opr_pc, WRAM, PPU_RAM);  break;
        case 0x61: addr = _zpxi(opr_pc, WRAM, PPU_RAM); break;
        case 0x71: addr = _zpiy(opr_pc, WRAM, PPU_RAM); break;

        case 0xE9: addr = _imm(opr_pc, WRAM, PPU_RAM);  break;
        case 0xE5: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0xF5: addr = _zpx(opr_pc, WRAM, PPU_RAM);  break;
        case 0xED: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;
        case 0xFD: addr = _abx(opr_pc, WRAM, PPU_RAM);  break;
        case 0xF9: addr = _aby(opr_pc, WRAM, PPU_RAM);  break;
        case 0xE1: addr = _zpxi(opr_pc, WRAM, PPU_RAM); break;
        case 0xF1: addr = _zpiy(opr_pc, WRAM, PPU_RAM); break;

        case 0xC9: addr = _imm(opr_pc, WRAM, PPU_RAM);  break;
        case 0xC5: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0xD5: addr = _zpx(opr_pc, WRAM, PPU_RAM);  break;
        case 0xCD: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;
        case 0xDD: addr = _abx(opr_pc, WRAM, PPU_RAM);  break;
        case 0xD9: addr = _aby(opr_pc, WRAM, PPU_RAM);  break;
        case 0xC1: addr = _zpxi(opr_pc, WRAM, PPU_RAM); break;
        case 0xD1: addr = _zpiy(opr_pc, WRAM, PPU_RAM); break;

        case 0xE0: addr = _imm(opr_pc, WRAM, PPU_RAM); break;
        case 0xE4: addr = _zp(opr_pc, WRAM, PPU_RAM);  break;
        case 0xEC: addr = _abs(opr_pc, WRAM, PPU_RAM); break;

        case 0xC0: addr = _imm(opr_pc, WRAM, PPU_RAM); break;
        case 0xC4: addr = _zp(opr_pc, WRAM, PPU_RAM);  break;
        case 0xCC: addr = _abs(opr_pc, WRAM, PPU_RAM); break;

        case 0x29: addr = _imm(opr_pc, WRAM, PPU_RAM);  break;
        case 0x25: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0x35: addr = _zpx(opr_pc, WRAM, PPU_RAM);  break;
        case 0x2D: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;
        case 0x3D: addr = _abx(opr_pc, WRAM, PPU_RAM);  break;
        case 0x39: addr = _aby(opr_pc, WRAM, PPU_RAM);  break;
        case 0x21: addr = _zpxi(opr_pc, WRAM, PPU_RAM); break;
        case 0x31: addr = _zpiy(opr_pc, WRAM, PPU_RAM); break;

        case 0x09: addr = _imm(opr_pc, WRAM, PPU_RAM);  break;
        case 0x05: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0x15: addr = _zpx(opr_pc, WRAM, PPU_RAM);  break;
        case 0x0D: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;
        case 0x1D: addr = _abx(opr_pc, WRAM, PPU_RAM);  break;
        case 0x19: addr = _aby(opr_pc, WRAM, PPU_RAM);  break;
        case 0x01: addr = _zpxi(opr_pc, WRAM, PPU_RAM); break;
        case 0x11: addr = _zpiy(opr_pc, WRAM, PPU_RAM); break;

        case 0x49: addr = _imm(opr_pc, WRAM, PPU_RAM);  break;
        case 0x45: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0x55: addr = _zpx(opr_pc, WRAM, PPU_RAM);  break;
        case 0x4D: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;
        case 0x5D: addr = _abx(opr_pc, WRAM, PPU_RAM);  break;
        case 0x59: addr = _aby(opr_pc, WRAM, PPU_RAM);  break;
        case 0x41: addr = _zpxi(opr_pc, WRAM, PPU_RAM); break;
        case 0x51: addr = _zpiy(opr_pc, WRAM, PPU_RAM); break;

        case 0x24: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0x2C: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;

                   /* load / store */
        case 0xA9: addr = _imm(opr_pc, WRAM, PPU_RAM);  break;
        case 0xA5: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0xB5: addr = _zpx(opr_pc, WRAM, PPU_RAM);  break;
        case 0xAD: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;
        case 0xBD: addr = _abx(opr_pc, WRAM, PPU_RAM);  break;
        case 0xB9: addr = _aby(opr_pc, WRAM, PPU_RAM);  break;
        case 0xA1: addr = _zpxi(opr_pc, WRAM, PPU_RAM); break;
        case 0xB1: addr = _zpiy(opr_pc, WRAM, PPU_RAM); break;

        case 0xA2: addr = _imm(opr_pc, WRAM, PPU_RAM);  break;
        case 0xA6: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0xB6: addr = _zpy(opr_pc, WRAM, PPU_RAM);  break;
        case 0xAE: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;
        case 0xBE: addr = _aby(opr_pc, WRAM, PPU_RAM);  break;

        case 0xA0: addr = _imm(opr_pc, WRAM, PPU_RAM);  break;
        case 0xA4: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0xB4: addr = _zpx(opr_pc, WRAM, PPU_RAM);  break;
        case 0xAC: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;
        case 0xBC: addr = _abx(opr_pc, WRAM, PPU_RAM);  break;

        case 0x85: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0x95: addr = _zpx(opr_pc, WRAM, PPU_RAM);  break;
        case 0x8D: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;
        case 0x9D: addr = _abx(opr_pc, WRAM, PPU_RAM);  break;
        case 0x99: addr = _aby(opr_pc, WRAM, PPU_RAM);  break;
        case 0x81: addr = _zpxi(opr_pc, WRAM, PPU_RAM); break;
        case 0x91: addr = _zpiy(opr_pc, WRAM, PPU_RAM); break;

        case 0x86: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0x96: addr = _zpy(opr_pc, WRAM, PPU_RAM);  break;
        case 0x8E: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;

        case 0x84: addr = _zp(opr_pc, WRAM, PPU_RAM);   break;
        case 0x94: addr = _zpx(opr_pc, WRAM, PPU_RAM);  break;
        case 0x8C: addr = _abs(opr_pc, WRAM, PPU_RAM);  break;

        //           // transfer 
        //case 0xAA: _mov(X,ACC); break; // TAX
        //case 0xA8: _mov(Y,ACC); break; // TAY
        //case 0x8A: _mov(ACC,X); break; // TXA
        //case 0x98: _mov(ACC,Y); break; // TYA
        //case 0xBA: _mov(X,SP); break; // TSX
        //case 0x9A: SP=X; break; // TXS

                   /* shift */
        case 0x06: addr = _zp(opr_pc, WRAM, PPU_RAM);  break;
        case 0x16: addr = _zpx(opr_pc, WRAM, PPU_RAM); break;
        case 0x0E: addr = _abs(opr_pc, WRAM, PPU_RAM); break;
        case 0x1E: addr = _abx(opr_pc, WRAM, PPU_RAM); break;

        case 0x46: addr = _zp(opr_pc, WRAM, PPU_RAM);  break;
        case 0x56: addr = _zpx(opr_pc, WRAM, PPU_RAM); break;
        case 0x4E: addr = _abs(opr_pc, WRAM, PPU_RAM); break;
        case 0x5E: addr = _abx(opr_pc, WRAM, PPU_RAM); break;

        case 0x26: addr = _zp(opr_pc, WRAM, PPU_RAM);  break;
        case 0x36: addr = _zpx(opr_pc, WRAM, PPU_RAM); break;
        case 0x2E: addr = _abs(opr_pc, WRAM, PPU_RAM); break;
        case 0x3E: addr = _abx(opr_pc, WRAM, PPU_RAM); break;

        case 0x66: addr = _zp(opr_pc, WRAM, PPU_RAM);  break;
        case 0x76: addr = _zpx(opr_pc, WRAM, PPU_RAM); break;
        case 0x6E: addr = _abs(opr_pc, WRAM, PPU_RAM); break;
        case 0x7E: addr = _abx(opr_pc, WRAM, PPU_RAM); break;

        case 0xE6: addr = _zp(opr_pc, WRAM, PPU_RAM);  break;
        case 0xF6: addr = _zpx(opr_pc, WRAM, PPU_RAM); break;
        case 0xEE: addr = _abs(opr_pc, WRAM, PPU_RAM); break;
        case 0xFE: addr = _abx(opr_pc, WRAM, PPU_RAM); break;
        //case 0xE8: _incr(X); break;
        //case 0xC8: _incr(Y); break;

        case 0xC6: addr = _zp(opr_pc, WRAM, PPU_RAM);  break;
        case 0xD6: addr = _zpx(opr_pc, WRAM, PPU_RAM); break;
        case 0xCE: addr = _abs(opr_pc, WRAM, PPU_RAM); break;
        case 0xDE: addr = _abx(opr_pc, WRAM, PPU_RAM); break;
        //case 0xCA: _decr(X); break;
        //case 0x88: _decr(Y); break;

        //           // branch 
        //case 0x90: _bra(!CFlag); break; // BCC
        //case 0xB0: _bra( CFlag); break; // BCS
        //case 0xD0: _bra(!ZFlag); break; // BNE
        //case 0xF0: _bra( ZFlag); break; // BEQ
        //case 0x10: _bra(!NFlag); break; // BPL
        //case 0x30: _bra( NFlag); break; // BMI
        //case 0x50: _bra(!VFlag); break; // BVC
        //case 0x70: _bra( VFlag); break; // BVS

                   /* jump / call / return */
        case 0x4C: addr =_abs(opr_pc, WRAM, PPU_RAM) ; break; // JMP abs
        case 0x6C: addr =_absi(opr_pc, WRAM, PPU_RAM); break; // JMP (abs)

        case 0x20: _push16(PC+1);addr=_abs(opr_pc, WRAM, PPU_RAM); break; // JSR

        //case 0x60: PC=_pop16()+1; break; // RTS
        //case 0x40: _unbindFlags(_pop8());PC=_pop16(); break; // RTI

                   /* flag */
        //case 0x38: CFlag=1; break; // SEC
        //case 0xF8: DFlag=1; break; // SED
        //case 0x78: IFlag=1; break; // SEI

        //case 0x18: CFlag=0; break; // CLC
        //case 0xD8: DFlag=0; break; // CLD
        //case 0x58: IFlag=0; break; // CLI (この瞬間に割り込みがかかるかも知れん…)
        //case 0xB8: VFlag=0; break; // CLV

                   /* stack */
        //case 0x48: _push8(ACC); break; // PHA
        //case 0x08: _push8(_bindFlags()); break; // PHP
        //case 0x68: ACC=_pop8();NFlag=ACC>>7;ZFlag=ACC==0; break; // PLA
        //case 0x28: _unbindFlags(_pop8()); break; // PLP

        //           // others 
        //case 0x00: // BRK
        //           BFlag=1;
        //           PC++;
        //           exec_irq(IRQ, WRAM, PPU_RAM, SP_RAM);
        //           break;

       case 0xEA: break; // NOP

        default:
                   break;
    }

    op = true;
}

void CPU::exec_op(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM){
    switch(IR){
        /* ALU */
        case 0x69: _adc(addr);  break;
        case 0x65: _adc(addr);  break;
        case 0x75: _adc(addr);  break;
        case 0x6D: _adc(addr);  break;
        case 0x7D: _adc(addr);  break;
        case 0x79: _adc(addr);  break;
        case 0x61: _adc(addr); break;
        case 0x71: _adc(addr); break;

        case 0xE9: _sbc(addr);  break;
        case 0xE5: _sbc(addr);   break;
        case 0xF5: _sbc(addr);  break;
        case 0xED: _sbc(addr);  break;
        case 0xFD: _sbc(addr);  break;
        case 0xF9: _sbc(addr);  break;
        case 0xE1: _sbc(addr); break;
        case 0xF1: _sbc(addr); break;

        case 0xC9: _cmp(ACC,addr);  break;
        case 0xC5: _cmp(ACC,addr);   break;
        case 0xD5: _cmp(ACC,addr);  break;
        case 0xCD: _cmp(ACC,addr);  break;
        case 0xDD: _cmp(ACC,addr);  break;
        case 0xD9: _cmp(ACC,addr);  break;
        case 0xC1: _cmp(ACC,addr); break;
        case 0xD1: _cmp(ACC,addr); break;

        case 0xE0: _cmp(X,addr); break;
        case 0xE4: _cmp(X,addr);  break;
        case 0xEC: _cmp(X,addr); break;

        case 0xC0: _cmp(Y,addr); break;
        case 0xC4: _cmp(Y,addr);  break;
        case 0xCC: _cmp(Y,addr); break;

        case 0x29: _and(addr);  break;
        case 0x25: _and(addr);   break;
        case 0x35: _and(addr);  break;
        case 0x2D: _and(addr);  break;
        case 0x3D: _and(addr);  break;
        case 0x39: _and(addr);  break;
        case 0x21: _and(addr); break;
        case 0x31: _and(addr); break;

        case 0x09: _ora(addr);  break;
        case 0x05: _ora(addr);   break;
        case 0x15: _ora(addr);  break;
        case 0x0D: _ora(addr);  break;
        case 0x1D: _ora(addr);  break;
        case 0x19: _ora(addr);  break;
        case 0x01: _ora(addr); break;
        case 0x11: _ora(addr); break;

        case 0x49: _eor(addr);  break;
        case 0x45: _eor(addr);   break;
        case 0x55: _eor(addr);  break;
        case 0x4D: _eor(addr);  break;
        case 0x5D: _eor(addr);  break;
        case 0x59: _eor(addr);  break;
        case 0x41: _eor(addr); break;
        case 0x51: _eor(addr); break;

        case 0x24: _bit(addr);   break;
        case 0x2C: _bit(addr);  break;

                   /* laod / store */
        case 0xA9: _load(ACC,addr);  break;
        case 0xA5: _load(ACC,addr);   break;
        case 0xB5: _load(ACC,addr);  break;
        case 0xAD: _load(ACC,addr);  break;
        case 0xBD: _load(ACC,addr);  break;
        case 0xB9: _load(ACC,addr);  break;
        case 0xA1: _load(ACC,addr); break;
        case 0xB1: _load(ACC,addr); break;

        case 0xA2: _load(X,addr);  break;
        case 0xA6: _load(X,addr);   break;
        case 0xB6: _load(X,addr);  break;
        case 0xAE: _load(X,addr);  break;
        case 0xBE: _load(X,addr);  break;

        case 0xA0: _load(Y,addr);  break;
        case 0xA4: _load(Y,addr);   break;
        case 0xB4: _load(Y,addr);  break;
        case 0xAC: _load(Y,addr);  break;
        case 0xBC: _load(Y,addr);  break;

        case 0x85: _store(ACC,addr);   break;
        case 0x95: _store(ACC,addr);  break;
        case 0x8D: _store(ACC,addr);  break;
        case 0x9D: _store(ACC,addr);  break;
        case 0x99: _store(ACC,addr);  break;
        case 0x81: _store(ACC,addr); break;
        case 0x91: _store(ACC,addr); break;

        case 0x86: _store(X,addr);   break;
        case 0x96: _store(X,addr);  break;
        case 0x8E: _store(X,addr);  break;

        case 0x84: _store(Y,addr);   break;
        case 0x94: _store(Y,addr);  break;
        case 0x8C: _store(Y,addr);  break;

                   /* transfer */
        case 0xAA: _mov(X,ACC); break; // TAX
        case 0xA8: _mov(Y,ACC); break; // TAY
        case 0x8A: _mov(ACC,X); break; // TXA
        case 0x98: _mov(ACC,Y); break; // TYA
        case 0xBA: _mov(X,SP); break; // TSX
        case 0x9A: SP=X; break; // TXS

                   /* shift */
        case 0x0A: _asla();       break;
        case 0x06: _asl(addr);  break;
        case 0x16: _asl(addr); break;
        case 0x0E: _asl(addr); break;
        case 0x1E: _asl(addr); break;

        case 0x4A: _lsra();       break;
        case 0x46: _lsr(addr);  break;
        case 0x56: _lsr(addr); break;
        case 0x4E: _lsr(addr); break;
        case 0x5E: _lsr(addr); break;

        case 0x2A: _rola();       break;
        case 0x26: _rol(addr);  break;
        case 0x36: _rol(addr); break;
        case 0x2E: _rol(addr); break;
        case 0x3E: _rol(addr); break;

        case 0x6A: _rora();       break;
        case 0x66: _ror(addr);  break;
        case 0x76: _ror(addr); break;
        case 0x6E: _ror(addr); break;
        case 0x7E: _ror(addr); break;

        case 0xE6: _inc(addr);  break;
        case 0xF6: _inc(addr); break;
        case 0xEE: _inc(addr); break;
        case 0xFE: _inc(addr); break;
        case 0xE8: _incr(X); break;
        case 0xC8: _incr(Y); break;

        case 0xC6: _dec(addr);  break;
        case 0xD6: _dec(addr); break;
        case 0xCE: _dec(addr); break;
        case 0xDE: _dec(addr); break;
        case 0xCA: _decr(X); break;
        case 0x88: _decr(Y); break;

                   /* branch */
        case 0x90: _bra(!CFlag); break; // BCC
        case 0xB0: _bra( CFlag); break; // BCS
        case 0xD0: _bra(!ZFlag); break; // BNE
        case 0xF0: _bra( ZFlag); break; // BEQ
        case 0x10: _bra(!NFlag); break; // BPL
        case 0x30: _bra( NFlag); break; // BMI
        case 0x50: _bra(!VFlag); break; // BVC
        case 0x70: _bra( VFlag); break; // BVS

                   /* jump / call / return */
        case 0x4C: PC=addr; break; // JMP abs
        case 0x6C: PC=addr; break; // JMP (abs)

        case 0x20: PC=addr; break; // JSR

        case 0x60: PC=_pop16()+1; break; // RTS
        case 0x40: _unbindFlags(_pop8());PC=_pop16(); break; // RTI

                   /* flag */
        case 0x38: CFlag=1; break; // SEC
        case 0xF8: DFlag=1; break; // SED
        case 0x78: IFlag=1; break; // SEI

        case 0x18: CFlag=0; break; // CLC
        case 0xD8: DFlag=0; break; // CLD
        case 0x58: IFlag=0; break; // CLI (この瞬間に割り込みがかかるかも知れん…)
        case 0xB8: VFlag=0; break; // CLV

                   /* stack */
        case 0x48: _push8(ACC); break; // PHA
        case 0x08: _push8(_bindFlags()); break; // PHP
        case 0x68: ACC=_pop8();NFlag=ACC>>7;ZFlag=ACC==0; break; // PLA
        case 0x28: _unbindFlags(_pop8()); break; // PLP

                   /* others */
        case 0x00: // BRK
                   BFlag=1;
                   PC++;
                   exec_irq(IRQ, WRAM, PPU_RAM, SP_RAM);
                   break;

        case 0xEA: break; // NOP

        default:
                   //printf("undefined opcode: %02x", opc);
                   break;
    }
    op = false;
}

void CPU::exec_irq(int cause, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM){
    uint16_t vect;

    switch(cause){  
        case RESET: vect = 0xFFFC; break;
        case NMI:   vect = 0xFFFA; break;
        case IRQ:   vect = 0xFFFE; break;
        default:    vect = 0xFFFE; break;
    }
    //if(log) cout << "interrupt occured!! jmp to " << vect << endl;

    _push16(PC);
    _push8(_bindFlags());
    IFlag = 1;
    PC = read_mem16(vect, WRAM, PPU_RAM);
    op = false;
}
/*
void CPU::test_exec(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM){
    IR = read(PC, WRAM, PPU_RAM);
    //dump_regs(IR);
    PC++;
    uint16_t opr_pc = PC;

    uint8_t aaa = (IR >> 5) & 0b111;
    uint8_t bbb = (IR >> 2) & 0b111;
    uint8_t cc = IR & 0b11;

    bool aflag[8];
    bool bflag[8];
    bool cflag[4];

    DEC_3_8(aflag, aaa, 1);
    DEC_3_8(bflag, bbb, 1);
    DEC_2_4(cflag, cc,  1);

    bool LDY = (IR == 0xA0 || IR == 0xA4 || IR == 0xB4 || IR == 0xAC || IR == 0xBC);
    bool CPX = (IR == 0xE0 || IR == 0xE4 || IR == 0xEC);
    bool CPY = (IR == 0xC0 || IR == 0xC4 || IR == 0xCC);
    bool JSR = (IR == 0x20);
    bool acc, imm, zero, zero_x, zero_y, abs, abs_x, abs_y, ind, ind_x, ind_y;

    if(JSR) _push16(PC+1);

    acc = cflag[2] & bflag[2] & !((aaa >> 2) & 1);

    imm = (cflag[1] & bflag[2]) |
        (cflag[2] & bflag[0]) |
        (cflag[0] & bflag[0] & (LDY | CPY | CPX));

    zero = (cflag[1] & bflag[1])|
           (cflag[2] & bflag[1])|
           (cflag[0] & bflag[1]);

    zero_x = (cflag[1] & bflag[5])|
             (!(aflag[4] | aflag[5]) & cflag[2] & bflag[5])|   //STX, LDX
             (cflag[0] & bflag[5]);

    zero_y = (aflag[4] | aflag[5]) & cflag[2] & bflag[5];    //STX. LDX

    abs = (cflag[1] & bflag[3])|
          (cflag[2] & bflag[3])|
          (!aflag[3] & cflag[0] & bflag[3]);   //JMP(indirect)

    abs_x = ((cflag[1] & bflag[7])|
            (cflag[0] & bflag[7]))|
            (!aflag[5] & cflag[2] & bflag[7]);     //LDX

    abs_y = (cflag[1] & bflag[6]) |
            (aflag[5] & cflag[2] & bflag[7]);      //LDX

    ind = aflag[3] & cflag[0] & bflag[3];
    ind_x = cflag[1] & bflag[0];
    ind_y = cflag[1] & bflag[4];

    if(imm) addr = _imm(opr_pc, WRAM, PPU_RAM);
    else if(zero) addr = _zp(opr_pc, WRAM, PPU_RAM);
    else if(zero_x) addr = _zpx(opr_pc, WRAM, PPU_RAM);
    else if(zero_y) addr = _zpy(opr_pc, WRAM, PPU_RAM);
    else if(abs) addr = _abs(opr_pc, WRAM, PPU_RAM);
    else if(abs_x) addr = _abx(opr_pc, WRAM, PPU_RAM);
    else if(abs_y) addr = _aby(opr_pc, WRAM, PPU_RAM);
    else if(ind) addr = _absi(opr_pc, WRAM, PPU_RAM);
    else if(ind_x) addr = _zpxi(opr_pc, WRAM, PPU_RAM);
    else if(ind_y) addr = _zpiy(opr_pc, WRAM, PPU_RAM);
    //else printf("addressing ERROR\n");

    uint8_t IRH = ((IR >> 4) & 0b1111);
    uint8_t IRL = (IR & 0b1111);

    if(IR == 0x00){
        BFlag=1;
        PC++;
        exec_irq(IRQ, WRAM, PPU_RAM, SP_RAM);
    }
    else if(IR == 0x20)
        //PC = addr;
        PC = _abs(opr_pc, WRAM, PPU_RAM);
    else if(IR == 0x40){
        _unbindFlags(_pop8());
        PC=_pop16();
    }
    else if(IR == 0x60){
        PC=_pop16()+1;
    }
    else if(IRL == 0x08){
        switch(IRH){
            case 0b0000:_push8(_bindFlags()); break;
            case 0b0010:_unbindFlags(_pop8()); break; 
            case 0b0100:_push8(ACC); break;
            case 0b0110:ACC=_pop8();NFlag=ACC>>7;ZFlag=ACC==0; break;
            case 0b1000:_decr(Y); break;
            case 0b1010:_mov(Y,ACC); break;
            case 0b1100:_incr(Y); break;
            case 0b1110:_incr(X); break;
            case 0b0001:CFlag = 0; break;
            case 0b0011:DFlag = 1; break;
            case 0b0101:IFlag = 0; break;
            case 0b0111:IFlag = 1; break;
            case 0b1001:_mov(ACC,Y);; break;
            case 0b1011:VFlag = 0; break;
            case 0b1101:DFlag = 0; break;
            case 0b1111:DFlag = 1; break;
        }
    }
    else if(IRL == 0x0A && !acc){
        switch(IRH){
            case 0b1000:_mov(ACC,X); break; 
            case 0b1001:SP=X; break;
            case 0b1010:_mov(X,ACC); break;
            case 0b1011:_mov(X,SP); break;
            case 0b1100:_decr(X); break;
            case 0b1110: break;
            default:printf("Error1\n");
        }
    }
    else if((IR & 0b11111) == 0x10){
        switch((IR >> 5) & 0b111){
            case 0b000:_bra(!NFlag); break; 
            case 0b001:_bra( NFlag); break;
            case 0b010:_bra(!VFlag); break;
            case 0b011:_bra( VFlag); break;
            case 0b100:_bra(!CFlag); break;
            case 0b101:_bra( CFlag); break;
            case 0b110:_bra(!ZFlag); break;
            case 0b111:_bra( ZFlag); break;
        }
    }
    else{
        switch(cc){
            case 0b01:
                switch(aaa){
                    case 0b000:_ora(addr); break; 
                    case 0b001:_and(addr); break;
                    case 0b010:_eor(addr); break;
                    case 0b011:_adc(addr); break;
                    case 0b100:_store(ACC, addr); break;
                    case 0b101:_load(ACC, addr); break;
                    case 0b110:_cmp(ACC, addr); break;
                    case 0b111:_sbc(addr); break;
                }
                break;
            case 0b10:
                switch(aaa){
                    case 0b000: if(acc) _asla() else _asl(addr); break;
                    case 0b001: if(acc) _rola() else _rol(addr); break;
                    case 0b010: if(acc) _lsra() else _lsr(addr); break;
                    case 0b011: if(acc) _rora() else _ror(addr); break;
                    case 0b100:_store(X, addr); break;
                    case 0b101:_load(X, addr); break;
                    case 0b110:_dec(addr); break;
                    case 0b111:_dec(addr); break;
                }
                break;
            case 0b00:
                switch(aaa){
                    case 0b001:_bit(addr); break;
                    case 0b010:PC = addr; break;
                    case 0b011:PC = addr; break;
                    case 0b100:_store(Y, addr); break; 
                    case 0b101:_load(Y, addr); break;
                    case 0b110:_cmp(Y, addr); break;
                    case 0b111:_cmp(X, addr); break;
                    default:printf("Error2\n");
                }
                break;
            case 0b11:printf("Error3\n"); break;
        }
    }

}

void CPU::DEC_2_4(bool* dout, uint8_t din, bool en){
    dout[0]  = en & (din == 0b00);
    dout[1]  = en & (din == 0b01);
    dout[2]  = en & (din == 0b10);
    dout[3]  = en & (din == 0b11);
}

void CPU::DEC_3_8(bool* dout, uint8_t din, bool en){
    uint8_t din0 = din & 0b11;
    bool din1 = (din >> 2) & 1;
    DEC_2_4(dout, din0, en & ~din1);
    DEC_2_4(dout+4, din0, en & din1);
}
*/


