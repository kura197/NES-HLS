#include "cpu.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include "instr.h"

//void CPU::dump_regs(uint8_t insn){
//    printf("%04x %02x   A:%02x X:%02x Y:%02x P:%02x SP:%02x\n",
//                    PC, insn, ACC, X, Y, _bindFlags(), SP);
//    //printf("SP[0xFF]:%02x\tSP[0xFE]:%02x\tSP[0xFD]:%02x\n\n", read_mem8(0x1FF), read_mem8(0x1FE), read_mem8(0x1FD));
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

//void CPU::set_irq(bool signal)
//{
//  irq_line = signal;
//}
//
//void CPU::set_reset(bool signal)
//{
//  reset_line = signal;
//}

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
    //rest += 114;
    //do{
    //    if(!IFlag){
    //        if (reset_line) exec_irq(RESET, WRAM, PPU_RAM, SP_RAM),reset_line=false;
    //        else if (irq_line) exec_irq(IRQ, WRAM, PPU_RAM, SP_RAM),irq_line=false;
    //        //else if (nmi_line) exec_irq(NMI); nmi_line = false;
    //    }
    //    //if (nmi) exec_irq(NMI); nmi = false;

        uint8_t opc = read(PC, WRAM, PPU_RAM);
        //if(log) dump_regs(opc);
        PC++;
        uint16_t opr_pc = PC;
        switch(opc){
              /* ALU */
            case 0x69: _adc(_imm());  break;
            case 0x65: _adc(_zp());   break;
            case 0x75: _adc(_zpx());  break;
            case 0x6D: _adc(_abs());  break;
            case 0x7D: _adc(_abx());  break;
            case 0x79: _adc(_aby());  break;
            case 0x61: _adc(_zpxi()); break;
            case 0x71: _adc(_zpiy()); break;

            case 0xE9: _sbc(_imm());  break;
            case 0xE5: _sbc(_zp());   break;
            case 0xF5: _sbc(_zpx());  break;
            case 0xED: _sbc(_abs());  break;
            case 0xFD: _sbc(_abx());  break;
            case 0xF9: _sbc(_aby());  break;
            case 0xE1: _sbc(_zpxi()); break;
            case 0xF1: _sbc(_zpiy()); break;

            case 0xC9: _cmp(ACC,_imm());  break;
            case 0xC5: _cmp(ACC,_zp());   break;
            case 0xD5: _cmp(ACC,_zpx());  break;
            case 0xCD: _cmp(ACC,_abs());  break;
            case 0xDD: _cmp(ACC,_abx());  break;
            case 0xD9: _cmp(ACC,_aby());  break;
            case 0xC1: _cmp(ACC,_zpxi()); break;
            case 0xD1: _cmp(ACC,_zpiy()); break;

            case 0xE0: _cmp(X,_imm()); break;
            case 0xE4: _cmp(X,_zp());  break;
            case 0xEC: _cmp(X,_abs()); break;

            case 0xC0: _cmp(Y,_imm()); break;
            case 0xC4: _cmp(Y,_zp());  break;
            case 0xCC: _cmp(Y,_abs()); break;

            case 0x29: _and(_imm());  break;
            case 0x25: _and(_zp());   break;
            case 0x35: _and(_zpx());  break;
            case 0x2D: _and(_abs());  break;
            case 0x3D: _and(_abx());  break;
            case 0x39: _and(_aby());  break;
            case 0x21: _and(_zpxi()); break;
            case 0x31: _and(_zpiy()); break;

            case 0x09: _ora(_imm());  break;
            case 0x05: _ora(_zp());   break;
            case 0x15: _ora(_zpx());  break;
            case 0x0D: _ora(_abs());  break;
            case 0x1D: _ora(_abx());  break;
            case 0x19: _ora(_aby());  break;
            case 0x01: _ora(_zpxi()); break;
            case 0x11: _ora(_zpiy()); break;

            case 0x49: _eor(_imm());  break;
            case 0x45: _eor(_zp());   break;
            case 0x55: _eor(_zpx());  break;
            case 0x4D: _eor(_abs());  break;
            case 0x5D: _eor(_abx());  break;
            case 0x59: _eor(_aby());  break;
            case 0x41: _eor(_zpxi()); break;
            case 0x51: _eor(_zpiy()); break;

            case 0x24: _bit(_zp());   break;
            case 0x2C: _bit(_abs());  break;

              /* laod / store */
            case 0xA9: _load(ACC,_imm());  break;
            case 0xA5: _load(ACC,_zp());   break;
            case 0xB5: _load(ACC,_zpx());  break;
            case 0xAD: _load(ACC,_abs());  break;
            case 0xBD: _load(ACC,_abx());  break;
            case 0xB9: _load(ACC,_aby());  break;
            case 0xA1: _load(ACC,_zpxi()); break;
            case 0xB1: _load(ACC,_zpiy()); break;

            case 0xA2: _load(X,_imm());  break;
            case 0xA6: _load(X,_zp());   break;
            case 0xB6: _load(X,_zpy());  break;
            case 0xAE: _load(X,_abs());  break;
            case 0xBE: _load(X,_aby());  break;

            case 0xA0: _load(Y,_imm());  break;
            case 0xA4: _load(Y,_zp());   break;
            case 0xB4: _load(Y,_zpx());  break;
            case 0xAC: _load(Y,_abs());  break;
            case 0xBC: _load(Y,_abx());  break;

            case 0x85: _store(ACC,_zp());   break;
            case 0x95: _store(ACC,_zpx());  break;
            case 0x8D: _store(ACC,_abs());  break;
            case 0x9D: _store(ACC,_abx());  break;
            case 0x99: _store(ACC,_aby());  break;
            case 0x81: _store(ACC,_zpxi()); break;
            case 0x91: _store(ACC,_zpiy()); break;

            case 0x86: _store(X,_zp());   break;
            case 0x96: _store(X,_zpy());  break;
            case 0x8E: _store(X,_abs());  break;

            case 0x84: _store(Y,_zp());   break;
            case 0x94: _store(Y,_zpx());  break;
            case 0x8C: _store(Y,_abs());  break;

              /* transfer */
            case 0xAA: _mov(X,ACC); break; // TAX
            case 0xA8: _mov(Y,ACC); break; // TAY
            case 0x8A: _mov(ACC,X); break; // TXA
            case 0x98: _mov(ACC,Y); break; // TYA
            case 0xBA: _mov(X,SP); break; // TSX
            case 0x9A: SP=X; break; // TXS

              /* shift */
            case 0x0A: _asla();       break;
            case 0x06: _asl(_zp());  break;
            case 0x16: _asl(_zpx()); break;
            case 0x0E: _asl(_abs()); break;
            case 0x1E: _asl(_abx()); break;

            case 0x4A: _lsra();       break;
            case 0x46: _lsr(_zp());  break;
            case 0x56: _lsr(_zpx()); break;
            case 0x4E: _lsr(_abs()); break;
            case 0x5E: _lsr(_abx()); break;

            case 0x2A: _rola();       break;
            case 0x26: _rol(_zp());  break;
            case 0x36: _rol(_zpx()); break;
            case 0x2E: _rol(_abs()); break;
            case 0x3E: _rol(_abx()); break;

            case 0x6A: _rora();       break;
            case 0x66: _ror(_zp());  break;
            case 0x76: _ror(_zpx()); break;
            case 0x6E: _ror(_abs()); break;
            case 0x7E: _ror(_abx()); break;

            case 0xE6: _inc(_zp());  break;
            case 0xF6: _inc(_zpx()); break;
            case 0xEE: _inc(_abs()); break;
            case 0xFE: _inc(_abx()); break;
            case 0xE8: _incr(X); break;
            case 0xC8: _incr(Y); break;

            case 0xC6: _dec(_zp());  break;
            case 0xD6: _dec(_zpx()); break;
            case 0xCE: _dec(_abs()); break;
            case 0xDE: _dec(_abx()); break;
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
            case 0x4C: PC=_abs() ; break; // JMP abs
            case 0x6C: PC=_absi(); break; // JMP (abs)

            case 0x20: _push16(PC+1);PC=_abs(); break; // JSR

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
    //}while(rest > 0);

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
}
