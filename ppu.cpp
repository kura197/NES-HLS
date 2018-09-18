
#include "ppu.h"

void PPU::load_ppuram(){ 
    PPU_RAM = nes->ram->get_PPU_RAM() - 0x2000; 
}

void PPU::load_spram(){ 
    SP_RAM = nes->ram->get_SP_RAM(); 
}

void PPU::render(uint8_t line){
    //printf("BG:%d\tSP:%d\n",nes->ram->EnBG, nes->ram->EnSP);
    if(nes->ram->EnBG)  bg_render(line);
    if(nes->ram->EnSP)  sp_render(line);
}

#define _pttnbit(data, data_bit, ret_bit) (((data >> data_bit) & 1) << ret_bit)
void PPU::bg_render(uint8_t line){

    uint8_t sc_x = nes->ram->BGoffset_X;
    uint8_t sc_y = nes->ram->BGoffset_Y;
    uint8_t tile_offset = sc_x / 8;
    bool low = nes->ram->NameAddrL;
    bool high = nes->ram->NameAddrH;
    uint16_t name_base = (!low&!high) ? 0x2000 :
                         (low&!high)  ? 0x2400 :
                         (!low&high)  ? 0x2800 :
                         (low&high)   ? 0x2C00 : 0x2000;
    uint16_t attr_base = (!low&!high) ? 0x23C0 :
                         (low&!high)  ? 0x27C0 :
                         (!low&high)  ? 0x2BC0 :
                         (low&high)   ? 0x2FC0 : 0x23C0;
    //printf("name_base:%04x\n",name_base);
    //printf("low:%d\thigh:%d\n",low,high);
    uint8_t x = 0;
    for(int col = 0; col < 33; col++){
        int name_index;
        //uint32_t name;
        uint16_t name;
        int attr_index;
        uint8_t attr;
        uint8_t tile = col + tile_offset;
        if(tile < 32){
            name_index = (line/8)*32 + tile;
            name = 16 * PPU_RAM[name_base + name_index];
            attr_index = (line/32)*8 + tile/4;
            attr = PPU_RAM[attr_base + attr_index];
        }else{
            name_index = (line/8)*32 + col + tile_offset - 32;
            attr_index = (line/32)*8 + tile/4 - 8;
            if(!low&!high){
                name = 16 * PPU_RAM[0x2400 + name_index];
                attr = PPU_RAM[0x27C0 + attr_index];
            }  
            else if(low&!high){
                name = 16 * PPU_RAM[0x2000 + name_index];
                attr = PPU_RAM[0x23C0 + attr_index];
            } 
        }

        uint16_t pttn_base = (nes->ram->BGPtnAddr) ? 0x1000 : 0x0000;
        //uint8_t pttn_L = PPU_RAM[pttn_base + name + (line % 8)];
        //uint8_t pttn_H = PPU_RAM[pttn_base + name + (line % 8) + 8];
        uint8_t pttn_L = CROM[pttn_base + name + (line % 8)];
        uint8_t pttn_H = CROM[pttn_base + name + (line % 8) + 8];
        bool upper = (line % 32) < 16;
        bool left = (tile % 4) < 2;
        uint8_t locate;
        if(upper & left)
            locate = (attr >> 0) & 0x3;
        else if(upper & !left)
            locate = (attr >> 2) & 0x3;
        else if(!upper & left)
            locate = (attr >> 4) & 0x3;
        else if(!upper & !left)
            locate = (attr >> 6) & 0x3;

        uint16_t color_addr_base = (0x3F00 + locate*4);
        uint8_t color;
        //if(112 <= line && line <= 119 && col == 10)printf("line:%d name: %04x    %0x\n",line, 0x2000+name_index, pttn); 
        for(int i = 0; i < 8; i++){
            uint8_t offset = _pttnbit(pttn_H,(7-i),1) | _pttnbit(pttn_L,(7-i),0);
            bool valid = true;
            if(offset != 0) color = PPU_RAM[color_addr_base + offset];
            else { 
                color = PPU_RAM[0x3F00];
                //color = PPU_RAM[color_addr_base];
                valid = false;
            }
            BG_Valid[x] = valid;
            if(col == 0){ 
                if(i >= (sc_x%8)) store_vram(line, x++, color, false); 
            }
            else if(col == 32){
                if(i < (sc_x%9)) store_vram(line, x++, color, false);
            }
            else store_vram(line, x++, color, false);
            //printf("vram_addr = %d\n", 256*line + x);

            //store_vram(line, 8*col, i, color);
            //if(112 <= line && line <= 119 && col == 10)printf("%02x ",color);
        }
        //if(112 <= line && line <= 119 && col == 10)printf("\n");
    }
}

