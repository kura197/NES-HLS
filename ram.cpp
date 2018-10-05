
#include "ram.h"
#include <fstream>
#include <iostream>

using namespace std;



uint8_t RAM::read(uint16_t addr, uint8_t* WRAM, uint8_t* PPU_RAM, struct SPREG* spreg, uint8_t* CROM){
    uint8 data = 0;
    switch(addr){
        case 0x2002: 
              data[7] = spreg->VBlank;
              data[6] = spreg->SPhit;
              data[5] = spreg->num_ScanSP;

              spreg->VBlank = false;
              BGoffset_sel_X = false;
              PPUAddr_sel_H = false;
            break;
        case 0x2007:
            data = read_2007(PPU_RAM, CROM);
            break;
        case 0x4016:
            data = read_pad_1();
            break;
        case 0x4017:
            data = read_pad_2();
            break;
        default:
            if(addr < 0x800) data = WRAM[addr];
            break;
    }
    return data;
}

void RAM::write(uint16_t addr, uint8_t data, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, struct SPREG* spreg, uint8_t* CROM){
    switch(addr){
        case 0x2000: 
            write_2000(data, spreg);
            break;
        case 0x2001: 
            write_2001(data, spreg);
            break;
        case 0x2003: 
            write_2003(data);
            break;
        case 0x2004: 
            write_2004(data, SP_RAM);
            break;
        case 0x2005: 
            write_2005(data, spreg);
            break;
        case 0x2006: 
            write_2006(data);
            break;
        case 0x2007: 
            write_2007(data, PPU_RAM, CROM);
            break;
        case 0x4014: 
            DMA_start(data, WRAM, SP_RAM);
            break;
        case 0x4016: 
            reset_pad(data);
            break;
        default:
            if(addr < 0x800) WRAM[addr] = data;
            break;
    }
}

void RAM::write_2000(uint8 data, struct SPREG* spreg){
    spreg->VBlank_NMI = data[7];    
    spreg->BGPtnAddr =  data[4];    
    spreg->SPPtnAddr =  data[3];    
    PPUInc =            data[2];    
    spreg->NameAddrH =  data[1];    
    spreg->NameAddrL =  data[0];    
}

void RAM::write_2001(uint8 data, struct SPREG* spreg){
    spreg->EnSP =      data[4];    
    spreg->EnBG =      data[3];    
    spreg->SPMSK =     data[2];    
    spreg->BGMSK =     data[1];    
}

void RAM::write_2003(uint8_t data){
    SPAddr = data;
}

void RAM::write_2004(uint8_t data, uint8_t* SP_RAM){
    SP_RAM[SPAddr++] = data;
}

void RAM::write_2005(uint8_t data, struct SPREG* spreg){
    if(!BGoffset_sel_X){
        spreg->BGoffset_X = data;
        BGoffset_sel_X = true;
    }  
    else{
        spreg->BGoffset_Y = data;
        BGoffset_sel_X = false;
    }
}

void RAM::write_2006(uint8 data){
    if(!PPUAddr_sel_H){
        PPUAddr.set_slc(8, data);
        PPUAddr_sel_H = true;
    }
    else{
        PPUAddr.set_slc(0, data);
        PPUAddr_sel_H = false;
    } 
}

void RAM::write_2007(uint8_t data, uint8_t* PPU_RAM, uint8_t* CROM){
    uint16 addr = PPUAddr;
    switch(addr){
        case 0x3F10:
        case 0x3F14:
        case 0x3F18:
        case 0x3F1C:
            addr &= 0xFFEF;
            break;
        default:
            break;
    }
    if(addr[13])    PPU_RAM[addr & 0x1FFF] = data;
    else CROM[addr] = data;
    PPUAddr = (PPUInc) ? PPUAddr + 32 : PPUAddr + 1;
}

uint8_t RAM::read_2007(uint8_t* PPU_RAM, uint8_t* CROM){
    uint8_t data;
    uint16 addr = PPUAddr;
    switch(addr){
        case 0x3F10:
        case 0x3F14:
        case 0x3F18:
        case 0x3F1C:
            addr &= 0xFFEF;
            break;
        default:
            break;
    }

    uint8_t tmp_data;
    if(addr[13]) tmp_data = PPU_RAM[addr & 0x1FFF];
    else tmp_data = CROM[addr];
    if(addr < 0x3F00){
        data = spram_buf;
        spram_buf = tmp_data;
    }
    else  data = tmp_data;
    PPUAddr = (PPUInc) ? PPUAddr + 32 : PPUAddr + 1;
    return data;
}

void RAM::DMA_start(uint8_t addr_H, uint8_t* WRAM, uint8_t* SP_RAM){
    DMAExcute = 1;
    DMAAddrH = addr_H;
    DMAAddrL = 0;
}

void RAM::reset_pad(uint8 data){
    if(pad_reset_state == 1 && (data[0] == 0))
        pad_reset_state = 2;
    
    if(pad_reset_state == 0 && (data[0] == 1))
        pad_reset_state = 1;

    if(pad_reset_state == 2){
        pad_reset_state = 0;
        pad_read_state = 0;
        pad_input = key_input;
    }
}

uint8_t RAM::read_pad_1(){
    uint8_t data;
    data = pad_input[pad_read_state];
    pad_read_state++;
    return data;
}

uint8_t RAM::read_pad_2(){
    return 0;
}
