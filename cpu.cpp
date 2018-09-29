#include "cpu.h"
#include <cstdio>
#include "instr.h"
#include "HLS/hls.h"

//const bool enlog = true;
const bool enlog = false;

void CPU::dump_regs(uint8_t insn){
    uint8_t flag = _bindFlags();
    //printf("%04x %02x   A:%02x X:%02x Y:%02x P:%02x SP:%02x\n",
    //                PC, insn, ACC, X, Y, flag, SP);
    uint32_t cache = get_cache();
    printf("%04x %02x   A:%02x X:%02x Y:%02x P:%02x SP:%02x    cache:%08x\n",
                    PC, insn, ACC, X, Y, flag, SP, cache);
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

uint8_t CPU::read_mem8(uint16_t addr, uint8_t* WRAM, uint32_t* PROM){
    uint8_t data = 0;
    if((addr >> 15) & 1)
        data = read_prom_ex8(addr, PROM);
    else data = WRAM[addr&0x7FF]; //ok??
    return data;
}

uint8_t CPU::norm_read8(uint16_t addr, uint8_t* WRAM){
    uint8_t data = 0;
    data = WRAM[addr&0x7FF];
    return data;
}

uint16_t CPU::norm_read16(uint16_t addr, uint8_t* WRAM){
    uint16_t data;
    data = norm_read8(addr, WRAM);
    data |= (uint16_t)norm_read8(addr+1, WRAM) << 8;
    return data;
}

void CPU::norm_write8(uint16_t addr, uint8_t data, uint8_t* WRAM){
    WRAM[addr&0x7FF] = data; 
}

uint8_t CPU::read_prom(uint16_t addr, uint8_t* PROM){
    return PROM[addr & ~(1 << 15)];
}

uint16_t CPU::read_prom16(uint16_t addr, uint8_t* PROM){
    uint16_t data;
    data = read_prom(addr, PROM);
    data |= (uint16_t)read_prom(addr+1, PROM) << 8;
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
    //SP_RAM[DMAAddrL] = WRAM[(uint16_t)DMAAddrH << 8 | DMAAddrL];
    //DMAAddrL++;
    //if(DMAAddrL == 0)
    //    DMAExcute = 0;
    if(DMAExcute){
        uint16_t high = (uint16_t)DMAAddrH << 8;
        for(int low = 0; low <= 0xFF; low++){
            SP_RAM[low] = WRAM[high | low];
        }
        DMAExcute = 0;
    }
}

void CPU::exec(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint32_t* PROM, struct SPREG* spreg, uint16_t* Stack, uint8_t* CROM){

    //if(DMAExcute) exec_DMA(SP_RAM, WRAM);
    //else execution(WRAM, PPU_RAM, SP_RAM, PROM, spreg, Stack, CROM);
    execution(WRAM, PPU_RAM, SP_RAM, PROM, spreg, Stack, CROM);
}


void CPU::exec_irq(int cause, uint16_t nmi_vec, uint16_t res_vec, uint16_t irq_vec){
    if(enlog) printf("nmi interrupt occur\n");

    Stack_PC = PC;
    Stack_Flags = _bindFlags();
    //SP-=3;
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
    //cache_false();
    PC_update = true;
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

void CPU::execution(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint32_t* PROM, struct SPREG* spreg, uint16_t* Stack, uint8_t* CROM){

    hls_register struct ADDRESS adr;
    hls_register bool op_adc, op_sbc, op_cmp, op_and, op_ora, op_eor, op_bit;
    hls_register bool op_load, op_store, op_mov, op_asl, op_lsr, op_rol, op_ror, op_bra_false;
    hls_register bool op_inc, op_dec, op_bra, op_jmp, op_jsr, op_rts, op_rti, op_push, op_pop;
    hls_register bool acc, x, y;

    acc = false, x = false, y = false;
    set_mode_false(&adr);
    set_op_false;


    cache_update(PC, PROM);
    //if(V[2] == false) return;

    uint8_t IR = cache[0];
    //V[0] = false;

    //hls_register uint8_t IR = read_prom(PC, PROM);
    if(enlog) dump_regs(IR);
    //PC++;

    switch(IR){
        /* ALU */
        case 0x69: op_adc = true; adr.imm = true; break;
        case 0x65: op_adc = true; adr.zp = true; break;
        case 0x75: op_adc = true; adr.zpx = true; break;
        case 0x6D: op_adc = true; adr.abs = true; break;
        case 0x7D: op_adc = true; adr.abx = true; break;
        case 0x79: op_adc = true; adr.aby = true; break;
        case 0x61: op_adc = true; adr.zpxi = true; break;
        case 0x71: op_adc = true; adr.zpiy = true; break;

        case 0xE9: op_sbc = true; adr.imm = true; break;
        case 0xE5: op_sbc = true; adr.zp = true;  break;
        case 0xF5: op_sbc = true; adr.zpx = true; break;
        case 0xED: op_sbc = true; adr.abs = true; break;
        case 0xFD: op_sbc = true; adr.abx = true; break;
        case 0xF9: op_sbc = true; adr.aby = true; break;
        case 0xE1: op_sbc = true; adr.zpxi = true; break;
        case 0xF1: op_sbc = true; adr.zpiy = true; break;

        case 0xC9: op_cmp = true; acc = true; adr.imm = true;  break;
        case 0xC5: op_cmp = true; acc = true; adr.zp = true;   break;
        case 0xD5: op_cmp = true; acc = true; adr.zpx = true;  break;
        case 0xCD: op_cmp = true; acc = true; adr.abs = true;  break;
        case 0xDD: op_cmp = true; acc = true; adr.abx = true;  break;
        case 0xD9: op_cmp = true; acc = true; adr.aby = true;  break;
        case 0xC1: op_cmp = true; acc = true; adr.zpxi = true; break;
        case 0xD1: op_cmp = true; acc = true; adr.zpiy = true; break;

        case 0xE0: op_cmp = true; x = true; adr.imm = true; break;
        case 0xE4: op_cmp = true; x = true; adr.zp = true; break;
        case 0xEC: op_cmp = true; x = true; adr.abs = true; break;

        case 0xC0: op_cmp = true; y = true; adr.imm = true; break;
        case 0xC4: op_cmp = true; y = true; adr.zp = true;  break;
        case 0xCC: op_cmp = true; y = true; adr.abs = true; break;

        case 0x29: op_and = true; adr.imm = true;  break;
        case 0x25: op_and = true; adr.zp = true;   break;
        case 0x35: op_and = true; adr.zpx = true;  break;
        case 0x2D: op_and = true; adr.abs = true;  break;
        case 0x3D: op_and = true; adr.abx = true;  break;
        case 0x39: op_and = true; adr.aby = true;  break;
        case 0x21: op_and = true; adr.zpxi = true; break;
        case 0x31: op_and = true; adr.zpiy = true; break;

        case 0x09: op_ora = true; adr.imm = true;  break;
        case 0x05: op_ora = true; adr.zp = true;   break;
        case 0x15: op_ora = true; adr.zpx = true;  break;
        case 0x0D: op_ora = true; adr.abs = true;  break;
        case 0x1D: op_ora = true; adr.abx = true;  break;
        case 0x19: op_ora = true; adr.aby = true;  break;
        case 0x01: op_ora = true; adr.zpxi = true; break;
        case 0x11: op_ora = true; adr.zpiy = true; break;

        case 0x49: op_eor = true; adr.imm = true;  break;
        case 0x45: op_eor = true; adr.zp = true;   break;
        case 0x55: op_eor = true; adr.zpx = true;  break;
        case 0x4D: op_eor = true; adr.abs = true;  break;
        case 0x5D: op_eor = true; adr.abx = true;  break;
        case 0x59: op_eor = true; adr.aby = true;  break;
        case 0x41: op_eor = true; adr.zpxi = true; break;
        case 0x51: op_eor = true; adr.zpiy = true; break;

        case 0x24: op_bit = true; adr.zp = true;  break;
        case 0x2C: op_bit = true; adr.abs = true;  break;

                   /* laod / store */
        case 0xA9: op_load = true; acc = true; adr.imm = true;  break;
        case 0xA5: op_load = true; acc = true; adr.zp = true;   break;
        case 0xB5: op_load = true; acc = true; adr.zpx = true;  break;
        case 0xAD: op_load = true; acc = true; adr.abs = true;  break;
        case 0xBD: op_load = true; acc = true; adr.abx = true;  break;
        case 0xB9: op_load = true; acc = true; adr.aby = true;  break;
        case 0xA1: op_load = true; acc = true; adr.zpxi = true; break;
        case 0xB1: op_load = true; acc = true; adr.zpiy = true; break;

        case 0xA2: op_load = true; x = true; adr.imm = true;  break;
        case 0xA6: op_load = true; x = true; adr.zp = true;  break;
        case 0xB6: op_load = true; x = true; adr.zpy = true;  break;
        case 0xAE: op_load = true; x = true; adr.abs = true;  break;
        case 0xBE: op_load = true; x = true; adr.aby = true;  break;

        case 0xA0: op_load = true; y = true; adr.imm = true;  break;
        case 0xA4: op_load = true; y = true; adr.zp = true;  break;
        case 0xB4: op_load = true; y = true; adr.zpx = true;  break;
        case 0xAC: op_load = true; y = true; adr.abs = true;  break;
        case 0xBC: op_load = true; y = true; adr.abx = true;  break;

        case 0x85: op_store = true; acc = true; adr.zp = true;   break;
        case 0x95: op_store = true; acc = true; adr.zpx = true;  break;
        case 0x8D: op_store = true; acc = true; adr.abs = true;  break;
        case 0x9D: op_store = true; acc = true; adr.abx = true;  break;
        case 0x99: op_store = true; acc = true; adr.aby = true;  break;
        case 0x81: op_store = true; acc = true; adr.zpxi = true; break;
        case 0x91: op_store = true; acc = true; adr.zpiy = true; break;

        case 0x86: op_store = true; x = true; adr.zp = true;  break;
        case 0x96: op_store = true; x = true; adr.zpy = true;  break;
        case 0x8E: op_store = true; x = true; adr.abs = true;  break;

        case 0x84: op_store = true; y = true; adr.zp = true;  break;
        case 0x94: op_store = true; y = true; adr.zpx = true;  break;
        case 0x8C: op_store = true; y = true; adr.abs = true;  break;

                   /* transfer */
        case 0xAA: _mov(X,ACC); break; // TAX
        case 0xA8: _mov(Y,ACC); break; // TAY
        case 0x8A: _mov(ACC,X); break; // TXA
        case 0x98: _mov(ACC,Y); break; // TYA
        case 0xBA: _mov(X,SP);  break; // TSX
        case 0x9A: SP=X; break; // TXS

                   /* shift */
        case 0x0A: _asla(); break;
        case 0x06: op_asl = true; adr.zp = true; break;
        case 0x16: op_asl = true; adr.zpx = true; break;
        case 0x0E: op_asl = true; adr.abs = true; break;
        case 0x1E: op_asl = true; adr.abx = true; break;

        case 0x4A: _lsra(); break;
        case 0x46: op_lsr = true; adr.zp = true;  break;
        case 0x56: op_lsr = true; adr.zpx = true; break;
        case 0x4E: op_lsr = true; adr.abs = true; break;
        case 0x5E: op_lsr = true; adr.abx = true; break;

        case 0x2A: _rola(); break;
        case 0x26: op_rol = true; adr.zp = true;  break;
        case 0x36: op_rol = true; adr.zpx = true; break;
        case 0x2E: op_rol = true; adr.abs = true; break;
        case 0x3E: op_rol = true; adr.abx = true; break;

        case 0x6A: _rora(); break;
        case 0x66: op_ror = true; adr.zp = true;  break;
        case 0x76: op_ror = true; adr.zpx = true; break;
        case 0x6E: op_ror = true; adr.abs = true; break;
        case 0x7E: op_ror = true; adr.abx = true; break;

        case 0xE6: op_inc = true; adr.zp = true; break;
        case 0xF6: op_inc = true; adr.zpx = true; break;
        case 0xEE: op_inc = true; adr.abs = true; break;
        case 0xFE: op_inc = true; adr.abx = true; break;
        case 0xE8: _incr(X); break;
        case 0xC8: _incr(Y); break;

        case 0xC6: op_dec = true; adr.zp = true;  break;
        case 0xD6: op_dec = true; adr.zpx = true; break;
        case 0xCE: op_dec = true; adr.abs = true; break;
        case 0xDE: op_dec = true; adr.abx = true; break;
        case 0xCA: _decr(X);  break;
        case 0x88: _decr(Y);  break;

        case 0x90: if(!CFlag) {op_bra = true; adr.imm = true; } else  op_bra_false = true; break; // BCC
        case 0xB0: if( CFlag) {op_bra = true; adr.imm = true; } else  op_bra_false = true; break; // BCS
        case 0xD0: if(!ZFlag) {op_bra = true; adr.imm = true; } else  op_bra_false = true; break; // BNE
        case 0xF0: if( ZFlag) {op_bra = true; adr.imm = true; } else  op_bra_false = true; break; // BEQ
        case 0x10: if(!NFlag) {op_bra = true; adr.imm = true; } else  op_bra_false = true; break; // BPL
        case 0x30: if( NFlag) {op_bra = true; adr.imm = true; } else  op_bra_false = true; break; // BMI
        case 0x50: if(!VFlag) {op_bra = true; adr.imm = true; } else  op_bra_false = true; break; // BVC
        case 0x70: if( VFlag) {op_bra = true; adr.imm = true; } else  op_bra_false = true; break; // BVS

                       /* jump / call / return */
        case 0x4C: op_jmp = true; adr.abs  = true; break; // JMP abs
        case 0x6C: op_jmp = true; adr.absi = true; break; // JMP (abs)

        case 0x20: op_jsr = true; adr.abs = true; break; // JSR

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
        case 0x48: op_push = true; acc = true; break; // PHA
        case 0x08: op_push = true; break; // PHP
        case 0x68: op_pop = true; acc = true; break; // PLA
        case 0x28: op_pop = true; break; // PLP


        case 0xEA: break; // NOP

        default:
                   //printf("undefined opcode: %02x\n", (uint)IR);
                   break;
    }

    if(V[2] == false && (adr.abs | adr.abx | adr.aby | adr.absi)){
        //printf("OK\n");
        return;
    }
    if(V[1] == false && (adr.imm | adr.zp | adr.zpx | adr.zpy | adr.zpiy | adr.zpxi | op_bra_false)){
        return;
    }

    //if(V[2] == false) return;
    PC++;
    V[0] = false;

    uint16_t addr;
    addr = addressing(adr, WRAM, PROM);

    //hls_register uint8_t rddata = read_mem8(addr, WRAM, PROM);
    //uint8_t rddata = read_mem8(addr, WRAM, PROM);
    uint8_t rddata = ((addr >> 15) & 1 || op_store) ? read_prom_ex8(addr, PROM) : read(addr, WRAM, PPU_RAM, spreg, CROM);
    //uint8_t rddata = ((addr >> 15) & 1) ? read_mem8(addr, WRAM, PROM) : read(addr, WRAM, PPU_RAM, spreg, CROM);
    
    //uint8_t rddata;
    //if((addr >> 15) & 1 || op_store) rddata = read_prom_ex8(addr, PROM);
    //else rddata = read(addr, WRAM, PPU_RAM, spreg, CROM);


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
        uint8_t reg;
        _load(reg, addr, rddata);
        if(acc) ACC = reg;
        else if(x) X = reg;
        else if(y) Y = reg;

    } 
    //else if(op_store){
    //    if(acc) rddata = ACC;
    //    else if(x) rddata = X;
    //    else if(y) rddata = Y;
    //    _store(rddata, addr);
    //}
    //else if(op_asl){
    //    _asl(addr, rddata);
    //}
    //else if(op_lsr){
    //    _lsr(addr, rddata);
    //}
    //else if(op_rol){
    //    _rol(addr, rddata);
    //}
    //else if(op_ror){
    //    _ror(addr, rddata);
    //}
    //else if(op_inc){
    //    _inc(addr, rddata);
    //}
    //else if(op_dec){
    //    _dec(addr, rddata);
    //}
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
        PC_update = true;
    }
    else if(op_bra_false){
        V[1] = false; 
        PC++;
    }
    else if(op_jmp){
        PC = addr;
        //cache_false();
        PC_update = true;
    }
    else if(op_jsr){
        push_ex16(PC-1, Stack);
        //push16(PC-1, Stack);
        PC = addr;
        //cache_false();
        PC_update = true;
    }
    else if(op_push){
        if(acc) rddata = ACC;
        else rddata = _bindFlags();
        push_ex8(rddata, Stack);
        //push8(rddata, Stack);
    }
    else if(op_rts){
        PC=pop_ex16(Stack)+1;
        //PC=pop16(Stack)+1;
        //cache_false();
        PC_update = true;
    }
    else if(op_rti){
        _unbindFlags(Stack_Flags);
        PC = Stack_PC;
        //SP+=3;
        //cache_false();
        PC_update = true;
    }
    else if(op_pop){
        rddata = pop_ex8(Stack);
        //rddata = pop8(Stack);
        if(acc){
            ACC = rddata;
            NFlag=ACC>>7;
            ZFlag=ACC==0;
        }
        else _unbindFlags(rddata);
    }

}

