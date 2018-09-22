
#include "ppu.h"

#define get_bit(data, bit) ((data >> bit) & 1)
bool PPU::render(uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* VRAM, uint8_t BG_offset_x, uint8_t BG_offset_y){
    //if(line < 256) line++;
    //else line = 0;
    bool nmi = false;
    line++;
    if(line == 240){
        set_bit(WRAM, 0x2002, 7);
        if(get_bit(WRAM[0x2000], 7))
            nmi = true;
    }
    else if(line == 255){
        clr_bit(WRAM, 0x2002, 6);
        //clr_bit(WRAM, 0x2000, 1);
        //clr_bit(WRAM, 0x2000, 0);
        WRAM[0x2000] &= 0xFC;
    }
    //if(nes->ram->EnBG)  bg_render(line);
    //if(nes->ram->EnSP)  sp_render(line);
    if(line < 240){
        uint16_t ctrlreg1 = WRAM[0x2000];
        uint8_t ctrlreg2 = WRAM[0x2001];
        bool BGen = (ctrlreg2 >> 3) & 1;
        bool SPen = (ctrlreg2 >> 4) & 1;
        if(BGen)  bg_render(line, ctrlreg1, ctrlreg2, WRAM, PPU_RAM, VRAM, BG_offset_x, BG_offset_y);
        if(SPen)  sp_render(line, ctrlreg1, ctrlreg2, WRAM, PPU_RAM, SP_RAM, VRAM);
    }
    return nmi;
}

#define _pttnbit(data, data_bit, ret_bit) (((data >> data_bit) & 1) << ret_bit)
void PPU::bg_render(uint8_t line, uint8_t ctrlreg1, uint8_t ctrlreg2, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* VRAM, uint8_t BG_offset_x, uint8_t BG_offset_y){

    uint8_t sc_x = BG_offset_x;
    uint8_t sc_y = BG_offset_y;
    uint8_t tile_offset = sc_x / 8;
    //bool low = nes->ram->NameAddrL;
    //bool high = nes->ram->NameAddrH;
    //bool low = get_bit(WRAM[0x2000], 0);
    //bool high = get_bit(WRAM[0x2000], 1);
    bool low = ctrlreg1 & 1;
    bool high = (ctrlreg1 >> 1) & 1;
    uint16_t name_base = (!low&!high) ? 0x2000 :
                         (low&!high)  ? 0x2400 :
                         (!low&high)  ? 0x2800 :
                         (low&high)   ? 0x2C00 : 0x2000;
    uint16_t attr_base = (!low&!high) ? 0x23C0 :
                         (low&!high)  ? 0x27C0 :
                         (!low&high)  ? 0x2BC0 :
                         (low&high)   ? 0x2FC0 : 0x23C0;
    uint8_t x = 0;
    for(int col = 0; col < 33; col++){
        int name_index;
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

        //uint16_t pttn_base = (nes->ram->BGPtnAddr) ? 0x1000 : 0x0000;
        //uint16_t pttn_base = (get_bit(WRAM[0x2000], 4)) ? 0x1000 : 0x0000;
        uint16_t pttn_base = ((ctrlreg1 >> 4) & 1) ? 0x1000 : 0x0000;
        uint8_t pttn_L = PPU_RAM[pttn_base + name + (line % 8)];
        uint8_t pttn_H = PPU_RAM[pttn_base + name + (line % 8) + 8];
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
            bool render_en = false;
            if(col == 0){ 
                if(i >= (sc_x%8)) render_en = true;
            }
            else if(col == 32){
                if(i < (sc_x%9)) render_en = true;
            }
            else render_en = true;
            
            if(render_en) store_vram(line, x++, color, false, WRAM, VRAM, ctrlreg2);
        }
    }
}