void PPU::sp_render(uint8_t line){
    int num_sp = 0;
    for(int spr = 0; spr < 64; spr++){
        uint8_t spr_x = SP_RAM[4*spr+3];
        uint8_t spr_y = SP_RAM[4*spr];
        uint32_t spr_ptn_index = 16*SP_RAM[4*spr+1];
        //if(line == 0)printf("sprite:%02x\tX:%02x Y:%02x index:%04x\n",spr,spr_x,spr_y,spr_ptn_index);
        if(line < spr_y || spr_y + 8 <= line)
            continue;
        num_sp++;

        uint8_t spr_att = SP_RAM[4*spr+2];
        uint8_t palette = spr_att & 0x3;
        bool bg_priority = (spr_att >> 5) & 0x1;
        bool lr_rev = (spr_att >> 6) & 0x1;
        bool ud_rev = (spr_att >> 7) & 0x1;

        uint32_t pttn_base = (nes->ram->SPPtnAddr) ? 0x1000 : 0x0000;
        uint8_t pttn_offset = (ud_rev) ? 7 - (line - spr_y) : line - spr_y;
        //uint8_t pttn_L = PPU_RAM[pttn_base + spr_ptn_index + (pttn_offset % 8)];
        //uint8_t pttn_H = PPU_RAM[pttn_base + spr_ptn_index + (pttn_offset % 8) + 8];
        uint8_t pttn_L = CROM[pttn_base + spr_ptn_index + (pttn_offset % 8)];
        uint8_t pttn_H = CROM[pttn_base + spr_ptn_index + (pttn_offset % 8) + 8];
        //printf("pttn_H addr : %02x\n",pttn_base + spr_ptn_index + (pttn_offset % 8) + 8);
        //printf("ptn_index : %02x\n",spr_ptn_index);

        uint16_t color_addr_base = (0x3F10 + palette*4);
        uint8_t color;
        for(int i = 0; i < 8; i++){
            int8_t offset;
            if(lr_rev) offset = _pttnbit(pttn_H,i,1) | _pttnbit(pttn_L,i,0);
            else offset = _pttnbit(pttn_H,(7-i),1) | _pttnbit(pttn_L,(7-i),0);
            //printf("H:%02x\tL:%02x\n",pttn_H,pttn_L);
            if(offset == 0) continue;
            //if(offset == 0) offset = -16;
            //if(spr == 0 && BG_Valid[spr_x+i]) nes->ram->SP_hit();
            if(spr == 0) nes->ram->SP_hit();
            color = PPU_RAM[color_addr_base + offset];
            if(!(bg_priority & BG_Valid[spr_x+i]))store_vram(line, spr_x+i, color, true);
        }
    }
    if(num_sp >= 9) nes->ram->num_ScanSP = true;
    else nes->ram->num_ScanSP = false;
}