uint16_t CPU::addressing(struct ADDRESS adr, uint8_t* WRAM, uint32_t* PROM){
    uint16_t addr;
    //uint16_t tmp16 = read_prom16(PC, PROM);
    //uint8_t tmp8 = (uint8_t)tmp16;

    if(adr.imm){
        addr = PC++;
        V[1] = false;
    }
    else if(adr.abs | adr.abx | adr.aby | adr.absi){
        //uint16_t tmp16 = read_prom16(PC, PROM);
        uint16_t tmp16 = (uint16_t)cache[2] << 8 | cache[1];
        V[2] = V[1] = false;
        PC+=2;
        if(adr.abs | adr.absi) addr = tmp16;
        else if(adr.abx) addr = tmp16 + X;
        else if(adr.aby) addr = tmp16 + Y;
    }
    else if(adr.zp | adr.zpx | adr.zpy | adr.zpiy | adr.zpxi){
        //uint8_t tmp8 = read_prom(PC, PROM);
        uint8_t tmp8 = cache[1];
        V[1] = false;
        PC++;
        if(adr.zp | adr.zpiy) addr = tmp8;
        else if(adr.zpx | adr.zpxi) addr = tmp8 + X;
        else if(adr.zpy) addr = tmp8 + Y;
    } 

    if(adr.absi | adr.zpxi | adr.zpiy){
        if(adr.zpxi) addr &= 0xff;
        addr = norm_read16(addr, WRAM);
        if(adr.zpiy) addr += Y;
    }

    return addr;
}

