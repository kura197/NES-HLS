
#include "ppu.h"

#define get_bit(data, bit) ((data >> bit) & 1)
bool PPU::render(uint8_t* PPU_RAM, uint8_t* SP_RAM, uint6* VRAM, struct SPREG* spreg, uint8_t* CROM){
    bool nmi = false;
    line++;
    if(line == 240){
        spreg->VBlank = true;
        if(spreg->VBlank_NMI)
            nmi = true;
    }
    else if(line == 255){
        spreg->SPhit = false;
        spreg->NameAddrL = false;
        spreg->NameAddrH = false;
    }
    if(line < 240){
        bool BGen = spreg->EnBG;
        bool SPen = spreg->EnSP;
        if(BGen)  bg_render(line, spreg, PPU_RAM, VRAM, CROM);
        if(SPen)  sp_render(line, spreg, PPU_RAM, SP_RAM, VRAM, CROM);
    }
    return nmi;
}

#define _pttnbit(data, data_bit, ret_bit) (((data >> data_bit) & 1) << ret_bit)
void PPU::bg_render(uint8_t line, struct SPREG* spreg, uint8_t* PPU_RAM, uint6* VRAM, uint8_t* CROM){
    uint8_t sc_x = spreg->BGoffset_X;
    uint8_t sc_y = spreg->BGoffset_Y;
    uint8_t tile_offset = sc_x / 8;
    bool low = spreg->NameAddrL;
    bool high = spreg->NameAddrH;
    uint16_t name_base = (!low&!high) ? 0x2000 :
                         (low&!high)  ? 0x2400 :
                         (!low&high)  ? 0x2800 :
                         (low&high)   ? 0x2C00 : 0x2000;
    uint16_t attr_base = (!low&!high) ? 0x23C0 :
                         (low&!high)  ? 0x27C0 :
                         (!low&high)  ? 0x2BC0 :
                         (low&high)   ? 0x2FC0 : 0x23C0;
    //uint8_t x = 0;
    for(int col = 0; col < 33; col++){
        int name_index;
        uint16_t name;
        int attr_index;
        uint8_t attr;
        uint8_t tile = col + tile_offset;
        if(tile < 32){
            name_index = (line/8)*32 + tile;
            //name = 16 * PPU_RAM[name_base + name_index];
            name = 16 * read_PPURAM(name_base + name_index, PPU_RAM);
            attr_index = (line/32)*8 + tile/4;
            //attr = PPU_RAM[attr_base + attr_index];
            attr = read_PPURAM(attr_base + attr_index, PPU_RAM);
        }else{
            name_index = (line/8)*32 + col + tile_offset - 32;
            attr_index = (line/32)*8 + tile/4 - 8;
            if(!low&!high){
                //name = 16 * PPU_RAM[0x2400 + name_index];
                //attr = PPU_RAM[0x27C0 + attr_index];
                name = 16 * read_PPURAM(0x2400 + name_index, PPU_RAM);
                attr = read_PPURAM(0x27C0 + attr_index, PPU_RAM);
            }  
            else if(low&!high){
                //name = 16 * PPU_RAM[0x2000 + name_index];
                //attr = PPU_RAM[0x23C0 + attr_index];
                name = 16 * read_PPURAM(0x2000 + name_index, PPU_RAM);
                attr = read_PPURAM(0x23C0 + attr_index, PPU_RAM);
            } 
        }

        uint16_t pttn_base = (spreg->BGPtnAddr) ? 0x1000 : 0x0000;
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
        for(int i = 0; i < 8; i++){
            uint8_t offset = _pttnbit(pttn_H,(7-i),1) | _pttnbit(pttn_L,(7-i),0);
            bool valid = true;
            if(offset != 0) color = read_PPURAM(color_addr_base + offset, PPU_RAM);
            else { 
                color = read_PPURAM(0x3F00, PPU_RAM);
                valid = false;
            }
            bool render_en = false;
            if(col == 0){ 
                if(i >= (sc_x%8)) render_en = true;
            }
            else if(col == 32){
                if(i < (sc_x%9)) render_en = true;
            }
            else render_en = true;
            
            uint8_t x = col*8 + i;
            BG_Valid[x] = valid;
            if(render_en) store_vram(line, x, color, false, VRAM, spreg);
        }
    }
}

void PPU::sp_render(uint8_t line, struct SPREG* spreg, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint6* VRAM, uint8_t* CROM){
    int num_sp = 0;
    for(int spr = 0; spr < 64; spr++){
        uint8_t spr_y = SP_RAM[4*spr];
        if(line < spr_y || spr_y + 8 <= line)
            continue;
        num_sp++;

        uint8_t spr_x = SP_RAM[4*spr+3];
        uint32_t spr_ptn_index = 16*SP_RAM[4*spr+1];
        uint8_t spr_att = SP_RAM[4*spr+2];
        uint8_t palette = spr_att & 0x3;
        bool bg_priority = (spr_att >> 5) & 0x1;
        bool lr_rev = (spr_att >> 6) & 0x1;
        bool ud_rev = (spr_att >> 7) & 0x1;

        uint16_t pttn_base = (spreg->SPPtnAddr) ? 0x1000 : 0x0000;
        uint8_t pttn_offset = (ud_rev) ? 7 - (line - spr_y) : line - spr_y;
        //uint8_t pttn_L = PPU_RAM[pttn_base + spr_ptn_index + (pttn_offset % 8)];
        //uint8_t pttn_H = PPU_RAM[pttn_base + spr_ptn_index + (pttn_offset % 8) + 8];
        uint8_t pttn_L = CROM[pttn_base + spr_ptn_index + (pttn_offset % 8)];
        uint8_t pttn_H = CROM[pttn_base + spr_ptn_index + (pttn_offset % 8) + 8];

        uint16_t color_addr_base = (0x3F10 + palette*4);
        uint8_t color;
        for(int i = 0; i < 8; i++){
            int8_t offset;
            if(lr_rev) offset = _pttnbit(pttn_H,i,1) | _pttnbit(pttn_L,i,0);
            else offset = _pttnbit(pttn_H,(7-i),1) | _pttnbit(pttn_L,(7-i),0);
            if(offset == 0) continue;
            if(spr == 0) spreg->SPhit = true;
            color = read_PPURAM(color_addr_base + offset, PPU_RAM);
            if(!(bg_priority & BG_Valid[spr_x+i])) store_vram(line, spr_x+i, color, true, VRAM, spreg);
        }
    }
    if(num_sp >= 9) spreg->num_ScanSP = true;
    else spreg->num_ScanSP = false;
}

#define _rgb(r, g, b) (red = r, green = g, blue = b)
void PPU::store_vram(uint8_t line, uint8_t x, uint8_t color, bool sprite, uint6* VRAM, struct SPREG* spreg){
    //BGR
    //if(x < 8 && ((!sprite & nes->ram->BGMSK) || (sprite & nes->ram->SPMSK)))
    bool SPMSK = spreg->SPMSK;
    bool BGMSK = spreg->BGMSK;
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

uint8_t PPU::read_PPURAM(uint16_t addr, uint8_t* PPU_RAM){
    return PPU_RAM[addr & 0x1FFF];
}
