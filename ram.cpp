
#include "ram.h"
#include <fstream>
#include <iostream>

using namespace std;


//void RAM::dump_WRAM(uint16_t start_addr, uint16_t size){
//    int i, k;
//    for(i = start_addr, k = 0;i < start_addr + size; i++, k++){
//        if(k % 16 == 0){
//            printf("\n");
//            printf("%04x : ", i);
//        }
//        printf("%02x ",WRAM[i]);
//    }
//    printf("\n");
//}
//
//void RAM::dump_PROM(uint16_t start_addr, uint16_t size){
//    int i, k;
//    for(i = start_addr, k = 0;i < start_addr + size; i++, k++){
//        if(k % 16 == 0){
//            printf("\n");
//            printf("%04x : ", i);
//        }
//        printf("%02x ",PROM[i-0x8000]);
//    }
//    printf("\n");
//}
//
//void RAM::dump_PPURAM(uint16_t start_addr, uint16_t size){
//    int i, k;
//    for(i = start_addr, k = 0;i < start_addr + size; i++, k++){
//        if(k % 16 == 0){
//            printf("\n");
//            printf("%04x : ", i);
//        }
//        printf("%02x ",PPU_RAM[i]);
//    }
//    printf("\n");
//}

uint8_t RAM::read(uint16_t addr, uint8_t* WRAM, uint8_t* PPU_RAM){
    uint8_t data;
    if(addr < 0x2000)   addr = addr & 0x7FF;
    else if(addr < 0x4000) addr = addr & 0x2007;

    uint8_t tmp;
    switch(addr){
        //case 0x2002: 
        //    //data = _set(VBlank,7)|_set(SPhit,6)|_set(num_ScanSP,5);
        //    data = WRAM[addr];
        //    tmp = data & ~(1 << 7);
        //    WRAM[addr] = tmp;
        //    //VBlank = false;
        //    //BGoffset_sel_X = false;
        //    //PPUAddr_sel_H = false;
        //    break;
        case 0x2007:
            data = read_2007(PPU_RAM);
            break;
        case 0x4016:
            data = read_pad_1();
            break;
        case 0x4017:
            data = read_pad_2();
            break;
        default:
            data = WRAM[addr];
            if(addr == 0x2002){
                tmp = data & ~(1 << 7);
                WRAM[addr] = tmp;
            }
            break;
    }
    return data;
}

void RAM::write(uint16_t addr, uint8_t data, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM){
    if(addr < 0x2000)   addr = addr & 0x7FF;
    else if(addr < 0x4000) addr = addr & 0x2007;
    switch(addr){
        case 0x2000: 
            //write_2000(data);
            PPUInc = (bool)((data >> 2) & 1);    
            break;
        //case 0x2001: 
        //    write_2001(data);
        //    break;
        case 0x2003: 
            write_2003(data);
            break;
        case 0x2004: 
            write_2004(data, SP_RAM);
            break;
        case 0x2005: 
            write_2005(data);
            break;
        case 0x2006: 
            write_2006(data);
            break;
        case 0x2007: 
            write_2007(data, PPU_RAM);
            break;
        case 0x4014: 
            DMA_start(data, WRAM, SP_RAM);
            break;
        case 0x4016: 
            reset_pad(data);
            break;
        default:
            break;
    }
    WRAM[addr] = data;
}

//void RAM::set_VBlank(bool vblank, bool nmi){
//    VBlank = vblank;
//    if(nmi && (!VBlank || VBlank_NMI))
//        nes->cpu->set_nmi(VBlank);
//}

//void RAM::write_2000(uint8_t data){
//    //VBlank_NMI = (bool)((data >> 7) & 1);    
//    //SPSize =     (bool)((data >> 5) & 1);    
//    //BGPtnAddr =  (bool)((data >> 4) & 1);    
//    //SPPtnAddr =  (bool)((data >> 3) & 1);    
//    PPUInc =     (bool)((data >> 2) & 1);    
//    //NameAddrH =  (bool)((data >> 1) & 1);    
//    //NameAddrL =  (bool)((data >> 0) & 1);    
//}

