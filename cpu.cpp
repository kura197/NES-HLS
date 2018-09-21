#include "cpu.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include "instr.h"

//void CPU::dump_regs(uint8_t insn){
//    printf("%04x %02x   A:%02x X:%02x Y:%02x P:%02x SP:%02x\n",
//                    PC, insn, ACC, X, Y, _bindFlags(), SP);
//
//}

uint16_t CPU::read_mem16(uint16_t addr, uint8_t* WRAM, uint8_t* PPU_RAM){
    uint16_t rddata;
    rddata = read(addr, WRAM, PPU_RAM);
    rddata = rddata | ((uint16_t)read(addr+1, WRAM, PPU_RAM) << 8);
    return rddata;
}

void CPU::write_mem16(uint16_t addr, uint16_t data, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM){
    write(addr, (uint8_t)data, WRAM, PPU_RAM, SP_RAM);
    write(addr+1, (uint8_t)(data >> 8), WRAM, PPU_RAM, SP_RAM);
}

void CPU::push8(uint8_t data, uint8_t* WRAM){
    WRAM[0x100|(uint8_t)(SP--)] = data;
}

uint8_t CPU::pop8(uint8_t* WRAM){
    return WRAM[0x100|(uint8_t)(++SP)];
}

void CPU::push16(uint16_t data, uint8_t* WRAM){
    WRAM[0x100|(uint8_t)(SP--)] = (uint8_t)(data >> 8);
    WRAM[0x100|(uint8_t)(SP--)] = (uint8_t)data;
}

uint16_t CPU::pop16(uint8_t* WRAM){
    uint16_t data;
    data = WRAM[0x100|(uint8_t)(++SP)];
    data |= (uint16_t)WRAM[0x100|(uint8_t)(++SP)] << 8;
    return data;
}

uint8_t CPU::norm_read8(uint16_t addr, uint8_t* WRAM){
    return WRAM[addr];
}

uint16_t CPU::norm_read16(uint16_t addr, uint8_t* WRAM){
    uint16_t data;
    data = WRAM[addr];
    data |= (uint16_t)WRAM[addr+1] << 8;
    return data;
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
    //if(!op){
    //    if(reset_line) exec_irq(RESET, WRAM, PPU_RAM, SP_RAM); reset_line = false;
    //    if(nmi_line) exec_irq(NMI, WRAM, PPU_RAM, SP_RAM); nmi_line = false;
    //    exec_addressing(WRAM, PPU_RAM, SP_RAM);
    //}
    //else exec_op(WRAM, PPU_RAM, SP_RAM);
    
    //if(reset_line) exec_irq(RESET, WRAM, PPU_RAM, SP_RAM); reset_line = false;
    //if(nmi_line) exec_irq(NMI, WRAM, PPU_RAM, SP_RAM); nmi_line = false;
    //test_exec(WRAM, PPU_RAM, SP_RAM);

    if(reset_line) exec_irq(RESET, WRAM, PPU_RAM, SP_RAM); reset_line = false;
    if(nmi_line) exec_irq(NMI, WRAM, PPU_RAM, SP_RAM); nmi_line = false;
    execution(WRAM, PPU_RAM, SP_RAM);
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

    //_push16(PC);
    //_push8(_bindFlags());
    push16(PC, WRAM);
    push8(_bindFlags(), WRAM);
    IFlag = 1;
    //PC = read_mem16(vect, WRAM, PPU_RAM);
    PC = WRAM[vect];
    PC |= (uint16_t)WRAM[vect+1] << 8;
    //op = false;
}

#define set_mode_false {  \
    imm = false;   \
    zp = false;    \
    zpx = false;   \
    zpy = false;   \
    abs = false;   \
    abx = false;   \
    aby = false;   \
    zpxi = false;  \
    zpiy = false;  \
    absi = false;  \
    imp = false;   \
}

#define set_op_false { \
    op_adc = false;   \
    op_sbc = false;   \
    op_cmp = false;   \
    op_and = false;   \
    op_ora = false;   \
    op_eor = false;   \
    op_bit = false;   \
    op_load = false;  \
    op_store = false; \
    op_mov = false;   \
    op_asl = false;   \
    op_lsr = false;   \
    op_rol = false;   \
    op_ror = false;   \
    op_inc = false;   \
    op_dec = false;   \
    op_bra = false;   \
    op_jmp = false;   \
    op_jsr = false;   \
    op_rts = false;   \
    op_rti = false;   \
}