#define _rgb(r, g, b) (red = r, green = g, blue = b)
void PPU::store_vram(uint8_t line, uint8_t x, uint8_t color, bool sprite){
    //BGR
    if(x < 8 && ((!sprite & nes->ram->BGMSK) || (sprite & nes->ram->SPMSK)))
        VRAM[256*line + x] = 0x3F;
    else 
        VRAM[256*line + x] = color;
    //uint8_t blue, green, red; 
    ////printf("%02x\n",color);
    //switch(color){
    //    case 0x00: _rgb(0x75,0x75,0x75); break;
    //    case 0x01: _rgb(0x27,0x1B,0x8F); break;
    //    case 0x02: _rgb(0x00,0x00,0xAB); break;
    //    case 0x03: _rgb(0x47,0x00,0x9F); break;
    //    case 0x04: _rgb(0x8F,0x00,0x77); break;
    //    case 0x05: _rgb(0xAB,0x00,0x13); break;
    //    case 0x06: _rgb(0xA7,0x00,0x00); break;
    //    case 0x07: _rgb(0x7F,0x0B,0x00); break;
    //    case 0x08: _rgb(0x43,0x2F,0x00); break;
    //    case 0x09: _rgb(0x00,0x47,0x00); break;
    //    case 0x0a: _rgb(0x00,0x51,0x00); break;
    //    case 0x0b: _rgb(0x00,0x3F,0x17); break;
    //    case 0x0c: _rgb(0x1B,0x3F,0x5F); break;
    //    case 0x0d: _rgb(0x00,0x00,0x00); break;
    //    case 0x0e: _rgb(0x00,0x00,0x00); break;
    //    case 0x0f: _rgb(0x00,0x00,0x00); break;

    //    case 0x10: _rgb(0xBC,0xBC,0xBC); break;
    //    case 0x11: _rgb(0x00,0x73,0xEF); break;
    //    case 0x12: _rgb(0x23,0x3B,0xEF); break;
    //    case 0x13: _rgb(0x83,0x00,0xF3); break;
    //    case 0x14: _rgb(0xBF,0x00,0xBF); break;
    //    case 0x15: _rgb(0xE7,0x00,0x5B); break;
    //    case 0x16: _rgb(0xDB,0x2B,0x00); break;
    //    case 0x17: _rgb(0xCB,0x4F,0x0F); break;
    //    case 0x18: _rgb(0x8B,0x73,0x00); break;
    //    case 0x19: _rgb(0x00,0x97,0x00); break;
    //    case 0x1a: _rgb(0x00,0xAB,0x00); break;
    //    case 0x1b: _rgb(0x00,0x93,0x3B); break;
    //    case 0x1c: _rgb(0x00,0x83,0x8B); break;
    //    case 0x1d: _rgb(0x00,0x00,0x00); break;
    //    case 0x1e: _rgb(0x00,0x00,0x00); break;
    //    case 0x1f: _rgb(0x00,0x00,0x00); break;

    //    case 0x20: _rgb(0xFF,0xFF,0xFF); break;
    //    case 0x21: _rgb(0x3F,0xBF,0xFF); break;
    //    case 0x22: _rgb(0x5F,0x73,0xFF); break;
    //    case 0x23: _rgb(0xA7,0x8B,0xFD); break;
    //    case 0x24: _rgb(0xF7,0x7B,0xFF); break;
    //    case 0x25: _rgb(0xFF,0x77,0xB7); break;
    //    case 0x26: _rgb(0xFF,0x77,0x63); break;
    //    case 0x27: _rgb(0xFF,0x9B,0x3B); break;
    //    case 0x28: _rgb(0xF3,0xBF,0x3F); break;
    //    case 0x29: _rgb(0x83,0xD3,0x13); break;
    //    case 0x2a: _rgb(0x4F,0xDF,0x4B); break;
    //    case 0x2b: _rgb(0x58,0xF8,0x98); break;
    //    case 0x2c: _rgb(0x00,0xEB,0xDB); break;
    //    case 0x2d: _rgb(0x75,0x75,0x75); break;
    //    case 0x2e: _rgb(0x00,0x00,0x00); break;
    //    case 0x2f: _rgb(0x00,0x00,0x00); break;

    //    case 0x30: _rgb(0xFF,0xFF,0xFF); break;
    //    case 0x31: _rgb(0xAB,0xE7,0xFF); break;
    //    case 0x32: _rgb(0xC7,0xD7,0xFF); break;
    //    case 0x33: _rgb(0xD7,0xCB,0xFF); break;
    //    case 0x34: _rgb(0xFF,0xC7,0xFF); break;
    //    case 0x35: _rgb(0xFF,0xC7,0xDB); break;
    //    case 0x36: _rgb(0xFF,0xBF,0xB3); break;
    //    case 0x37: _rgb(0xFF,0xDB,0xAB); break;
    //    case 0x38: _rgb(0xFF,0xE7,0xA3); break;
    //    case 0x39: _rgb(0xE3,0xFF,0xA3); break;
    //    case 0x3a: _rgb(0xAB,0xF3,0xBF); break;
    //    case 0x3b: _rgb(0xB3,0xFF,0xCF); break;
    //    case 0x3c: _rgb(0x9F,0xFF,0xF3); break;
    //    case 0x3d: _rgb(0xBC,0xBC,0xBC); break;
    //    case 0x3e: _rgb(0x00,0x00,0x00); break;
    //    case 0x3f: _rgb(0x00,0x00,0x00); break;

    //    default: _rgb(0,0,0); break;
    //}
    //if(x < 8 && ((!sprite & nes->ram->BGMSK) || (sprite & nes->ram->SPMSK))){
    //    VRAM[3*(256*(239-line) + x)] = 0;
    //    VRAM[3*(256*(239-line) + x) + 1] = 0;
    //    VRAM[3*(256*(239-line) + x) + 2] = 0;
    //}
    //else{
    //    if(!en_gray){
    //        VRAM[3*(256*(239-line) + x)] = blue;
    //        VRAM[3*(256*(239-line) + x) + 1] = green;
    //        VRAM[3*(256*(239-line) + x) + 2] = red;
    //    }
    //    else{
    //        uint8_t value = 0.229*red + 0.587*green + 0.114*blue;
    //        VRAM[3*(256*(239-line) + x)] = value;
    //        VRAM[3*(256*(239-line) + x) + 1] = value;
    //        VRAM[3*(256*(239-line) + x) + 2] = value;
    //    }
    //    VRAM_gray[256*(239-line) + x] = 0.229*red + 0.587*green + 0.114*blue;
    //}
}


