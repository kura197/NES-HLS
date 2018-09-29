
#include "ram.h"
#include <fstream>
#include <iostream>

using namespace std;



uint8_t RAM::read(uint16_t addr, uint8_t* WRAM, uint8_t* PPU_RAM, struct SPREG* spreg, uint8_t* CROM){
    uint8 data = 0;
    //if(addr < 0x2000)   addr = addr & 0x7FF;
    //else if(addr < 0x4000) addr = addr & 0x2007;
    //if(addr >= 0x8000) printf("read error. addr:%04x\n", addr);
    //uint8_t tmp;
    switch(addr){
        case 0x2002: 
              //data = _set(spreg->VBlank,7)|_set(spreg->SPhit,6)|_set(spreg->num_ScanSP,5);
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
            //if(addr >= 0x800) printf("read error. addr:%04x\n", addr);
            if(addr < 0x800) data = WRAM[addr];
            //printf("addr:%04x\n",addr);
            //if(addr == 0x2002){
            //    tmp = data & ~(1 << 7);
            //    WRAM[addr] = tmp;
            //}
            break;
    }
    return data;
}

void RAM::write(uint16_t addr, uint8_t data, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, struct SPREG* spreg, uint8_t* CROM){
    //if(addr < 0x2000)   addr = addr & 0x7FF;
    //else if(addr < 0x4000) addr = addr & 0x2007;
    //if(addr >= 0x8000) printf("write error. addr:%04x\n", addr);
    switch(addr){
        case 0x2000: 
            write_2000(data, spreg);
            //PPUInc = (bool)((data >> 2) & 1);    
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

//void RAM::set_VBlank(bool vblank, bool nmi){
//    VBlank = vblank;
//    if(nmi && (!VBlank || VBlank_NMI))
//        nes->cpu->set_nmi(VBlank);
//}

void RAM::write_2000(uint8 data, struct SPREG* spreg){
    //spreg->VBlank_NMI = (bool)((data >> 7) & 1);    
    //spreg->BGPtnAddr =  (bool)((data >> 4) & 1);    
    //spreg->SPPtnAddr =  (bool)((data >> 3) & 1);    
    //PPUInc =            (bool)((data >> 2) & 1);    
    //spreg->NameAddrH =  (bool)((data >> 1) & 1);    
    //spreg->NameAddrL =  (bool)((data >> 0) & 1);    
    spreg->VBlank_NMI = data[7];    
    spreg->BGPtnAddr =  data[4];    
    spreg->SPPtnAddr =  data[3];    
    PPUInc =            data[2];    
    spreg->NameAddrH =  data[1];    
    spreg->NameAddrL =  data[0];    
}

void RAM::write_2001(uint8 data, struct SPREG* spreg){
    //spreg->EnSP =      (bool)((data >> 4) & 1);    
    //spreg->EnBG =      (bool)((data >> 3) & 1);    
    //spreg->SPMSK =     (bool)((data >> 2) & 1);    
    //spreg->BGMSK =     (bool)((data >> 1) & 1);    
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
    //write_SPRAM(SPAddr++, data);
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
        //PPUAddr = (uint16_t)data << 8;
        PPUAddr.set_slc(8, data);
        PPUAddr_sel_H = true;
    }
    else{
        //PPUAddr |= data;
        PPUAddr.set_slc(0, data);
        PPUAddr_sel_H = false;
    } 
}

void RAM::write_2007(uint8_t data, uint8_t* PPU_RAM, uint8_t* CROM){
    //uint16_t PPU_Addr = ((uint16_t)PPUAddr_H << 8) | PPUAddr_L;
    //uint32_t addr = PPUAddr & 0x3fff;
    uint16 addr = PPUAddr;
    switch(addr){
        case 0x3f10:
            addr = 0x3f00;
            break;
        case 0x3f14:
            addr = 0x3f04;
            break;
        case 0x3f18:
            addr = 0x3f08;
            break;
        case 0x3f1C:
            addr = 0x3f0C;
            break;
        default:
            break;
    }
    //printf("ppu_ram write addr:%04x data:%02x\n", addr, data);
    if(addr[13])    PPU_RAM[addr & 0x1FFF] = data;
    else CROM[addr] = data;
    PPUAddr = (PPUInc) ? PPUAddr + 32 : PPUAddr + 1;
    //PPUAddr_H = (uint8_t)(PPU_Addr >> 8);
    //PPUAddr_L = (uint8_t)PPU_Addr;
}

uint8_t RAM::read_2007(uint8_t* PPU_RAM, uint8_t* CROM){
    uint8_t data;
    //uint16_t PPU_Addr = ((uint16_t)PPUAddr_H << 8) | PPUAddr_L;
    //uint32_t addr = PPUAddr & 0x3fff;
    uint16 addr = PPUAddr;
    switch(addr){
        case 0x3f10:
            addr = 0x3f00;
            break;
        case 0x3f14:
            addr = 0x3f04;
            break;
        case 0x3f18:
            addr = 0x3f08;
            break;
        case 0x3f1C:
            addr = 0x3f0C;
            break;
        default:
            break;
    }

    //uint8_t tmp_data = PPU_RAM[addr];
    uint8_t tmp_data;
    if(addr[13]) tmp_data = PPU_RAM[addr & 0x1FFF];
    else tmp_data = CROM[addr];
    if(addr < 0x3F00){
        data = spram_buf;
        spram_buf = tmp_data;
    }
    else  data = tmp_data;
    //printf("read: %04x %02x\n", addr, data);
    PPUAddr = (PPUInc) ? PPUAddr + 32 : PPUAddr + 1;
    //PPUAddr_H = (uint8_t)(PPU_Addr >> 8);
    //PPUAddr_L = (uint8_t)PPU_Addr;
    return data;
}

void RAM::DMA_start(uint8_t addr_H, uint8_t* WRAM, uint8_t* SP_RAM){
//    uint16_t addr = ((uint16_t)addr_H << 8);
//#pragma ii 2
//    for(int i = 0; i < 256; i++)
//        SP_RAM[i] = WRAM[addr + i];
//        //write_SPRAM(i, WRAM[addr+i]);
    
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
    //switch(pad_read_state){
    //   //A
    //   case 0: 
    //       data = pad_input[0];
    //       //pad_read_state++;
    //       break;
    //   //B
    //   case 1: 
    //       data = pad_input[1];
    //       //pad_read_state++;
    //       break;
    //   //SELECT
    //   case 2: 
    //       data = pad_input[2];
    //       //pad_read_state++;
    //       break;
    //   //START
    //   case 3: 
    //       data = pad_input[3];
    //       //pad_read_state++;
    //       break;
    //   //UP
    //   case 4: 
    //       data = pad_input[4];
    //       //pad_read_state++;
    //       break;
    //   //DOWN
    //   case 5: 
    //       data = pad_input[5];
    //       //pad_read_state++;
    //       break;
    //   //LEFT
    //   case 6: 
    //       data = pad_input[6];
    //       //pad_read_state++;
    //       break;
    //   //RIGHT
    //   case 7: 
    //       data = pad_input[7];
    //       //pad_read_state = 0;
    //       break;
    //   default:
    //       break;
    //}
    data = pad_input[pad_read_state];
    pad_read_state++;
    return data;
}

uint8_t RAM::read_pad_2(){
    return 0;
}

//void RAM::frame_end(uint8_t* WRAM){
//    //NameAddrH = false;    
//    //NameAddrL = false;    
//    uint8_t tmp = WRAM[0x2000];
//    tmp &= ~0b00000011;
//    WRAM[0x2000] = tmp;
//}