uint8_t CPU::read_prom_ex8(uint16_t addr, uint32_t* PROM){
    uint16_t tmp = read_prom_ex16(addr, PROM);
    uint8_t data;
    if(addr & 1) data = (uint8_t)(tmp >> 8);
    else data = (uint8_t)tmp;
    return data;
}

uint16_t CPU::read_prom_ex16(uint16_t addr, uint32_t* PROM){
    uint32_t tmp = read_prom_ex32(addr, PROM);
    uint16_t data;
    if((addr >> 1) & 1) data = (uint16_t)(tmp >> 16);
    else data = (uint16_t)tmp;
    return data;
}

uint32_t CPU::read_prom_ex32(uint16_t addr, uint32_t* PROM){
    //return PROM[addr >> 3];
    return PROM[(addr & 0x7FFF) >> 2];
}

void CPU::cache_update(uint16_t addr, uint32_t* PROM){

    uint16_t read_addr;
    //if(v == 4) read_addr = addr;
    //else read_addr = cache_addr;
    if(PC_update) read_addr = addr;
    else read_addr = cache_addr;
    hls_register uint32_t data = read_prom_ex32(read_addr, PROM);

    uint8_t loc = read_addr & 0x3;

    cache_addr = read_addr;

    uint8_t v;
    if(V[0]) v = 0;
    else if(V[1]) v = 1;
    else if(V[2]) v = 2;
    else if(V[3]) v = 3;
    else v = 4;

    if(PC_update) {
        cache_false();
        v = 4;
    }

    uint8_t k = 0;
    #pragma unroll
    for(int i = 0; i < 4; i++){
        if(i+v < 4 && V[i+v])
            k++;
    }
    #pragma unroll
    for(int i = 0; i < 4; i++){
        if(i < k){
            cache[i] = cache[i+v];
            V[i] = true;
        }
        else if(i - k  + loc < 4){
            cache[i] = (uint8_t)(data >> 8*(i - k + loc));
            //loc++;
            V[i] = true;
            cache_addr++;
        }
        else
            V[i] = false;
    }

    //for(int i = 0; i < 4; i++){
    //    if(i+v < 4 && V[i+v]){
    //        V[i] = true;
    //        cache[i] = cache[i+v];
    //    }
    //    else if(loc < 4){
    //        V[i] = true;
    //        cache[i] = (uint8_t)(data >> 8*loc);
    //        loc++;
    //        cache_addr++;
    //    }
    //    else
    //        V[i] = false;
    //}

/*
    hls_register uint8_t data0 = (uint8_t)data;
    hls_register uint8_t data1 = (uint8_t)(data >> 8);
    hls_register uint8_t data2 = (uint8_t)(data >> 16);
    hls_register uint8_t data3 = (uint8_t)(data >> 24);
    if(PC_update){
        V[0] = V[1] = V[2] = V[3] = true;
        switch(loc){
            case 0:
                cache[0] = data0;
                cache[1] = data1;
                cache[2] = data2;
                cache[3] = data3;
                cache_addr+=4;
                break;
            case 1:
                cache[0] = data1;
                cache[1] = data2;
                cache[2] = data3;
                V[3] = false;
                cache_addr+=3;
                break;
            case 2:
                cache[0] = data2;
                cache[1] = data3;
                V[2] = V[3] = false;
                cache_addr+=2;
                break;
            case 3:
                cache[0] = data3;
                V[1] = V[2] = V[3] = false;
                cache_addr+=1;
                break;
        }
    }
    else{
        if(!V[0] & !V[1] & !V[2] & V[3]){
            V[0] = V[1] = V[2] = V[3] = true;
            cache[0] = cache[3];
            switch(loc){
                case 0:
                    cache[1] = data0;
                    cache[2] = data1;
                    cache[3] = data2;
                    cache_addr+=3;
                    break;
                case 1:
                    cache[1] = data1;
                    cache[2] = data2;
                    cache[3] = data3;
                    cache_addr+=3;
                    break;
                case 2:
                    cache[1] = data2;
                    cache[2] = data3;
                    V[3] = false;
                    cache_addr+=2;
                    break;
                case 3:
                    cache[1] = data3;
                    V[2] = V[3] = false;
                    cache_addr+=1;
                    break;
            }
        }
        else if(!V[0] & !V[1] & V[2] & V[3]){
            V[0] = V[1] = V[2] = V[3] = true;
            cache[0] = cache[2];
            cache[1] = cache[3];
            switch(loc){
                case 0:
                    cache[2] = data0;
                    cache[3] = data1;
                    cache_addr+=2;
                    break;
                case 1:
                    cache[2] = data1;
                    cache[3] = data2;
                    cache_addr+=2;
                    break;
                case 2:
                    cache[2] = data2;
                    cache[3] = data3;
                    cache_addr+=2;
                    break;
                case 3:
                    cache[2] = data3;
                    V[3] = false;
                    cache_addr+=1;
                    break;
            }
        }
        else if(!V[0] & !V[1] & V[2] & !V[3]){
            V[0] = V[1] = V[2] = V[3] = true;
            cache[0] = cache[2];
            switch(loc){
                case 0:
                    cache[1] = data0;
                    cache[2] = data1;
                    cache[3] = data2;
                    cache_addr+=3;
                    break;
                case 1:
                    cache[1] = data1;
                    cache[2] = data2;
                    cache[3] = data3;
                    cache_addr+=3;
                    break;
                case 2:
                    cache[1] = data2;
                    cache[2] = data3;
                    V[3] = false;
                    cache_addr+=2;
                    break;
                case 3:
                    cache[1] = data3;
                    V[2] = V[3] = false;
                    cache_addr+=1;
                    break;
            }
        }
        else if(!V[0] & !V[1] & !V[2] & !V[3]){
            V[0] = V[1] = V[2] = V[3] = true;
            switch(loc){
                case 0:
                    cache[0] = data0;
                    cache[1] = data1;
                    cache[2] = data2;
                    cache[3] = data3;
                    cache_addr+=4;
                    break;
                case 1:
                    cache[0] = data1;
                    cache[1] = data2;
                    cache[2] = data3;
                    V[3] = false;
                    cache_addr+=3;
                    break;
                case 2:
                    cache[0] = data2;
                    cache[1] = data3;
                    V[2] = V[3] = false;
                    cache_addr+=2;
                    break;
                case 3:
                    cache[0] = data3;
                    V[1] = V[2] = V[3] = false;
                    cache_addr+=1;
                    break;
            }
        }
        else if(V[0] & V[1] & !V[2] & !V[3]){
            V[0] = V[1] = V[2] = V[3] = true;
            switch(loc){
                case 0:
                    cache[2] = data0;
                    cache[3] = data1;
                    cache_addr+=2;
                    break;
                case 1:
                    cache[2] = data1;
                    cache[3] = data2;
                    cache_addr+=2;
                    break;
                case 2:
                    cache[2] = data2;
                    cache[3] = data3;
                    cache_addr+=2;
                    break;
                case 3:
                    cache[2] = data3;
                    V[3] = false;
                    cache_addr+=1;
                    break;
            }
        }
        else if(!V[0] & V[1] & V[2] & !V[3]){
            V[0] = V[1] = V[2] = V[3] = true;
            cache[0] = cache[1];
            cache[1] = cache[2];
            switch(loc){
                case 0:
                    cache[2] = data0;
                    cache[3] = data1;
                    cache_addr+=2;
                    break;
                case 1:
                    cache[2] = data1;
                    cache[3] = data2;
                    cache_addr+=2;
                    break;
                case 2:
                    cache[2] = data2;
                    cache[3] = data3;
                    cache_addr+=2;
                    break;
                case 3:
                    cache[2] = data3;
                    V[3] = false;
                    cache_addr+=1;
                    break;
            }
        }
        else if(V[0] & !V[1] & !V[2] & !V[3]){
            V[0] = V[1] = V[2] = V[3] = true;
            switch(loc){
                case 0:
                    cache[1] = data0;
                    cache[2] = data1;
                    cache[3] = data2;
                    cache_addr+=3;
                    break;
                case 1:
                    cache[1] = data1;
                    cache[2] = data2;
                    cache[3] = data3;
                    cache_addr+=3;
                    break;
                case 2:
                    cache[1] = data2;
                    cache[2] = data3;
                    V[3] = false;
                    cache_addr+=2;
                    break;
                case 3:
                    cache[1] = data3;
                    V[2] = V[3] = false;
                    cache_addr+=1;
                    break;
            }
        }
        else if(!V[0] & V[1] & !V[2] & !V[3]){
            V[0] = V[1] = V[2] = V[3] = true;
            cache[0] = cache[1];
            switch(loc){
                case 0:
                    cache[1] = data0;
                    cache[2] = data1;
                    cache[3] = data2;
                    cache_addr+=3;
                    break;
                case 1:
                    cache[1] = data1;
                    cache[2] = data2;
                    cache[3] = data3;
                    cache_addr+=3;
                    break;
                case 2:
                    cache[1] = data2;
                    cache[2] = data3;
                    V[3] = false;
                    cache_addr+=2;
                    break;
                case 3:
                    cache[1] = data3;
                    V[2] = V[3] = false;
                    cache_addr+=1;
                    break;
            }
        }
        else if(!V[0] & V[1] & V[2] & V[3]){
            V[0] = V[1] = V[2] = V[3] = true;
            cache[0] = cache[1];
            cache[1] = cache[2];
            cache[2] = cache[3];
            cache_addr+=1;
            switch(loc){
                case 0:
                    cache[3] = data0;
                    break;
                case 1:
                    cache[3] = data1;
                    break;
                case 2:
                    cache[3] = data2;
                    break;
                case 3:
                    cache[3] = data3;
                    break;
            }
        }
    }
         
*/
    PC_update = false;
         
}        
         
