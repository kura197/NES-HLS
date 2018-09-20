#define _imm() (PC++)

#define _abs()  (PC+=2,read_mem16(opr_pc, WRAM, PPU_RAM))
#define _abxi() (PC+=2,read_mem16(read_mem16(opr_pc)+X, WRAM, PPU_RAM))
#define _abx()  (PC+=2,read_mem16(opr_pc, WRAM, PPU_RAM)+X)
#define _aby()  (PC+=2,read_mem16(opr_pc, WRAM, PPU_RAM)+Y)
#define _absi() (PC+=2,read_mem16(read_mem16(opr_pc, WRAM, PPU_RAM), WRAM, PPU_RAM))

#define _zp()   (read(PC++, WRAM, PPU_RAM))
#define _zpxi() (read_mem16((uint8_t)(read(PC++, WRAM, PPU_RAM)+X), WRAM, PPU_RAM))
#define _zpx()  ((uint8_t)(read(PC++, WRAM, PPU_RAM)+X))
#define _zpy()  ((uint8_t)(read(PC++, WRAM, PPU_RAM)+Y))
#define _zpi()  (read_mem16(read(PC++, WRAM, PPU_RAM)))
#define _zpiy() (read_mem16(read(PC++, WRAM, PPU_RAM), WRAM, PPU_RAM)+Y)

#define _push8(dat)  write(0x100|(uint8_t)(SP--),dat, WRAM, PPU_RAM, SP_RAM)
#define _pop8()      read(0x100|(uint8_t)(++SP), WRAM, PPU_RAM)
#define _push16(dat) (write_mem16(0x100|(uint8_t)(SP-1),dat, WRAM, PPU_RAM, SP_RAM),SP-=2)
#define _pop16()     (SP+=2,read_mem16(0x100|(uint8_t)(SP-1), WRAM, PPU_RAM))

#define _bindFlags() ((NFlag<<7)|(VFlag<<6)|0x20|(BFlag<<4)|(DFlag<<3)|(IFlag<<2)|(ZFlag<<1)|CFlag)
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
#define _adc(cycle,adr) { \
  uint16_t  s=read(adr, WRAM, PPU_RAM); \
  uint16_t t=ACC+s+CFlag; \
  CFlag=(uint8_t)(t>>8); \
  ZFlag=(t&0xff)==0; \
  NFlag=(t>>7)&1; \
  VFlag=!((ACC^s)&0x80)&&((ACC^t)&0x80); \
  ACC=(uint8_t)t; \
  rest-=cycle; \
}
// TODO : decimal support
#define _sbc(cycle,adr) { \
  uint16_t  s=read(adr, WRAM, PPU_RAM); \
  uint16_t t=ACC-s-(CFlag?0:1); \
  CFlag=t<0x100; \
  ZFlag=(t&0xff)==0; \
  NFlag=(t>>7)&1; \
  VFlag=((ACC^s)&0x80)&&((ACC^t)&0x80); \
  ACC=(uint8_t)t; \
  rest-=cycle; \
}
#define _cmp(cycle,reg,adr) { \
  uint16_t t=(uint16_t)reg-read(adr, WRAM, PPU_RAM); \
  CFlag=t<0x100; \
  ZFlag=(t&0xff)==0; \
  NFlag=(t>>7)&1; \
  rest-=cycle; \
}

#define _and(cycle,adr) { \
  ACC&=read(adr, WRAM, PPU_RAM); \
  NFlag=ACC>>7; \
  ZFlag=ACC==0; \
  rest-=cycle; \
}
#define _ora(cycle,adr) { \
  ACC|=read(adr, WRAM, PPU_RAM); \
  NFlag=ACC>>7; \
  ZFlag=ACC==0; \
  rest-=cycle; \
}
#define _eor(cycle,adr) { \
  ACC^=read(adr, WRAM, PPU_RAM); \
  NFlag=ACC>>7; \
  ZFlag=ACC==0; \
  rest-=cycle; \
}

#define _bit(cycle,adr) { \
  uint8_t t=read(adr, WRAM, PPU_RAM); \
  NFlag=t>>7; \
  VFlag=(t>>6)&1; \
  ZFlag=(ACC&t)==0; \
  rest-=cycle; \
}

#define _load(cycle,reg,adr) { \
  reg=read(adr, WRAM, PPU_RAM); \
  NFlag=reg>>7; \
  ZFlag=reg==0; \
  rest-=cycle; \
}
#define _store(cycle,reg,adr) { \
  write(adr,reg, WRAM, PPU_RAM, SP_RAM); \
  rest-=cycle; \
}

#define _mov(cycle,dest,src) { \
  dest=src; \
  NFlag=src>>7; \
  ZFlag=src==0; \
  rest-=cycle; \
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
  arg=(arg>>1)|(CFlag<<7); \
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

#define _sfta(cycle,reg,op) { op(reg);rest-=cycle; }
#define _sft(cycle,adr,op) { \
  uint16_t a=adr; \
  uint8_t t=read(a, WRAM, PPU_RAM); \
  op(t); \
  write(a,t, WRAM, PPU_RAM, SP_RAM); \
  rest-=cycle; \
}

#define _asla(cycle)    _sfta(cycle,ACC,_asli)
#define _asl(cycle,adr) _sft(cycle,adr,_asli)
#define _lsra(cycle)    _sfta(cycle,ACC,_lsri)
#define _lsr(cycle,adr) _sft(cycle,adr,_lsri)
#define _rola(cycle)    _sfta(cycle,ACC,_roli)
#define _rol(cycle,adr) _sft(cycle,adr,_roli)
#define _rora(cycle)    _sfta(cycle,ACC,_rori)
#define _ror(cycle,adr) _sft(cycle,adr,_rori)

#define _incr(cycle,reg) _sfta(cycle,reg,_inci)
#define _inc(cycle,adr)  _sft(cycle,adr,_inci)
#define _decr(cycle,reg) _sfta(cycle,reg,_deci)
#define _dec(cycle,adr)  _sft(cycle,adr,_deci)

#define _bra(cycle,cond) { \
  int8_t rel=(int8_t)read(_imm(), WRAM, PPU_RAM); \
  rest-=cycle; \
  if (cond){ \
    rest-=(PC&0xff00)==((PC+rel)&0xff)?1:2; \
    PC+=rel; \
  } \
}
