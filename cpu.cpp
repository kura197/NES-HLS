#include "cpu.h"
#include <cstdio>
#include "instr.h"
#include "HLS/hls.h"

//const bool enlog = true;
const bool enlog = false;

void CPU::dump_regs(uint8_t insn){
    uint8_t flag = bindFlags();
    uint16_t pc = PC;
    uint8_t acc = ACC;
    uint8_t x = X;
    uint8_t y = Y;
    uint8_t sp = SP;
    //printf("%04x %02x   A:%02x X:%02x Y:%02x P:%02x SP:%02x\n",
    //                pc, insn, acc, x, y, flag, sp);
}


uint8_t CPU::read_mem8(uint16_t addr, uint8_t* WRAM, uint8_t* PROM){
    uint8_t data = 0;
    if((addr >> 15) & 1)
        data = read_prom8(addr, PROM);
    else data = WRAM[addr&0x7FF]; //ok??
    return data;
}

uint8_t CPU::read_wram8(uint16 addr, uint8_t* WRAM){
    uint8_t data = 0;
    data = WRAM[addr.slc<11>(0)];
    return data;
}

uint16_t CPU::read_wram16(uint16_t addr, uint8_t* WRAM){
    uint16 data;
    uint8 low = read_wram8(addr, WRAM);
    uint8 high = read_wram8(addr+1, WRAM);
    data.set_slc(0, low);
    data.set_slc(8, high);
    return (uint16_t)data;
}

uint8_t CPU::read_prom8(uint16 addr, uint8_t* PROM){
    return PROM[addr.slc<15>(0)];
}

uint16_t CPU::read_prom16(uint16_t addr, uint8_t* PROM){
    uint16 data;
    uint8 low = read_prom8(addr, PROM);
    uint8 high = read_prom8(addr+1, PROM);
    data.set_slc(0, low);
    data.set_slc(8, high);
    return data;
}

void CPU::set_nmi()
{
  nmi_line = true;
}


void CPU::set_reset()
{
  reset_line = true;
}

void CPU::exec_DMA(uint8_t* SP_RAM, uint8_t* WRAM){
    SP_RAM[DMAAddrL] = WRAM[(uint16_t)DMAAddrH << 8 | DMAAddrL];
    DMAAddrL++;
    if(DMAAddrL == 0)
        DMAExcute = 0;
    //if(DMAExcute){
    //    uint16_t high = (uint16_t)DMAAddrH << 8;
    //    for(int low = 0; low <= 0xFF; low++){
    //        SP_RAM[low] = WRAM[high | low];
    //    }
    //    DMAExcute = 0;
    //}
}

void CPU::exec(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* PROM, struct SPREG* spreg, uint16_t* Stack, uint8_t* CROM){

    if(DMAExcute) exec_DMA(SP_RAM, WRAM);
    else execution(WRAM, PPU_RAM, SP_RAM, PROM, spreg, Stack, CROM);
}


void CPU::exec_irq(int cause, uint16_t nmi_vec, uint16_t res_vec, uint16_t irq_vec){
    if(enlog) printf("nmi interrupt occur\n");

    Stack_PC = PC;
    Stack_Flags = bindFlags();
    CFlag = 0;
    ZFlag = 0;
    IFlag = 1;
    DFlag = 0;
    BFlag = 0;
    VFlag = 0; 
    NFlag = 0;
    switch(cause){  
        case NMI:   PC = nmi_vec; break;
        case RESET: PC = res_vec; break;
        case IRQ:   PC = irq_vec; break;
        default:    PC = res_vec; break;
    }
}

void CPU::set_mode_false(struct ADDRESS* adr){  
    adr->imm = false;   
    adr->zp = false;    
    adr->zpx = false;   
    adr->zpy = false;   
    adr->abs = false;   
    adr->abx = false;   
    adr->aby = false;   
    adr->zpxi = false;  
    adr->zpiy = false;  
    adr->absi = false;  
    adr->imp = false;   
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
    op_push = false;   \
    op_pop = false;   \
    op_bra_false = false; \
}