void CPU::execution(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM){
    bool imm, zp, zpx, zpy, abs, abx, aby, zpxi, zpiy, absi, imp;
    bool op_adc, op_sbc, op_cmp, op_and, op_ora, op_eor, op_bit;
    bool op_load, op_store, op_mov, op_asl, op_lsr, op_rol, op_ror;
    bool op_inc, op_dec, op_bra, op_jmp, op_jsr, op_rts, op_rti;
    bool acc = false, x = false, y = false;
    set_mode_false;
    set_op_false;

    uint16_t addr;

    //uint8_t IR = read(PC, WRAM, PPU_RAM);
    uint8_t IR = WRAM[PC];
    //dump_regs(IR);
    PC++;
    uint16_t opr_pc = PC;

    switch(IR){
        /* ALU */
        case 0x69: op_adc = true; imm = true; break;
        case 0x65: op_adc = true; zp = true; break;
        case 0x75: op_adc = true; zpx = true; break;
        case 0x6D: op_adc = true; abs = true; break;
        case 0x7D: op_adc = true; abx = true; break;
        case 0x79: op_adc = true; aby = true; break;
        case 0x61: op_adc = true; zpxi = true; break;
        case 0x71: op_adc = true; zpiy = true; break;

        case 0xE9: op_sbc = true; imm = true; break;
        case 0xE5: op_sbc = true; zp = true;  break;
        case 0xF5: op_sbc = true; zpx = true; break;
        case 0xED: op_sbc = true; abs = true; break;
        case 0xFD: op_sbc = true; abx = true; break;
        case 0xF9: op_sbc = true; aby = true; break;
        case 0xE1: op_sbc = true; zpxi = true; break;
        case 0xF1: op_sbc = true; zpiy = true; break;

        case 0xC9: op_cmp = true; acc = true; imm = true;  break;
        case 0xC5: op_cmp = true; acc = true; zp = true;   break;
        case 0xD5: op_cmp = true; acc = true; zpx = true;  break;
        case 0xCD: op_cmp = true; acc = true; abs = true;  break;
        case 0xDD: op_cmp = true; acc = true; abx = true;  break;
        case 0xD9: op_cmp = true; acc = true; aby = true;  break;
        case 0xC1: op_cmp = true; acc = true; zpxi = true; break;
        case 0xD1: op_cmp = true; acc = true; zpiy = true; break;

        case 0xE0: op_cmp = true; x = true; imm = true; break;
        case 0xE4: op_cmp = true; x = true; zp = true; break;
        case 0xEC: op_cmp = true; x = true; abs = true; break;

        case 0xC0: op_cmp = true; y = true; imm = true; break;
        case 0xC4: op_cmp = true; y = true; zp = true;  break;
        case 0xCC: op_cmp = true; y = true; abs = true; break;

        case 0x29: op_and = true; imm = true;  break;
        case 0x25: op_and = true; zp = true;   break;
        case 0x35: op_and = true; zpx = true;  break;
        case 0x2D: op_and = true; abs = true;  break;
        case 0x3D: op_and = true; abx = true;  break;
        case 0x39: op_and = true; aby = true;  break;
        case 0x21: op_and = true; zpxi = true; break;
        case 0x31: op_and = true; zpiy = true; break;

        case 0x09: op_ora = true; imm = true;  break;
        case 0x05: op_ora = true; zp = true;   break;
        case 0x15: op_ora = true; zpx = true;  break;
        case 0x0D: op_ora = true; abs = true;  break;
        case 0x1D: op_ora = true; abx = true;  break;
        case 0x19: op_ora = true; aby = true;  break;
        case 0x01: op_ora = true; zpxi = true; break;
        case 0x11: op_ora = true; zpiy = true; break;

        case 0x49: op_eor = true; imm = true;  break;
        case 0x45: op_eor = true; zp = true;   break;
        case 0x55: op_eor = true; zpx = true;  break;
        case 0x4D: op_eor = true; abs = true;  break;
        case 0x5D: op_eor = true; abx = true;  break;
        case 0x59: op_eor = true; aby = true;  break;
        case 0x41: op_eor = true; zpxi = true; break;
        case 0x51: op_eor = true; zpiy = true; break;

        case 0x24: op_bit = true; zp = true;  break;
        case 0x2C: op_bit = true; abs = true;  break;

                   /* laod / store */
        case 0xA9: op_load = true; acc = true; imm = true;  break;
        case 0xA5: op_load = true; acc = true; zp = true;   break;
        case 0xB5: op_load = true; acc = true; zpx = true;  break;
        case 0xAD: op_load = true; acc = true; abs = true;  break;
        case 0xBD: op_load = true; acc = true; abx = true;  break;
        case 0xB9: op_load = true; acc = true; aby = true;  break;
        case 0xA1: op_load = true; acc = true; zpxi = true; break;
        case 0xB1: op_load = true; acc = true; zpiy = true; break;

        case 0xA2: op_load = true; x = true; imm = true;  break;
        case 0xA6: op_load = true; x = true; zp = true;  break;
        case 0xB6: op_load = true; x = true; zpy = true;  break;
        case 0xAE: op_load = true; x = true; abs = true;  break;
        case 0xBE: op_load = true; x = true; aby = true;  break;

        case 0xA0: op_load = true; y = true; imm = true;  break;
        case 0xA4: op_load = true; y = true; zp = true;  break;
        case 0xB4: op_load = true; y = true; zpx = true;  break;
        case 0xAC: op_load = true; y = true; abs = true;  break;
        case 0xBC: op_load = true; y = true; abx = true;  break;

        case 0x85: op_store = true; acc = true; zp = true;   break;
        case 0x95: op_store = true; acc = true; zpx = true;  break;
        case 0x8D: op_store = true; acc = true; abs = true;  break;
        case 0x9D: op_store = true; acc = true; abx = true;  break;
        case 0x99: op_store = true; acc = true; aby = true;  break;
        case 0x81: op_store = true; acc = true; zpxi = true; break;
        case 0x91: op_store = true; acc = true; zpiy = true; break;

        case 0x86: op_store = true; x = true; zp = true;  break;
        case 0x96: op_store = true; x = true; zpy = true;  break;
        case 0x8E: op_store = true; x = true; abs = true;  break;

        case 0x84: op_store = true; y = true; zp = true;  break;
        case 0x94: op_store = true; y = true; zpx = true;  break;
        case 0x8C: op_store = true; y = true; abs = true;  break;

                   /* transfer */
        case 0xAA: _mov(X,ACC); break; // TAX
        case 0xA8: _mov(Y,ACC); break; // TAY
        case 0x8A: _mov(ACC,X); break; // TXA
        case 0x98: _mov(ACC,Y); break; // TYA
        case 0xBA: _mov(X,SP);  break; // TSX
        case 0x9A: SP=X; break; // TXS

                   /* shift */
        case 0x0A: op_asl = true; imp = true; break;
        case 0x06: op_asl = true; zp = true; break;
        case 0x16: op_asl = true; zpx = true; break;
        case 0x0E: op_asl = true; abs = true; break;
        case 0x1E: op_asl = true; abx = true; break;

        case 0x4A: op_lsr = true; imp = true;  break;
        case 0x46: op_lsr = true; zp = true;  break;
        case 0x56: op_lsr = true; zpx = true; break;
        case 0x4E: op_lsr = true; abs = true; break;
        case 0x5E: op_lsr = true; abx = true; break;

        case 0x2A: op_rol = true; imp = true;  break;
        case 0x26: op_rol = true; zp = true;  break;
        case 0x36: op_rol = true; zpx = true; break;
        case 0x2E: op_rol = true; abs = true; break;
        case 0x3E: op_rol = true; abx = true; break;

        case 0x6A: op_ror = true; imp = true;  break;
        case 0x66: op_ror = true; zp = true;  break;
        case 0x76: op_ror = true; zpx = true; break;
        case 0x6E: op_ror = true; abs = true; break;
        case 0x7E: op_ror = true; abx = true; break;

        case 0xE6: op_inc = true; zp = true; break;
        case 0xF6: op_inc = true; zpx = true; break;
        case 0xEE: op_inc = true; abs = true; break;
        case 0xFE: op_inc = true; abx = true; break;
        case 0xE8: _incr(X); break;
        case 0xC8: _incr(Y); break;

        case 0xC6: op_dec = true; zp = true;  break;
        case 0xD6: op_dec = true; zpx = true; break;
        case 0xCE: op_dec = true; abs = true; break;
        case 0xDE: op_dec = true; abx = true; break;
        case 0xCA: _decr(X);  break;
        case 0x88: _decr(Y);  break;

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
        case 0x4C: op_jmp = true; abs  = true; break; // JMP abs
        case 0x6C: op_jmp = true; absi = true; break; // JMP (abs)

        case 0x20: op_jsr = true; abs = true; break; // JSR

        case 0x60: op_rts = true; break; // RTS
        case 0x40: op_rti = true; break; // RTI

                   /* flag */
        case 0x38: CFlag=1;  break; // SEC
        case 0xF8: DFlag=1;  break; // SED
        case 0x78: IFlag=1;  break; // SEI

        case 0x18: CFlag=0;  break; // CLC
        case 0xD8: DFlag=0;  break; // CLD
        case 0x58: IFlag=0;  break; // CLI 
        case 0xB8: VFlag=0;  break; // CLV

                   /* stack */
        //case 0x48: _push8(ACC);  break; // PHA
        case 0x48: push8(ACC, WRAM);  break; // PHA
        //case 0x08: _push8(_bindFlags());  break; // PHP
        case 0x08: push8(_bindFlags(), WRAM);  break; // PHP
        //case 0x68: ACC=_pop8();NFlag=ACC>>7;ZFlag=ACC==0;break; // PLA
        case 0x68: ACC=pop8(WRAM);NFlag=ACC>>7;ZFlag=ACC==0;break; // PLA
        //case 0x28: _unbindFlags(_pop8()); break; // PLP
        case 0x28: _unbindFlags(pop8(WRAM)); break; // PLP

        //           /* others */
        //case 0x00: // BRK
        //           BFlag=1;
        //           PC++;
        //           exec_irq(IRQ, WRAM, PPU_RAM, SP_RAM);
        //           break;

        case 0xEA: break; // NOP

        default:
               //printf("undefined opcode: %02x\n", (uint)IR);
               break;
    }

    if(imm)
        addr = _imm(opr_pc, WRAM);
    else if(zp)
        addr = _zp(opr_pc, WRAM);
    else if(zpx)
        addr = _zpx(opr_pc, WRAM);
    else if(abs)
        addr = _abs(opr_pc, WRAM);
    else if(abx)
        addr = _abx(opr_pc, WRAM);
    else if(aby)
        addr = _aby(opr_pc, WRAM);
    else if(zpxi)
        addr = _zpxi(opr_pc, WRAM);
    else if(zpiy)
        addr = _zpiy(opr_pc, WRAM);
    else if(absi)
        addr = _absi(opr_pc, WRAM);
/*
    if(op_adc){
        _adc(addr);
    } 
    else if(op_sbc){
        _sbc(addr);
    } 
    else if(op_cmp){
        if(acc){
            _cmp(ACC, addr);
        }else if(x){
            _cmp(X, addr);
        }else if(y){
            _cmp(Y, addr);
        }
    } 
    else if(op_and){
        _and(addr);
    }
    else if(op_ora){
        _ora(addr);
    }
    else if(op_eor){
        _eor(addr);
    }
    else if(op_bit){
        _bit(addr);
    }
    else if(op_load){
        if(acc){
            _load(ACC, addr);
        }else if(x){
            _load(X, addr);
        }else if(y){
            _load(Y, addr);
        }
    } 
    else if(op_store){
        if(acc){
            _store(ACC, addr);
        }else if(x){
            _store(X, addr);
        }else if(y){
            _store(Y, addr);
        }
    }
    else if(op_asl){
        if(imp){
            _asla();
        }else{
            _asl(addr);
        }
    }
    else if(op_lsr){
        if(imp){
            _lsra();
        }else{
            _lsr(addr);
        }
    }
    else if(op_rol){
        if(imp){
            _rola();
        }else{
            _rol(addr);
        }
    }
    else if(op_ror){
        if(imp){
            _rora();
        }else{
            _ror(addr);
        }
    }
    else if(op_inc){
        _inc(addr);
    }
    else if(op_dec){
        _dec(addr);
    }
    else if(op_jmp){
        PC = addr;
    }
    else if(op_jsr){
        //_push16(PC-1);
        push16(PC-1, WRAM);
        PC = addr;
    }
    else if(op_rts){
        //PC=_pop16()+1;
        PC=pop16(WRAM)+1;
    }
    else if(op_rti){
        //_unbindFlags(_pop8());
        //PC=_pop16();
        _unbindFlags(pop8(WRAM));
        PC=pop16(WRAM);
    }
    //else printf("Error\n");
*/
}