//void RAM::write_2001(uint8_t data){
//    //printf("write $2001. data = %02x\n",data);
//    BGColor2 =  (bool)((data >> 7) & 1);    
//    BGColor1 =  (bool)((data >> 6) & 1);    
//    BGColor0 =  (bool)((data >> 5) & 1);    
//    EnSP =      (bool)((data >> 4) & 1);    
//    EnBG =      (bool)((data >> 3) & 1);    
//    SPMSK =     (bool)((data >> 2) & 1);    
//    BGMSK =     (bool)((data >> 1) & 1);    
//    DispType =  (bool)((data >> 0) & 1);    
//}

void RAM::write_2003(uint8_t data){
    SPAddr = data;
}

void RAM::write_2004(uint8_t data, uint8_t* SP_RAM){
    SP_RAM[SPAddr++] = data;
    //write_SPRAM(SPAddr++, data);
}

void RAM::write_2005(uint8_t data){
    if(!BGoffset_sel_X){
        scr.BGoffset_X = data;
        BGoffset_sel_X = true;
    }  
    else{
        scr.BGoffset_Y = data;
        BGoffset_sel_X = false;
    }
}

void RAM::write_2006(uint8_t data){
    if(!PPUAddr_sel_H){
        //PPUAddr_H = data;
        PPUAddr = (uint16_t)data << 8;
        PPUAddr_sel_H = true;
    }
    else{
        //PPUAddr_L = data;
        PPUAddr |= data;
        PPUAddr_sel_H = false;
    } 
}

void RAM::write_2007(uint8_t data, uint8_t* PPU_RAM){
    //uint16_t PPU_Addr = ((uint16_t)PPUAddr_H << 8) | PPUAddr_L;
    uint32_t addr = PPUAddr & 0x3fff;
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
    PPU_RAM[addr] = data;
    PPUAddr = (PPUInc) ? PPUAddr + 32 : PPUAddr + 1;
    //PPUAddr_H = (uint8_t)(PPU_Addr >> 8);
    //PPUAddr_L = (uint8_t)PPU_Addr;
}

uint8_t RAM::read_2007(uint8_t* PPU_RAM){
    uint8_t data;
    //uint16_t PPU_Addr = ((uint16_t)PPUAddr_H << 8) | PPUAddr_L;
    uint32_t addr = PPUAddr & 0x3fff;
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

    uint8_t tmp_data = PPU_RAM[addr];
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
    uint16_t addr = ((uint16_t)addr_H << 8);
    for(int i = 0; i < 256; i++)
        SP_RAM[i] = WRAM[addr + i];
        //write_SPRAM(i, WRAM[addr+i]);
}

void RAM::reset_pad(uint8_t data){
    if(pad_reset_state == 1 && ((data & 1) == 0))
        pad_reset_state = 2;
    
    if(pad_reset_state == 0 && ((data & 1) == 1))
        pad_reset_state = 1;

    if(pad_reset_state == 2){
        pad_reset_state = 0;
        pad_read_state = 0;
        pad_input = key_input;
    }
}

uint8_t RAM::read_pad_1(){
    uint8_t data;
    switch(pad_read_state){
       //A
       case 0: 
           data = (pad_input & 1);
           pad_read_state++;
           break;
       //B
       case 1: 
           data = ((pad_input >> 1) & 1);
           pad_read_state++;
           break;
       //SELECT
       case 2: 
           data = ((pad_input >> 2) & 1);
           pad_read_state++;
           break;
       //START
       case 3: 
           data = ((pad_input >> 3) & 1);
           pad_read_state++;
           break;
       //UP
       case 4: 
           data = ((pad_input >> 4) & 1);
           pad_read_state++;
           break;
       //DOWN
       case 5: 
           data = ((pad_input >> 5) & 1);
           pad_read_state++;
           break;
       //LEFT
       case 6: 
           data = ((pad_input >> 6) & 1);
           pad_read_state++;
           break;
       //RIGHT
       case 7: 
           data = ((pad_input >> 7) & 1);
           pad_read_state = 0;
           break;
       default:
           break;
    }
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