void CPU::execution(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* PROM, struct SPREG* spreg, uint16_t* Stack, uint8_t* CROM){

    hls_register struct ADDRESS adr;
    hls_register uint1 op_adc, op_sbc, op_cmp, op_and, op_ora, op_eor, op_bit;
    hls_register uint1 op_load, op_store, op_mov, op_asl, op_lsr, op_rol, op_ror, op_bra_false;
    hls_register uint1 op_inc, op_dec, op_bra, op_jmp, op_jsr, op_rts, op_rti, op_push, op_pop;
    hls_register uint1 acc, x, y;
    hls_register uint1 skip;

    skip = false;
    acc = false, x = false, y = false;
    set_mode_false(&adr);
    set_op_false;

    uint8_t IR = read_prom8(PC, PROM);

    if(enlog) dump_regs(IR);

    switch(IR){
        case 0x69: case 0x65: case 0x75: case 0x6D: case 0x7D: case 0x79: case 0x61: case 0x71: 
            op_adc = true; break;

        case 0xE9: case 0xE5: case 0xF5: case 0xED: case 0xFD: case 0xF9: case 0xE1: case 0xF1: 
            op_sbc = true; break;

        case 0xC9: case 0xC5: case 0xD5: case 0xCD: case 0xDD: case 0xD9: case 0xC1: case 0xD1: 
            op_cmp = true; acc = true; break;

        case 0xE0: case 0xE4: case 0xEC: 
            op_cmp = true; x = true; break;

        case 0xC0:  case 0xC4:  case 0xCC: 
            op_cmp = true; y = true; break;

        case 0x29: case 0x25: case 0x35: case 0x2D: case 0x3D: case 0x39: case 0x21: case 0x31: 
            op_and = true; break;

        case 0x09: case 0x05: case 0x15: case 0x0D: case 0x1D: case 0x19: case 0x01: case 0x11: 
            op_ora = true; break;

        case 0x49: case 0x45: case 0x55: case 0x4D: case 0x5D: case 0x59: case 0x41: case 0x51: 
            op_eor = true; break;

        case 0x24: case 0x2C: 
            op_bit = true;  break;

        case 0xA9: case 0xA5: case 0xB5: case 0xAD: case 0xBD: case 0xB9: case 0xA1: case 0xB1: 
            op_load = true; acc = true; break;

        case 0xA2: case 0xA6: case 0xB6: case 0xAE: case 0xBE: 
            op_load = true; x = true; break; 

        case 0xA0: case 0xA4: case 0xB4: case 0xAC: case 0xBC: 
            op_load = true; y = true;  break;

        case 0x85: case 0x95: case 0x8D: case 0x9D: case 0x99: case 0x81: case 0x91: 
            op_store = true; acc = true; break;

        case 0x86: case 0x96: case 0x8E: 
            op_store = true; x = true;  break;

        case 0x84: case 0x94: case 0x8C: 
            op_store = true; y = true; break;

        case 0xAA: _mov(X,ACC); break; // TAX
        case 0xA8: _mov(Y,ACC); break; // TAY
        case 0x8A: _mov(ACC,X); break; // TXA
        case 0x98: _mov(ACC,Y); break; // TYA
        case 0xBA: _mov(X,SP);  break; // TSX
        case 0x9A: SP=X; break; // TXS

        case 0x0A: _asla(); break;

        case 0x06: case 0x16: case 0x0E: case 0x1E: 
                   op_asl = true; break;

        case 0x4A: _lsra(); break;

        case 0x46: case 0x56: case 0x4E: case 0x5E: 
                   op_lsr = true; break;

        case 0x2A: _rola(); break;

        case 0x26:case 0x36:case 0x2E:case 0x3E: 
                   op_rol = true; break;

        case 0x6A: _rora(); break;

        case 0x66:case 0x76:case 0x6E:case 0x7E: 
                   op_ror = true; break;

        case 0xE6:case 0xF6:case 0xEE:case 0xFE: 
                   op_inc = true; break;

        case 0xE8: _incr(X); break;
        case 0xC8: _incr(Y); break;

        case 0xC6: case 0xD6: case 0xCE: case 0xDE: 
                   op_dec = true; break;

        case 0xCA: _decr(X); break;
        case 0x88: _decr(Y); break;

        case 0x90: if(!CFlag) {op_bra = true; adr.imm = true; } else  {op_bra_false  = true;} break; // BCC
        case 0xB0: if( CFlag) {op_bra = true; adr.imm = true; } else  {op_bra_false  = true;} break; // BCS
        case 0xD0: if(!ZFlag) {op_bra = true; adr.imm = true; } else  {op_bra_false  = true;} break; // BNE
        case 0xF0: if( ZFlag) {op_bra = true; adr.imm = true; } else  {op_bra_false  = true;} break; // BEQ
        case 0x10: if(!NFlag) {op_bra = true; adr.imm = true; } else  {op_bra_false  = true;} break; // BPL
        case 0x30: if( NFlag) {op_bra = true; adr.imm = true; } else  {op_bra_false  = true;} break; // BMI
        case 0x50: if(!VFlag) {op_bra = true; adr.imm = true; } else  {op_bra_false  = true;} break; // BVC
        case 0x70: if( VFlag) {op_bra = true; adr.imm = true; } else  {op_bra_false  = true;} break; // BVS

        case 0x4C: case 0x6C: op_jmp = true; break; 

        case 0x20: op_jsr = true; break; // JSR

        case 0x60: op_rts = true; break; // RTS
        case 0x40: op_rti = true; break; // RTI

        case 0x38: CFlag=1; break; // SEC
        case 0xF8: DFlag=1; break; // SED
        case 0x78: IFlag=1; break; // SEI
        case 0x18: CFlag=0; break; // CLC
        case 0xD8: DFlag=0; break; // CLD
        case 0x58: IFlag=0; break; // CLI 
        case 0xB8: VFlag=0; break; // CLV

        case 0x48: op_push = true; acc = true; break; // PHA
        case 0x08: op_push = true; break; // PHP
        case 0x68: op_pop = true; acc = true; break; // PLA
        case 0x28: op_pop = true; break; // PLP

        default: break;
    }

    switch(IR){
        case 0x69: case 0xE9: case 0xC9: case 0xE0: case 0xC0: case 0x29: 
        case 0x09: case 0x49: case 0xA9: case 0xA2: case 0xA0:             
            adr.imm = true;  break;

        case 0x65: case 0xE5: case 0xC5: case 0xE4: case 0xC4: case 0x25: 
        case 0x05: case 0x45: case 0x24: case 0xA5: case 0xA6: case 0xA4: 
        case 0x85: case 0x86: case 0x84: case 0x06: case 0x46: case 0x26: 
        case 0x66: case 0xE6: case 0xC6: 
            adr.zp = true;  break;

        case 0x75: case 0xF5: case 0xD5: case 0x35: case 0x15: case 0x55: 
        case 0xB5: case 0xB4: case 0x95: case 0x94: 
        case 0x16: case 0x56: case 0x36: case 0x76: case 0xF6: case 0xD6: 
            adr.zpx = true;  break;

        case 0xB6: case 0x96: 
            adr.zpy = true;  break;

        case 0x6D: case 0xED: case 0xCD: case 0xEC: case 0xCC: case 0x2D: 
        case 0x0D: case 0x4D: case 0x2C: case 0xAD: case 0xAE: case 0xAC: 
        case 0x8D: case 0x8E: case 0x8C: 
        case 0x0E: case 0x4E: case 0x2E: case 0x6E: case 0xEE: case 0xCE: 
        case 0x20: case 0x4C: 
            adr.abs = true;  break;

        case 0x7D: case 0xFD: case 0xDD: case 0x3D: case 0x1D: case 0x9D: 
        case 0x5D: case 0xBD: case 0xBC: 
        case 0x5E: case 0x1E: case 0x3E: case 0x7E: case 0xFE: case 0xDE: 
            adr.abx = true;  break;

        case 0x79: case 0xF9: case 0xD9: case 0x39: case 0x19: case 0x59: 
        case 0xB9: case 0xBE: case 0x99: 
            adr.aby = true;  break;

        case 0x61: case 0xE1: case 0xC1: case 0x21: case 0x01: case 0x41: case 0xA1: case 0x81: 
            adr.zpxi = true; break;

        case 0x71: case 0xF1: case 0xD1: case 0x31: case 0x11: case 0x51: case 0xB1: case 0x91: 
            adr.zpiy = true; break;

        case 0x6C: adr.absi = true; break; // JMP (abs)

        case 0xAA: case 0xA8: case 0x8A: case 0x98: case 0xBA: case 0x9A:
        case 0x0A: case 0x4A: case 0x2A: case 0x6A: case 0xE8: case 0xC8: case 0xCA: case 0x88: 
        case 0x38: case 0xF8: case 0x78: case 0x18: case 0xD8: case 0x58: case 0xB8: case 0xEA:
            skip = true;  break;

        default: break;
    }


    PC++;
    if(skip) return;
    else if(op_bra_false){
        PC++;
        return;
    }

    uint16 addr;
    addr = addressing(adr, WRAM, PROM);

    uint8 rddata;
    rddata = read_mem8(addr, WRAM, PROM);
    


    if(op_adc){
        _adc(rddata);
    } 
    else if(op_sbc){
        _sbc(rddata);
    } 
    else if(op_cmp){
        uint8_t data;
        if(acc) data = ACC;
        else if(x) data = X;
        else if(y) data = Y;
        _cmp(data, rddata);
    } 
    else if(op_and){
        _and(rddata);
    }
    else if(op_ora){
        _ora(rddata);
    }
    else if(op_eor){
        _eor(rddata);
    }
    else if(op_bit){
        _bit(rddata);
    }
    else if(op_load){
        uint8 reg;
        _load(reg, addr, rddata);
        if(acc) ACC = reg;
        else if(x) X = reg;
        else if(y) Y = reg;

    } 
    else if(op_store|op_asl|op_lsr|op_rol|op_ror|op_inc|op_dec){
        if(op_store){
            if(acc) rddata = ACC;
            else if(x) rddata = X;
            else if(y) rddata = Y;
        }
        else if(op_asl){
            _asl(addr, rddata);
        }
        else if(op_lsr){
            _lsr(addr, rddata);
        }
        else if(op_rol){
            _rol(addr, rddata);
        }
        else if(op_ror){
            _ror(addr, rddata);
        }
        else if(op_inc){
            _inc(addr, rddata);
        }
        else if(op_dec){
            _dec(addr, rddata);
        }
        _store(rddata, addr);
    }
    else if(op_bra){
        _bra(rddata);
    }
    else if(op_jmp){
        PC = addr;
    }
    else if(op_jsr){
        push_ex16(PC-1, Stack);
        PC = addr;
    }
    else if(op_push){
        if(acc) rddata = ACC;
        else rddata = bindFlags();
        push_ex8(rddata, Stack);
    }
    else if(op_rts){
        PC=pop_ex16(Stack)+1;
    }
    else if(op_rti){
        _unbindFlags(Stack_Flags);
        PC = Stack_PC;
    }
    else if(op_pop){
        rddata = pop_ex8(Stack);
        if(acc){
            ACC = rddata;
            NFlag=ACC[7];
            ZFlag=ACC==0;
        }
        else _unbindFlags(rddata);
    }

}

