//#define _imm() (PC++)
//
//#define _abs()  (PC+=2,read_mem16(opr_pc, WRAM, PPU_RAM))
//#define _abxi() (PC+=2,read_mem16(read_mem16(opr_pc, WRAM, PPU_RAM)+X, WRAM, PPU_RAM))
//#define _abx()  (PC+=2,read_mem16(opr_pc, WRAM, PPU_RAM)+X)
//#define _aby()  (PC+=2,read_mem16(opr_pc, WRAM, PPU_RAM)+Y)
//#define _absi() (PC+=2,read_mem16(read_mem16(opr_pc, WRAM, PPU_RAM), WRAM, PPU_RAM))
//
//#define _zp()   (read(PC++, WRAM, PPU_RAM))
//#define _zpxi() (read_mem16((uint8_t)(read(PC++, WRAM, PPU_RAM)+X), WRAM, PPU_RAM))
//#define _zpx()  ((uint8_t)(read(PC++, WRAM, PPU_RAM)+X))
//#define _zpy()  ((uint8_t)(read(PC++, WRAM, PPU_RAM)+Y))
//#define _zpi()  (read_mem16(read(PC++, WRAM, PPU_RAM), WRAM, PPU_RAM))
//#define _zpiy() (read_mem16(read(PC++, WRAM, PPU_RAM), WRAM, PPU_RAM)+Y)

//#define _push8(dat)  write(0x100|(uint8_t)(SP--),dat, WRAM, PPU_RAM, SP_RAM)
//#define _pop8()      read(0x100|(uint8_t)(++SP), WRAM, PPU_RAM)
//#define _push16(dat) (write_mem16(0x100|(uint8_t)(SP-1),dat, WRAM, PPU_RAM, SP_RAM),SP-=2)
//#define _pop16()     (SP+=2,read_mem16(0x100|(uint8_t)(SP-1), WRAM, PPU_RAM))

#define _bindFlags() (((uint8_t)NFlag<<7)|((uint8_t)VFlag<<6)|0x20|((uint8_t)BFlag<<4)|  \
                     ((uint8_t)DFlag<<3)|((uint8_t)IFlag<<2)|((uint8_t)ZFlag<<1)|CFlag)
#define _unbindFlags(dd) { \
  uint8_t dat=dd; \
  NFlag=dat>>7; \
  VFlag=(dat>>6)&1; \
  BFlag=(dat>>4)&1; \
  DFlag=(dat>>3)&1; \
  IFlag=(dat>>2)&1; /* (iがクリアされた場合、割り込まれる可能性が) */ \
  ZFlag=(dat>>1)&1; \
  CFlag=dat&1; \
}

// TODO : decimal support
#define _adc(data) { \
  uint16_t  s=data; \
  uint16_t t=ACC+s+CFlag; \
  CFlag=(uint8_t)(t>>8); \
  ZFlag=(t&0xff)==0; \
  NFlag=(t>>7)&1; \
  VFlag=!((ACC^s)&0x80)&&((ACC^t)&0x80); \
  ACC=(uint8_t)t; \
}
// TODO : decimal support
#define _sbc(data) { \
  uint16_t  s=data; \
  uint16_t t=ACC-s-(CFlag?0:1); \
  CFlag=t<0x100; \
  ZFlag=(t&0xff)==0; \
  NFlag=(t>>7)&1; \
  VFlag=((ACC^s)&0x80)&&((ACC^t)&0x80); \
  ACC=(uint8_t)t; \
}
#define _cmp(reg,data) { \
  uint16_t t=(uint16_t)reg-data; \
  CFlag=t<0x100; \
  ZFlag=(t&0xff)==0; \
  NFlag=(t>>7)&1; \
}

#define _and(data) { \
  ACC&=data; \
  NFlag=ACC>>7; \
  ZFlag=ACC==0; \
}
#define _ora(data) { \
  ACC|=data; \
  NFlag=ACC>>7; \
  ZFlag=ACC==0; \
}
#define _eor(data) { \
  ACC^=data; \
  NFlag=ACC>>7; \
  ZFlag=ACC==0; \
}

#define _bit(data) { \
  uint8_t t=data; \
  NFlag=t>>7; \
  VFlag=(t>>6)&1; \
  ZFlag=(ACC&t)==0; \
}

#define _load(reg,adr) { \
  reg=((adr >> 15) & 1) ? read_prom(adr, PROM) : read(adr, WRAM, PPU_RAM); \
  NFlag=reg>>7; \
  ZFlag=reg==0; \
}
#define _store(reg,adr) { \
  write(adr,reg, WRAM, PPU_RAM, SP_RAM); \
}

#define _mov(dest,src) { \
  dest=src; \
  NFlag=src>>7; \
  ZFlag=src==0; \
}

#define _asli(arg) \
  CFlag=arg>>7; \
  arg<<=1; \
  NFlag=arg>>7; \
  ZFlag=arg==0;
#define _lsri(arg) \
  CFlag=arg&1; \
  arg>>=1; \
  NFlag=arg>>7; \
  ZFlag=arg==0;
#define _roli(arg) \
  uint8_t u=arg; \
  arg=(arg<<1)|CFlag; \
  CFlag=u>>7; \
  NFlag=arg>>7; \
  ZFlag=arg==0;
#define _rori(arg) \
  uint8_t u=arg; \
  arg=(arg>>1)|((uint8_t)CFlag<<7); \
  CFlag=u&1; \
  NFlag=arg>>7; \
  ZFlag=arg==0;
#define _inci(arg) \
  arg++; \
  NFlag=arg>>7; \
  ZFlag=arg==0;
#define _deci(arg) \
  arg--; \
  NFlag=arg>>7; \
  ZFlag=arg==0;

#define _sfta(reg,op) { op(reg); }
#define _sft(adr, data, op) { \
  uint16_t a=adr; \
  uint8_t t=data; \
  op(t); \
  norm_write8(a,t, WRAM); \
}

#define _asla()    _sfta(ACC,_asli)
#define _asl(adr,data) _sft(adr,data,_asli)
#define _lsra()    _sfta(ACC,_lsri)
#define _lsr(adr,data) _sft(adr,data,_lsri)
#define _rola()    _sfta(ACC,_roli)
#define _rol(adr,data) _sft(adr,data,_roli)
#define _rora()    _sfta(ACC,_rori)
#define _ror(adr,data) _sft(adr,data,_rori)

#define _incr(reg) _sfta(reg,_inci)
#define _inc(adr,data)  _sft(adr,data,_inci)
#define _decr(reg) _sfta(reg,_deci)
#define _dec(adr,data)  _sft(adr,data,_deci)

//#define _bra(cond) { \
//  int8_t rel=(int8_t)WRAM[PC++]; \
//  if (cond){ \
//    PC+=rel; \
//  } \
//}
  //int8_t rel=(int8_t)read_prom(PC++, PROM); 
#define _bra(data) { \
  int8_t rel=(int8_t)data; \
  PC+=rel; \
}