void PPU::sp_render(uint8_t line, uint8_t ctrlreg1, uint8_t ctrlreg2, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* VRAM){
    int num_sp = 0;
    for(int spr = 0; spr < 64; spr++){
        uint8_t spr_x = SP_RAM[4*spr+3];
        uint8_t spr_y = SP_RAM[4*spr];
        uint32_t spr_ptn_index = 16*SP_RAM[4*spr+1];
        if(line < spr_y || spr_y + 8 <= line)
            continue;
        num_sp++;

        uint8_t spr_att = SP_RAM[4*spr+2];
        uint8_t palette = spr_att & 0x3;
        bool bg_priority = (spr_att >> 5) & 0x1;
        bool lr_rev = (spr_att >> 6) & 0x1;
        bool ud_rev = (spr_att >> 7) & 0x1;

        //uint32_t pttn_base = (nes->ram->SPPtnAddr) ? 0x1000 : 0x0000;
        //uint32_t pttn_base = (get_bit(WRAM[0x2000], 3)) ? 0x1000 : 0x0000;
        uint32_t pttn_base = ((ctrlreg1 >> 3) & 1) ? 0x1000 : 0x0000;
        uint8_t pttn_offset = (ud_rev) ? 7 - (line - spr_y) : line - spr_y;
        uint8_t pttn_L = PPU_RAM[pttn_base + spr_ptn_index + (pttn_offset % 8)];
        uint8_t pttn_H = PPU_RAM[pttn_base + spr_ptn_index + (pttn_offset % 8) + 8];
        //printf("pttn_H addr : %02x\n",pttn_base + spr_ptn_index + (pttn_offset % 8) + 8);
        //printf("ptn_index : %02x\n",spr_ptn_index);

        uint16_t color_addr_base = (0x3F10 + palette*4);
        uint8_t color;
        for(int i = 0; i < 8; i++){
            int8_t offset;
            if(lr_rev) offset = _pttnbit(pttn_H,i,1) | _pttnbit(pttn_L,i,0);
            else offset = _pttnbit(pttn_H,(7-i),1) | _pttnbit(pttn_L,(7-i),0);
            if(offset == 0) continue;
            //if(spr == 0) SP_hit();
            if(spr == 0) set_bit(WRAM, 0x2002, 6);
            color = PPU_RAM[color_addr_base + offset];
            if(!(bg_priority & BG_Valid[spr_x+i])) store_vram(line, spr_x+i, color, true, WRAM, VRAM, ctrlreg2);
        }
    }
    //if(num_sp >= 9) nes->ram->num_ScanSP = true;
    //else nes->ram->num_ScanSP = false;
    if(num_sp >= 9) set_bit(WRAM, 0x2002, 5);
    else clr_bit(WRAM, 0x2002, 5);
}

#define _rgb(r, g, b) (red = r, green = g, blue = b)
void PPU::store_vram(uint8_t line, uint8_t x, uint8_t color, bool sprite, uint8_t* WRAM, uint8_t* VRAM, uint8_t ctrlreg2){
    //BGR
    //if(x < 8 && ((!sprite & nes->ram->BGMSK) || (sprite & nes->ram->SPMSK)))
    bool SPMSK = (ctrlreg2 >> 2) & 1;
    bool BGMSK = (ctrlreg2 >> 1) & 1;
    if(x < 8 && ((!sprite & BGMSK) || (sprite & SPMSK)))
        VRAM[256*line + x] = 0x3F;
    else 
        VRAM[256*line + x] = color;
}

void PPU::set_bit(uint8_t* WRAM, uint16_t addr, uint8_t bit){
    //uint8_t tmp = WRAM[addr];
    //tmp |= (1 << bit);
    //WRAM[addr] = tmp;
    WRAM[addr] |= (1 << bit);
}

void PPU::clr_bit(uint8_t* WRAM, uint16_t addr, uint8_t bit){
    //uint8_t tmp = WRAM[addr];
    //tmp &= ~(1 << bit);
    //WRAM[addr] = tmp;
    WRAM[addr] &= ~(1 << bit);
}