uint16_t CPU::addressing(struct ADDRESS adr, uint8_t* WRAM, uint8_t* PROM){
    uint16 addr;

    if(adr.imm){
        addr = PC++;
    }
    else if(adr.abs | adr.abx | adr.aby | adr.absi){
        uint16_t tmp16 = read_prom16(PC, PROM);
        PC+=2;
        if(adr.abs | adr.absi) addr = tmp16;
        else if(adr.abx) addr = tmp16 + X;
        else if(adr.aby) addr = tmp16 + Y;
    }
    else if(adr.zp | adr.zpx | adr.zpy | adr.zpiy | adr.zpxi){
        addr = read_prom8(PC, PROM);
        PC++;
        if(adr.zpx | adr.zpxi) addr += X;
        else if(adr.zpy) addr += Y;
    } 

    if(adr.absi | adr.zpxi | adr.zpiy){
        uint16_t rdaddr;
        if(adr.zpxi) rdaddr = addr.slc<8>(0);
        else rdaddr = addr;

        addr = read_wram16(rdaddr, WRAM);
        if(adr.zpiy) addr += Y;
    }

    return addr;
}

uint16_t CPU::get_PC(){
    return PC;
}

void CPU::push_ex8(uint8_t data, uint16_t* Stack){
    SP_wide = false;
    uint16 wrdata = 0;
    wrdata.set_slc(8, (uint8)data);
    Stack[(uint8_t)(SP--) & 0xFF] = (uint16_t)wrdata;
}