void CPU::cache_false(){
    #pragma unroll
    for(int i = 0; i < 4; i++)
        V[i] = false;
}

uint16_t CPU::get_PC(){
    return PC;
}

uint8_t CPU::get_SP(){
    return SP;
}

uint8_t CPU::get_ACC(){
    return ACC;
}

uint8_t CPU::get_X(){
    return X;
}

uint8_t CPU::get_Y(){
    return Y;
}

uint32_t CPU::get_cache(){
    uint32_t data = 0;
    data |= (uint32_t)cache[0] << 0;
    data |= (uint32_t)cache[1] << 8;
    data |= (uint32_t)cache[2] << 16;
    data |= (uint32_t)cache[3] << 24;
    return data;
}

void CPU::push_ex8(uint8_t data, uint16_t* Stack){
    //push_ex16((uint16_t)data, Stack);
    //wide[SP] = false;
    SP_wide = false;
    Stack[(uint8_t)(SP--) & 0xFF] = (uint16_t)data << 8;
    //SP++;
}

void CPU::push_ex16(uint16_t data, uint16_t* Stack){
    //wide[SP] = true;
    SP_wide = true;
    Stack[(uint8_t)(SP--) & 0xFF] = data;
    //SP--;
}

uint8_t CPU::pop_ex8(uint16_t* Stack){
    //--SP;
    //return (uint8_t)pop_ex16(Stack);
    
    uint16_t data = Stack[(uint8_t)(++SP) & 0xFF];
    uint8_t ret_data;
    //if(wide[SP]){
    if(SP_wide){
        ret_data = (uint8_t)data;
        //wide[SP] = false;
        SP_wide = false;
        SP--;
    }
    else
        ret_data = (uint8_t)(data >> 8);

    return ret_data;
}

uint16_t CPU::pop_ex16(uint16_t* Stack){
    //++SP;
    SP_wide = false;
    return Stack[(uint8_t)(++SP) & 0xFF];
}

