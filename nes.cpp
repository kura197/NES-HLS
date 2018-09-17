#include "nes.h"


NES::NES(uint8_t* PROM, uint8_t* CROM, uint8_t* VRAM){
    //cpu = new CPU(this);
    //ram = new RAM(this, PROM, CROM);
    //ppu = new PPU(this, VRAM, CROM);
}

NES::~NES(){
    //delete cpu;
    //delete ram;
    //delete ppu;
}


void NES::dump_WRAM(uint16_t start_addr, uint16_t size){
    ram->dump_WRAM(start_addr, size);
}

void NES::dump_PPURAM(uint16_t start_addr, uint16_t size){
    ram->dump_PPURAM(start_addr, size);
}

void NES::enlog(){
    cpu->enlog();
}

void NES::nes_reset(){
    cpu->reset();
}

void NES::exec_frame(){
    int line;
    ram->set_VBlank(false,true);
    for(line = 0; line < 240; line++){
        cpu->exec(114);
        ppu->render(line);
    }
    for(; line < 262; line++){
        if (line==241){
            ram->set_VBlank(true,false);
            cpu->exec(0); 
            ram->set_VBlank(true,true);
            cpu->exec(114);
        }
        else
            cpu->exec(114);
    }
    SP_hit_reset();
    ram->frame_end();
}

void NES::make_bmp(ofstream *bmp){
    ppu->make_bmp(bmp);
}

void NES::SP_hit_reset(){
    ram->SP_hit_reset();
}

uint8_t* NES::get_VRAM(){ 
    return ppu->get_VRAM(); 
}

void NES::enable_gray(){
    ppu->enable_gray();
}