void CPU::push_ex16(uint16_t data, uint16_t* Stack){
    SP_wide = true;
    Stack[(uint8_t)(SP--) & 0xFF] = data;
}

uint8_t CPU::pop_ex8(uint16_t* Stack){
    uint16_t data = Stack[(uint8_t)(++SP) & 0xFF];
    uint8_t ret_data;
    if(SP_wide){
        ret_data = (uint8_t)data;
        SP_wide = false;
        SP--;
    }
    else
        ret_data = (uint8_t)(data >> 8);

    return ret_data;
}

uint16_t CPU::pop_ex16(uint16_t* Stack){
    SP_wide = false;
    return Stack[(uint8_t)(++SP) & 0xFF];
}

void CPU::push8(uint8_t data, uint8_t* Stack){
    Stack[(uint8_t)(SP--) & 0xFF] = data;
}

uint8_t CPU::pop8(uint8_t* Stack){
    return Stack[(uint8_t)(++SP) & 0xFF];
}

void CPU::push16(uint16_t data, uint8_t* Stack){
    push8((uint8_t)(data >> 8), Stack);
    push8((uint8_t)data, Stack);
}

uint16_t CPU::pop16(uint8_t* Stack){
    uint16_t data;
    data = pop8(Stack);
    data |= (uint16_t)pop8(Stack) << 8;
    return data;
}

uint8_t CPU::bindFlags(){ 
    uint8 data;
    data[7] = NFlag; 
    data[6] = VFlag; 
    data[5] = 1; 
    data[4] = BFlag; 
    data[3] = DFlag; 
    data[2] = IFlag; 
    data[1] = ZFlag; 
    data[0] = CFlag; 
    return (uint8_t)data;
}

