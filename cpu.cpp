#include "cpu.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include "instr.h"

void CPU::dump_regs(uint8_t insn){
    printf("%04x %02x   A:%02x X:%02x Y:%02x P:%02x SP:%02x\n",
                    PC, insn, ACC, X, Y, _bindFlags(), SP);
    //printf("SP[0xFF]:%02x\tSP[0xFE]:%02x\tSP[0xFD]:%02x\n\n", read_mem8(0x1FF), read_mem8(0x1FE), read_mem8(0x1FD));
}

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

void CPU::set_irq(bool signal)
{
  irq_line = signal;
}

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
    //rest += clk;
    rest += 114;
    do{
        if(!IFlag){
            if (reset_line) exec_irq(RESET, WRAM, PPU_RAM, SP_RAM),reset_line=false;
            else if (irq_line) exec_irq(IRQ, WRAM, PPU_RAM, SP_RAM),irq_line=false;
            //else if (nmi_line) exec_irq(NMI); nmi_line = false;
        }
        //if (nmi) exec_irq(NMI); nmi = false;

        uint8_t opc = read(PC, WRAM, PPU_RAM);
        if(log) dump_regs(opc);
        PC++;
        uint16_t opr_pc = PC;
        switch(opc){
              /* ALU */
            case 0x69: _adc(2,_imm());  break;
            case 0x65: _adc(3,_zp());   break;
            case 0x75: _adc(4,_zpx());  break;
            case 0x6D: _adc(4,_abs());  break;
            case 0x7D: _adc(4,_abx());  break;
            case 0x79: _adc(4,_aby());  break;
            case 0x61: _adc(6,_zpxi()); break;
            case 0x71: _adc(5,_zpiy()); break;

            case 0xE9: _sbc(2,_imm());  break;
            case 0xE5: _sbc(3,_zp());   break;
            case 0xF5: _sbc(4,_zpx());  break;
            case 0xED: _sbc(4,_abs());  break;
            case 0xFD: _sbc(4,_abx());  break;
            case 0xF9: _sbc(4,_aby());  break;
            case 0xE1: _sbc(6,_zpxi()); break;
            case 0xF1: _sbc(5,_zpiy()); break;

            case 0xC9: _cmp(2,ACC,_imm());  break;
            case 0xC5: _cmp(3,ACC,_zp());   break;
            case 0xD5: _cmp(4,ACC,_zpx());  break;
            case 0xCD: _cmp(4,ACC,_abs());  break;
            case 0xDD: _cmp(4,ACC,_abx());  break;
            case 0xD9: _cmp(4,ACC,_aby());  break;
            case 0xC1: _cmp(6,ACC,_zpxi()); break;
            case 0xD1: _cmp(5,ACC,_zpiy()); break;

            case 0xE0: _cmp(2,X,_imm()); break;
            case 0xE4: _cmp(2,X,_zp());  break;
            case 0xEC: _cmp(3,X,_abs()); break;

            case 0xC0: _cmp(2,Y,_imm()); break;
            case 0xC4: _cmp(2,Y,_zp());  break;
            case 0xCC: _cmp(3,Y,_abs()); break;

            case 0x29: _and(2,_imm());  break;
            case 0x25: _and(3,_zp());   break;
            case 0x35: _and(4,_zpx());  break;
            case 0x2D: _and(4,_abs());  break;
            case 0x3D: _and(4,_abx());  break;
            case 0x39: _and(4,_aby());  break;
            case 0x21: _and(6,_zpxi()); break;
            case 0x31: _and(5,_zpiy()); break;

            case 0x09: _ora(2,_imm());  break;
            case 0x05: _ora(3,_zp());   break;
            case 0x15: _ora(4,_zpx());  break;
            case 0x0D: _ora(4,_abs());  break;
            case 0x1D: _ora(4,_abx());  break;
            case 0x19: _ora(4,_aby());  break;
            case 0x01: _ora(6,_zpxi()); break;
            case 0x11: _ora(5,_zpiy()); break;

            case 0x49: _eor(2,_imm());  break;
            case 0x45: _eor(3,_zp());   break;
            case 0x55: _eor(4,_zpx());  break;
            case 0x4D: _eor(4,_abs());  break;
            case 0x5D: _eor(4,_abx());  break;
            case 0x59: _eor(4,_aby());  break;
            case 0x41: _eor(6,_zpxi()); break;
            case 0x51: _eor(5,_zpiy()); break;

            case 0x24: _bit(3,_zp());   break;
            case 0x2C: _bit(4,_abs());  break;

              /* laod / store */
            case 0xA9: _load(2,ACC,_imm());  break;
            case 0xA5: _load(3,ACC,_zp());   break;
            case 0xB5: _load(4,ACC,_zpx());  break;
            case 0xAD: _load(4,ACC,_abs());  break;
            case 0xBD: _load(4,ACC,_abx());  break;
            case 0xB9: _load(4,ACC,_aby());  break;
            case 0xA1: _load(6,ACC,_zpxi()); break;
            case 0xB1: _load(5,ACC,_zpiy()); break;

            case 0xA2: _load(2,X,_imm());  break;
            case 0xA6: _load(3,X,_zp());   break;
            case 0xB6: _load(4,X,_zpy());  break;
            case 0xAE: _load(4,X,_abs());  break;
            case 0xBE: _load(4,X,_aby());  break;

            case 0xA0: _load(2,Y,_imm());  break;
            case 0xA4: _load(3,Y,_zp());   break;
            case 0xB4: _load(4,Y,_zpx());  break;
            case 0xAC: _load(4,Y,_abs());  break;
            case 0xBC: _load(4,Y,_abx());  break;

            case 0x85: _store(3,ACC,_zp());   break;
            case 0x95: _store(4,ACC,_zpx());  break;
            case 0x8D: _store(4,ACC,_abs());  break;
            case 0x9D: _store(5,ACC,_abx());  break;
            case 0x99: _store(5,ACC,_aby());  break;
            case 0x81: _store(6,ACC,_zpxi()); break;
            case 0x91: _store(6,ACC,_zpiy()); break;

            case 0x86: _store(3,X,_zp());   break;
            case 0x96: _store(4,X,_zpy());  break;
            case 0x8E: _store(4,X,_abs());  break;

            case 0x84: _store(3,Y,_zp());   break;
            case 0x94: _store(4,Y,_zpx());  break;
            case 0x8C: _store(4,Y,_abs());  break;

              /* transfer */
            case 0xAA: _mov(2,X,ACC); break; // TAX
            case 0xA8: _mov(2,Y,ACC); break; // TAY
            case 0x8A: _mov(2,ACC,X); break; // TXA
            case 0x98: _mov(2,ACC,Y); break; // TYA
            case 0xBA: _mov(2,X,SP); break; // TSX
            case 0x9A: SP=X;rest-=2; break; // TXS

              /* shift */
            case 0x0A: _asla(2);       break;
            case 0x06: _asl(5,_zp());  break;
            case 0x16: _asl(6,_zpx()); break;
            case 0x0E: _asl(6,_abs()); break;
            case 0x1E: _asl(7,_abx()); break;

            case 0x4A: _lsra(2);       break;
            case 0x46: _lsr(5,_zp());  break;
            case 0x56: _lsr(6,_zpx()); break;
            case 0x4E: _lsr(6,_abs()); break;
            case 0x5E: _lsr(7,_abx()); break;

            case 0x2A: _rola(2);       break;
            case 0x26: _rol(5,_zp());  break;
            case 0x36: _rol(6,_zpx()); break;
            case 0x2E: _rol(6,_abs()); break;
            case 0x3E: _rol(7,_abx()); break;

            case 0x6A: _rora(2);       break;
            case 0x66: _ror(5,_zp());  break;
            case 0x76: _ror(6,_zpx()); break;
            case 0x6E: _ror(6,_abs()); break;
            case 0x7E: _ror(7,_abx()); break;

            case 0xE6: _inc(5,_zp());  break;
            case 0xF6: _inc(6,_zpx()); break;
            case 0xEE: _inc(6,_abs()); break;
            case 0xFE: _inc(7,_abx()); break;
            case 0xE8: _incr(2,X); break;
            case 0xC8: _incr(2,Y); break;

            case 0xC6: _dec(5,_zp());  break;
            case 0xD6: _dec(6,_zpx()); break;
            case 0xCE: _dec(6,_abs()); break;
            case 0xDE: _dec(7,_abx()); break;
            case 0xCA: _decr(2,X); break;
            case 0x88: _decr(2,Y); break;

              /* branch */
            case 0x90: _bra(2,!CFlag); break; // BCC
            case 0xB0: _bra(2, CFlag); break; // BCS
            case 0xD0: _bra(2,!ZFlag); break; // BNE
            case 0xF0: _bra(2, ZFlag); break; // BEQ
            case 0x10: _bra(2,!NFlag); break; // BPL
            case 0x30: _bra(2, NFlag); break; // BMI
            case 0x50: _bra(2,!VFlag); break; // BVC
            case 0x70: _bra(2, VFlag); break; // BVS

              /* jump / call / return */
            case 0x4C: PC=_abs() ;rest-=3; break; // JMP abs
            case 0x6C: PC=_absi();rest-=5; break; // JMP (abs)

            case 0x20: _push16(PC+1);PC=_abs();rest-=6; break; // JSR

            case 0x60: PC=_pop16()+1;rest-=6; break; // RTS
            case 0x40: _unbindFlags(_pop8());PC=_pop16();rest-=6; break; // RTI

              /* flag */
            case 0x38: CFlag=1;rest-=2; break; // SEC
            case 0xF8: DFlag=1;rest-=2; break; // SED
            case 0x78: IFlag=1;rest-=2; break; // SEI

            case 0x18: CFlag=0;rest-=2; break; // CLC
            case 0xD8: DFlag=0;rest-=2; break; // CLD
            case 0x58: IFlag=0;rest-=2; break; // CLI (この瞬間に割り込みがかかるかも知れん…)
            case 0xB8: VFlag=0;rest-=2; break; // CLV

              /* stack */
            case 0x48: _push8(ACC);rest-=3; break; // PHA
            case 0x08: _push8(_bindFlags());rest-=3; break; // PHP
            case 0x68: ACC=_pop8();NFlag=ACC>>7;ZFlag=ACC==0;rest-=4; break; // PLA
            case 0x28: _unbindFlags(_pop8());rest-=4; break; // PLP

              /* others */
            case 0x00: // BRK
              BFlag=1;
              PC++;
              exec_irq(IRQ, WRAM, PPU_RAM, SP_RAM);
              break;

            case 0xEA: rest-=2; break; // NOP

            default:
              printf("undefined opcode: %02x", opc);
              break;
        }
    }while(rest > 0);

    return scr;
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
    rest -= 7;
}
