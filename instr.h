
//#define _bindFlags() (((uint8_t)NFlag<<7)|((uint8_t)VFlag<<6)|0x20|((uint8_t)BFlag<<4)|  \
                     ((uint8_t)DFlag<<3)|((uint8_t)IFlag<<2)|((uint8_t)ZFlag<<1)|CFlag)
//#define _unbindFlags(dd) { \
  uint8_t dat=dd; \
  NFlag=dat>>7; \
  VFlag=(dat>>6)&1; \
  BFlag=(dat>>4)&1; \
  DFlag=(dat>>3)&1; \
  IFlag=(dat>>2)&1; /* (iがクリアされた場合、割り込まれる可能性が) */ \
  ZFlag=(dat>>1)&1; \
  CFlag=dat&1; \
}

//#define _bindFlags(data) { \
    (uint8)data[7] = NFlag; \
    (uint8)data[6] = VFlag; \
    (uint8)data[5] = 1; \
    (uint8)data[4] = BFlag; \
    (uint8)data[3] = DFlag; \
    (uint8)data[2] = IFlag; \
    (uint8)data[1] = ZFlag; \
    (uint8)data[0] = CFlag; \
}
                         

#define _unbindFlags(dd) { \
  uint8 dat=dd; \
  NFlag=dat[7]; \
  VFlag=dat[6]; \
  BFlag=dat[4]; \
  DFlag=dat[3]; \
  IFlag=dat[2]; /* (iがクリアされた場合、割り込まれる可能性が) */ \
  ZFlag=dat[1]; \
  CFlag=dat[0]; \
}

// TODO : decimal support
#define _adc(data) { \
  uint16_t  s=data; \
  uint16 t=ACC+s+CFlag; \
  CFlag=t[8]; \
  ZFlag=(t.slc<8>(0))==0; \
  NFlag=t[7]; \
  VFlag=!((ACC^s)&0x80)&&((ACC^t)&0x80); \
  ACC=(uint8_t)t.slc<8>(0); \
}
// TODO : decimal support
#define _sbc(data) { \
  uint16_t  s=data; \
  uint16 t=ACC-s-(CFlag?0:1); \
  CFlag=t<0x100; \
  ZFlag=(t.slc<8>(0))==0; \
  NFlag=t[7]; \
  VFlag=((ACC^s)&0x80)&&((ACC^t)&0x80); \
  ACC=(uint8_t)t.slc<8>(0); \
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

#define _load(reg,adr,data) { \
  reg = data; \
  NFlag=reg>>7; \
  ZFlag=reg==0; \
}
#define _store(reg,adr) { \
  write(adr,reg, WRAM, PPU_RAM, SP_RAM, spreg, CROM); \
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
  op(data); \
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

#define _bra(data) { \
  int8_t rel=(int8_t)data; \
  PC+=rel; \
}
