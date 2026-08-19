/* AUTO-GENERATED compact exact-context AOT semantic templates. */
/* No ROM opcode decode, learning, or emulator fallback. */
#include "civilization_internal.h"
#include "civilization_generated_core.h"
#include "civilization_compact_aot.h"

static int civ_aot_template_000(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,p[0],2u,p[1]);
    i->cpu.pc=(i->cpu.p&CIV_P_Z)?p[2]:p[3];
    i->instruction_count++;return 1;
}

static int civ_aot_template_001(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA9u,2u,p[0]);
    i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|p[1]);
    civ_set_nz8(i,(uint8_t)i->cpu.a);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_002(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x8Du,3u,p[0]);
    if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),(uint8_t)i->cpu.a))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_003(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x22u,4u,p[0]);
    if(!civ_push8(i,p[1]))return 0;
    if(!civ_push8(i,p[2]))return 0;
    if(!civ_push8(i,p[3]))return 0;
    i->cpu.pbr=p[4];i->cpu.pc=p[5];
    i->instruction_count++;return 1;
}

static int civ_aot_template_004(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xC2u,2u,p[0]);
    i->cpu.p &= (uint8_t)~p[1];
    if(i->cpu.e)i->cpu.p|=(CIV_P_M|CIV_P_X);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_005(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA9u,3u,p[0]);
    i->cpu.a=p[1];
    civ_set_nz16(i,i->cpu.a);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_006(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA2u,3u,p[0]);
    i->cpu.x=p[1];
    civ_set_nz16(i,i->cpu.x);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_007(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE2u,2u,p[0]);
    i->cpu.p |= p[1];
    if(i->cpu.e)i->cpu.p|=(CIV_P_M|CIV_P_X);
    if(i->cpu.p&CIV_P_X){i->cpu.x&=0x00FFu;i->cpu.y&=0x00FFu;}
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_008(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x8Du,3u,p[0]);
    if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),i->cpu.a))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_009(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,p[0],p[1],p[2]);
    i->cpu.pc=p[3];
    i->instruction_count++;return 1;
}

static int civ_aot_template_00A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xADu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_00B(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,p[0],3u,p[1]);
    if(!civ_push8(i,p[2]))return 0;
    if(!civ_push8(i,p[3]))return 0;
    i->cpu.pc=p[4];
    i->instruction_count++;return 1;
}

static int civ_aot_template_00C(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x8Eu,3u,p[0]);
    if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),i->cpu.x))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_00D(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x28u,1u,0x000000u);
    { uint8_t v; if(!civ_pull8(i,&v))return 0; i->cpu.p=v;
      if(i->cpu.e){i->cpu.p|=(CIV_P_M|CIV_P_X);}
      if(i->cpu.p&CIV_P_X){i->cpu.x&=0x00FFu;i->cpu.y&=0x00FFu;} }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_00E(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x08u,1u,0x000000u);
    if(!civ_push8(i,i->cpu.p))return 0;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_00F(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x85u,2u,p[0]);
    if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]),i->cpu.a))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_010(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xBFu,4u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(p[0]+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_011(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xADu,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_012(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x18u,1u,0x000000u);
    i->cpu.p &= (uint8_t)~CIV_P_C;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_013(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xBFu,4u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(p[0]+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_014(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Cu,3u,p[0]);
    if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),0u))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_015(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x60u,1u,0x000000u);
    { uint8_t lo,hi; uint16_t target; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi))return 0;
      target=(uint16_t)((uint16_t)(lo|((uint16_t)hi<<8))+1u);
      if(!civ_generated_return_allowed(p[0],((uint32_t)i->cpu.pbr<<16)|target))return civ_fail_frontier(i,"RTS returned outside the finite Civilization proof set.",NULL);
      i->cpu.pc=target; }
    i->instruction_count++;return 1;
}

static int civ_aot_template_016(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x0Au,1u,0x000000u);
    { uint16_t v=i->cpu.a; if(v&0x8000u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v<<1); i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_017(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x29u,3u,p[0]);
    i->cpu.a=(uint16_t)(i->cpu.a&p[1]);
    civ_set_nz16(i,i->cpu.a);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_018(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xAAu,1u,0x000000u);
    i->cpu.x=i->cpu.a;
    civ_set_nz16(i,i->cpu.x);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_019(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xAEu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; i->cpu.x=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_01A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xC9u,3u,p[0]);
    civ_cmp16(i,i->cpu.a,p[1]);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_01B(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE8u,1u,0x000000u);
    i->cpu.x=(uint16_t)(i->cpu.x+1u);
    civ_set_nz16(i,i->cpu.x);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_01C(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA0u,3u,p[0]);
    i->cpu.y=p[1];
    civ_set_nz16(i,i->cpu.y);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_01D(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,p[0],2u,p[1]);
    i->cpu.pc=(i->cpu.p&CIV_P_C)?p[2]:p[3];
    i->instruction_count++;return 1;
}

static int civ_aot_template_01E(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x69u,3u,p[0]);
    if(!civ_adc16_binary(i,p[1],NULL))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_01F(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xC9u,2u,p[0]);
    civ_cmp8(i,(uint8_t)i->cpu.a,p[1]);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_020(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA5u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_021(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x85u,2u,p[0]);
    if(!civ_bus_write8(i,(uint16_t)(i->cpu.d+p[1]),(uint8_t)i->cpu.a))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_022(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Fu,4u,p[0]);
    if(!civ_bus_write8(i,(p[0]+i->cpu.x)&0xFFFFFFu,(uint8_t)i->cpu.a))return 0;
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_023(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x6Bu,1u,0x000000u);
    { uint8_t lo,hi,bank; uint16_t target; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi)||!civ_pull8(i,&bank))return 0;
      target=(uint16_t)((uint16_t)(lo|((uint16_t)hi<<8))+1u);
      if(!civ_generated_return_allowed(p[0],((uint32_t)bank<<16)|target))return civ_fail_frontier(i,"RTL returned outside the finite Civilization proof set.",NULL);
      i->cpu.pbr=bank;i->cpu.pc=target; }
    i->instruction_count++;return 1;
}

static int civ_aot_template_024(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Cu,3u,p[0]);
    if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),0u))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_025(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xC8u,1u,0x000000u);
    i->cpu.y=(uint16_t)(i->cpu.y+1u);
    civ_set_nz16(i,i->cpu.y);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_026(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xABu,1u,0x000000u);
    { uint8_t v; if(!civ_pull8(i,&v))return 0; i->cpu.dbr=v; civ_set_nz8(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_027(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA6u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; i->cpu.x=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_028(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,p[0],2u,p[1]);
    i->cpu.pc=(i->cpu.p&CIV_P_N)?p[2]:p[3];
    i->instruction_count++;return 1;
}

static int civ_aot_template_029(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x38u,1u,0x000000u);
    i->cpu.p |= CIV_P_C;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_02A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Fu,4u,p[0]);
    if(!civ_bus_write16(i,(p[0]+i->cpu.x)&0xFFFFFFu,i->cpu.a))return 0;
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_02B(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x4Au,1u,0x000000u);
    { uint16_t v=i->cpu.a; if(v&0x0001u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v>>1); i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_02C(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFAu,1u,0x000000u);
    { uint8_t lo,hi; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi))return 0; i->cpu.x=(uint16_t)(lo|((uint16_t)hi<<8)); civ_set_nz16(i,i->cpu.x); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_02D(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xDAu,1u,0x000000u);
    if(!civ_push8(i,(uint8_t)(i->cpu.x>>8)))return 0;
    if(!civ_push8(i,(uint8_t)i->cpu.x))return 0;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_02E(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x3Au,1u,0x000000u);
    i->cpu.a=(uint16_t)(i->cpu.a-1u);
    civ_set_nz16(i,i->cpu.a);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_02F(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x29u,2u,p[0]);
    i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&p[1]));
    civ_set_nz8(i,(uint8_t)i->cpu.a);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_030(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x88u,1u,0x000000u);
    i->cpu.y=(uint16_t)(i->cpu.y-1u);
    civ_set_nz16(i,i->cpu.y);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_031(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x89u,3u,p[0]);
    if((i->cpu.a&p[1])==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_032(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x8Bu,1u,0x000000u);
    if(!civ_push8(i,i->cpu.dbr))return 0;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_033(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x54u,3u,p[0]);
    return civ_block_move_step(i,p[1],p[2],1,NULL);
}

static int civ_aot_template_034(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x3Au,1u,0x000000u);
    i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|(uint8_t)((uint8_t)i->cpu.a-1u));
    civ_set_nz8(i,(uint8_t)i->cpu.a);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_035(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xEBu,1u,0x000000u);
    i->cpu.a=(uint16_t)((i->cpu.a<<8)|(i->cpu.a>>8));
    civ_set_nz8(i,(uint8_t)i->cpu.a);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_036(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x6Du,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; if(!civ_adc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_037(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xEEu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; v=(uint16_t)(v+1u); if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_038(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xAFu,4u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,p[0],&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_039(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x8Fu,4u,p[0]);
    if(!civ_bus_write16(i,p[0],i->cpu.a))return 0;
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_03A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Du,3u,p[0]);
    if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),i->cpu.a))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_03B(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x8Fu,4u,p[0]);
    if(!civ_bus_write8(i,p[0],(uint8_t)i->cpu.a))return 0;
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_03C(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE0u,3u,p[0]);
    civ_cmp16(i,i->cpu.x,p[1]);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_03D(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x68u,1u,0x000000u);
    { uint8_t lo,hi; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi))return 0; i->cpu.a=(uint16_t)(lo|((uint16_t)hi<<8)); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_03E(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x86u,2u,p[0]);
    if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]),i->cpu.x))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_03F(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x1Au,1u,0x000000u);
    i->cpu.a=(uint16_t)(i->cpu.a+1u);
    civ_set_nz16(i,i->cpu.a);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_040(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x8Au,1u,0x000000u);
    i->cpu.a=i->cpu.x;
    civ_set_nz16(i,i->cpu.a);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_041(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x48u,1u,0x000000u);
    if(!civ_push8(i,(uint8_t)(i->cpu.a>>8)))return 0;
    if(!civ_push8(i,(uint8_t)i->cpu.a))return 0;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_042(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x7Au,1u,0x000000u);
    { uint8_t lo,hi; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi))return 0; i->cpu.y=(uint16_t)(lo|((uint16_t)hi<<8)); civ_set_nz16(i,i->cpu.y); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_043(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x5Au,1u,0x000000u);
    if(!civ_push8(i,(uint8_t)(i->cpu.y>>8)))return 0;
    if(!civ_push8(i,(uint8_t)i->cpu.y))return 0;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_044(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA5u,2u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_045(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xCDu,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_046(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x89u,2u,p[0]);
    if(((uint8_t)i->cpu.a&p[1])==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_047(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x1Au,1u,0x000000u);
    i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|(uint8_t)((uint8_t)i->cpu.a+1u));
    civ_set_nz8(i,(uint8_t)i->cpu.a);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_048(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xCDu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; civ_cmp16(i,i->cpu.a,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_049(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xCAu,1u,0x000000u);
    i->cpu.x=(uint16_t)(i->cpu.x-1u);
    civ_set_nz16(i,i->cpu.x);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_04A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA8u,1u,0x000000u);
    i->cpu.y=i->cpu.a;
    civ_set_nz16(i,i->cpu.y);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_04B(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xBDu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_04C(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x4Au,1u,0x000000u);
    { uint8_t v=(uint8_t)i->cpu.a; if(v&0x01u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)(v>>1); i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_04D(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE9u,3u,p[0]);
    if(!civ_sbc16_binary(i,p[1],NULL))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_04E(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x69u,2u,p[0]);
    if(!civ_adc8_binary(i,p[1],NULL))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_04F(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x0Au,1u,0x000000u);
    { uint8_t v=(uint8_t)i->cpu.a; if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)(v<<1); i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_050(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x0Cu,3u,p[0]);
    { uint16_t v,a=i->cpu.a; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; if((uint16_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint16_t)(v|a); if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),v))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_051(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x65u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; if(!civ_adc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_052(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xEEu,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; v=(uint8_t)(v+1u); if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),v))return 0; civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_053(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE6u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; v=(uint16_t)(v+1u); if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_054(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xAFu,4u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,p[0],&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_055(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xEDu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; if(!civ_sbc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_056(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xC0u,3u,p[0]);
    { uint16_t y=i->cpu.y,v=p[1],r=(uint16_t)(y-v); if(y>=v)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; civ_set_nz16(i,r); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_057(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xCEu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; v=(uint16_t)(v-1u); if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_058(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x09u,2u,p[0]);
    i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|p[1]));
    civ_set_nz8(i,(uint8_t)i->cpu.a);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_059(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x64u,2u,p[0]);
    if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]),0u))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_05A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x48u,1u,0x000000u);
    if(!civ_push8(i,(uint8_t)i->cpu.a))return 0;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_05B(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x6Du,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; if(!civ_adc8_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_05C(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xACu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; i->cpu.y=v; civ_set_nz16(i,i->cpu.y); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_05D(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xBDu,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_05E(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Du,3u,p[0]);
    if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),(uint8_t)i->cpu.a))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_05F(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE9u,2u,p[0]);
    if(!civ_sbc8_binary(i,p[1],NULL))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_060(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xB7u,2u,p[0]);
    { uint8_t p0,p1,p2; uint16_t v,b=(uint16_t)(i->cpu.d+p[1]); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read16(i,a,&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_061(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xC6u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; v=(uint16_t)(v-1u); if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_062(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xC5u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; civ_cmp16(i,i->cpu.a,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_063(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x68u,1u,0x000000u);
    { uint8_t v; if(!civ_pull8(i,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_064(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x98u,1u,0x000000u);
    i->cpu.a=i->cpu.y;
    civ_set_nz16(i,i->cpu.a);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_065(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xCEu,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; v=(uint8_t)(v-1u); if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),v))return 0; civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_066(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x09u,3u,p[0]);
    i->cpu.a=(uint16_t)(i->cpu.a|p[1]);
    civ_set_nz16(i,i->cpu.a);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_067(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x0Cu,3u,p[0]);
    { uint8_t v,a=(uint8_t)i->cpu.a; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; if((uint8_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint8_t)(v|a); if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),v))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_068(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Eu,3u,p[0]);
    if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),0u))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_069(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xB9u,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.y)),&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_06A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xCCu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; civ_cmp16(i,i->cpu.y,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_06B(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x99u,3u,p[0]);
    if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.y)),i->cpu.a))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_06C(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x0Fu,4u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,p[0],&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_06D(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xB7u,2u,p[0]);
    { uint8_t p0,p1,p2,v; uint16_t b=(uint16_t)(i->cpu.d+p[1]); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_06E(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x65u,2u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; if(!civ_adc8_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_06F(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x99u,3u,p[0]);
    if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.y)),(uint8_t)i->cpu.a))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_070(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x97u,2u,p[0]);
    { uint8_t p0,p1,p2; uint16_t b=(uint16_t)(i->cpu.d+p[1]); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_write16(i,a,i->cpu.a))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_071(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x49u,2u,p[0]);
    i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a^p[1]));
    civ_set_nz8(i,(uint8_t)i->cpu.a);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_072(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xC5u,2u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_073(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE5u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; if(!civ_sbc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_074(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xEDu,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; if(!civ_sbc8_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_075(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x49u,3u,p[0]);
    i->cpu.a=(uint16_t)(i->cpu.a^p[1]);
    civ_set_nz16(i,i->cpu.a);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_076(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xB9u,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.y)),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_077(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x05u,2u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_078(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA4u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; i->cpu.y=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_079(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x1Cu,3u,p[0]);
    { uint8_t v,a=(uint8_t)i->cpu.a; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; if((uint8_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint8_t)(v&~a); if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),v))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_07A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA3u,2u,p[0]);
    { uint16_t v,a=(uint16_t)(i->cpu.s+p[1]); if(!civ_bus_read16(i,a,&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_07B(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x5Cu,4u,p[0]);
    i->cpu.pbr=p[1];i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_07C(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x8Cu,3u,p[0]);
    if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),i->cpu.y))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_07D(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Bu,1u,0x000000u);
    i->cpu.y=i->cpu.x;
    civ_set_nz16(i,i->cpu.y);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_07E(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x1Cu,3u,p[0]);
    { uint16_t v,a=i->cpu.a; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; if((uint16_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint16_t)(v&~a); if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),v))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_07F(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Eu,3u,p[0]);
    if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),0u))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_080(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x7Du,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),&v))return 0; if(!civ_adc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_081(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x7Fu,4u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(p[0]+i->cpu.x)&0xFFFFFFu,&v))return 0; if(!civ_adc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_082(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x63u,2u,p[0]);
    { uint16_t v,a=(uint16_t)(i->cpu.s+p[1]); if(!civ_bus_read16(i,a,&v))return 0; if(!civ_adc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_083(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x64u,2u,p[0]);
    if(!civ_bus_write8(i,(uint16_t)(i->cpu.d+p[1]),0u))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_084(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x2Fu,4u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,p[0],&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_085(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA3u,2u,p[0]);
    { uint8_t v; uint16_t a=(uint16_t)(i->cpu.s+p[1]); if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_086(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x24u,2u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; if(((uint8_t)i->cpu.a&v)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; i->cpu.p=(uint8_t)((i->cpu.p&~(CIV_P_N|CIV_P_V))|(v&(CIV_P_N|CIV_P_V))); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_087(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xBBu,1u,0x000000u);
    i->cpu.x=i->cpu.y;
    civ_set_nz16(i,i->cpu.x);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_088(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE6u,2u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; v=(uint8_t)(v+1u); if(!civ_bus_write8(i,(uint16_t)(i->cpu.d+p[1]),v))return 0; civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_089(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xC6u,2u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; v=(uint8_t)(v-1u); if(!civ_bus_write8(i,(uint16_t)(i->cpu.d+p[1]),v))return 0; civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_08A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x95u,2u,p[0]);
    if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]+i->cpu.x),i->cpu.a))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_08B(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xECu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; civ_cmp16(i,i->cpu.x,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_08C(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xDFu,4u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(p[0]+i->cpu.x)&0xFFFFFFu,&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_08D(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x97u,2u,p[0]);
    { uint8_t p0,p1,p2; uint16_t b=(uint16_t)(i->cpu.d+p[1]); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_write8(i,a,(uint8_t)i->cpu.a))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_08E(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x63u,2u,p[0]);
    { uint8_t v; uint16_t a=(uint16_t)(i->cpu.s+p[1]); if(!civ_bus_read8(i,a,&v))return 0; if(!civ_adc8_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_08F(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xB5u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]+i->cpu.x),&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_090(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA7u,2u,p[0]);
    { uint8_t p0,p1,p2; uint16_t v,b=(uint16_t)(i->cpu.d+p[1]); if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; if(!civ_bus_read16(i,((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)),&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_091(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x8Au,1u,0x000000u);
    i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|(uint8_t)i->cpu.x);
    civ_set_nz8(i,(uint8_t)i->cpu.a);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_092(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x05u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_093(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x83u,2u,p[0]);
    if(!civ_bus_write8(i,(uint16_t)(i->cpu.s+p[1]),(uint8_t)i->cpu.a))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_094(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x04u,2u,p[0]);
    { uint16_t v,a=i->cpu.a; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; if((uint16_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint16_t)(v|a); if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]),v))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_095(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x0Du,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_096(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x2Du,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_097(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE5u,2u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; if(!civ_sbc8_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_098(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x2Du,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_099(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x2Cu,3u,0x000677u);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|0x0677u),&v))return 0; if((i->cpu.a&v)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; i->cpu.p=(uint8_t)((i->cpu.p&~(CIV_P_N|CIV_P_V))|((v&0x8000u)?CIV_P_N:0u)|((v&0x4000u)?CIV_P_V:0u)); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_09A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x0Eu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; if(v&0x8000u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v<<1); if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_09B(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xDFu,4u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(p[0]+i->cpu.x)&0xFFFFFFu,&v))return 0; civ_cmp16(i,i->cpu.a,v); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_09C(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFFu,4u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(p[0]+i->cpu.x)&0xFFFFFFu,&v))return 0; if(!civ_sbc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_09D(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x6Fu,4u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,p[0],&v))return 0; if(!civ_adc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_09E(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x6Fu,4u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,p[0],&v))return 0; if(!civ_adc8_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_09F(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x78u,1u,0x000000u);
    i->cpu.p |= CIV_P_I;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0A0(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x84u,2u,p[0]);
    if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]),i->cpu.y))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0A1(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x91u,2u,p[0]);
    { uint16_t ptr; uint32_t a; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&ptr))return 0; a=(((uint32_t)i->cpu.dbr<<16)|((uint16_t)(ptr+i->cpu.y))); if(!civ_bus_write16(i,a,i->cpu.a))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0A2(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x06u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; if(v&0x8000u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v<<1); if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0A3(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA7u,2u,p[0]);
    { uint8_t p0,p1,p2,v; uint16_t b=(uint16_t)(i->cpu.d+p[1]); if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; if(!civ_bus_read8(i,((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0A4(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x87u,2u,0x000007u);
    { uint8_t p0,p1,p2; uint16_t b=(uint16_t)(i->cpu.d+0x07u); if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; if(!civ_bus_write16(i,((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)),i->cpu.a))return 0; }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0A5(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x0Eu,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)(v<<1); if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),v))return 0; civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0A6(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xC3u,2u,p[0]);
    { uint8_t v; uint16_t a=(uint16_t)(i->cpu.s+p[1]); if(!civ_bus_read8(i,a,&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0A7(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xCFu,4u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,p[0],&v))return 0; civ_cmp16(i,i->cpu.a,v); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0A8(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x4Du,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a^v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0A9(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFFu,4u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(p[0]+i->cpu.x)&0xFFFFFFu,&v))return 0; if(!civ_sbc8_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0AA(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,p[0]);
    if(!civ_push8(i,p[1]))return 0;
    if(!civ_push8(i,p[2]))return 0;
    if(i->cpu.x==0x0000u)i->cpu.pc=p[3];
    else if(i->cpu.x==0x0002u)i->cpu.pc=p[4];
    else if(i->cpu.x==0x0004u)i->cpu.pc=p[5];
    else if(i->cpu.x==0x0006u)i->cpu.pc=p[6];
    else if(i->cpu.x==0x0008u)i->cpu.pc=p[7];
    else if(i->cpu.x==0x000Au)i->cpu.pc=p[8];
    else if(i->cpu.x==0x000Cu)i->cpu.pc=p[9];
    else if(i->cpu.x==0x000Eu)i->cpu.pc=p[10];
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0AB(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x83u,2u,p[0]);
    if(!civ_bus_write16(i,(uint16_t)(i->cpu.s+p[1]),i->cpu.a))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0AC(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x44u,3u,p[0]);
    return civ_block_move_step(i,p[1],p[1],-1,NULL);
}

static int civ_aot_template_0AD(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x6Au,1u,0x000000u);
    { uint16_t v=i->cpu.a; uint16_t cin=(uint16_t)((i->cpu.p&CIV_P_C)?0x8000u:0u); if(v&0x0001u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)((v>>1)|cin); i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0AE(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x17u,2u,p[0]);
    { uint8_t p0,p1,p2,v; uint16_t b=(uint16_t)(i->cpu.d+p[1]); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0AF(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x25u,2u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0B0(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,p[0]);
    if(!civ_push8(i,p[1]))return 0;
    if(!civ_push8(i,p[2]))return 0;
    if(i->cpu.x==p[3])i->cpu.pc=p[4];
    else if(i->cpu.x==p[5])i->cpu.pc=p[6];
    else if(i->cpu.x==p[7])i->cpu.pc=p[8];
    else if(i->cpu.x==p[9])i->cpu.pc=p[10];
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0B1(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xC3u,2u,p[0]);
    { uint16_t v,a=(uint16_t)(i->cpu.s+p[1]); if(!civ_bus_read16(i,a,&v))return 0; civ_cmp16(i,i->cpu.a,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0B2(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x7Fu,4u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(p[0]+i->cpu.x)&0xFFFFFFu,&v))return 0; if(!civ_adc8_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0B3(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFEu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),&v))return 0; v=(uint16_t)(v+1u); if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0B4(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x17u,2u,0x000007u);
    { uint8_t p0,p1,p2; uint16_t v,b=(uint16_t)(i->cpu.d+0x07u); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read16(i,a,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0B5(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFEu,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),&v))return 0; v=(uint8_t)(v+1u); if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),v))return 0; civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0B6(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xCFu,4u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,p[0],&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0B7(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x0Du,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0B8(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE3u,2u,p[0]);
    { uint8_t v; uint16_t a=(uint16_t)(i->cpu.s+p[1]); if(!civ_bus_read8(i,a,&v))return 0; if(!civ_sbc8_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0B9(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x91u,2u,p[0]);
    { uint16_t ptr; uint32_t a; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&ptr))return 0; a=(((uint32_t)i->cpu.dbr<<16)|((uint16_t)(ptr+i->cpu.y))); if(!civ_bus_write8(i,a,(uint8_t)i->cpu.a))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0BA(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xDEu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),&v))return 0; v=(uint16_t)(v-1u); if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0BB(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x3Fu,4u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(p[0]+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0BC(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x25u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0BD(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x1Fu,4u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(p[0]+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0BE(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x06u,2u,0x000007u);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+0x07u),&v))return 0; if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)(v<<1); if(!civ_bus_write8(i,(uint16_t)(i->cpu.d+0x07u),v))return 0; civ_set_nz8(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0BF(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Au,1u,0x000000u);
    i->cpu.s=i->cpu.e?(uint16_t)(0x0100u|(i->cpu.x&0x00FFu)):i->cpu.x;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0C0(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x0Bu,1u,0x000000u);
    if(!civ_push8(i,(uint8_t)(i->cpu.d>>8)))return 0;
    if(!civ_push8(i,(uint8_t)i->cpu.d))return 0;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0C1(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,p[0]);
    if(!civ_push8(i,p[1]))return 0;
    if(!civ_push8(i,p[2]))return 0;
    if(i->cpu.x==0x0000u)i->cpu.pc=p[3];
    else if(i->cpu.x==p[4])i->cpu.pc=p[5];
    else if(i->cpu.x==p[6])i->cpu.pc=p[7];
    else if(i->cpu.x==p[8])i->cpu.pc=p[9];
    else if(i->cpu.x==p[10])i->cpu.pc=p[11];
    else if(i->cpu.x==p[12])i->cpu.pc=p[13];
    else if(i->cpu.x==p[14])i->cpu.pc=p[15];
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0C2(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,p[0]);
    if(!civ_push8(i,p[1]))return 0;
    if(!civ_push8(i,p[2]))return 0;
    if(i->cpu.x==p[3])i->cpu.pc=p[4];
    else if(i->cpu.x==p[5])i->cpu.pc=p[6];
    else if(i->cpu.x==p[7])i->cpu.pc=p[8];
    else if(i->cpu.x==p[9])i->cpu.pc=p[10];
    else if(i->cpu.x==p[11])i->cpu.pc=p[12];
    else if(i->cpu.x==p[13])i->cpu.pc=p[14];
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0C3(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xDDu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),&v))return 0; civ_cmp16(i,i->cpu.a,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0C4(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE4u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; civ_cmp16(i,i->cpu.x,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0C5(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x2Bu,1u,0x000000u);
    { uint8_t lo,hi; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi))return 0; i->cpu.d=(uint16_t)(lo|((uint16_t)hi<<8)); civ_set_nz16(i,i->cpu.d); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0C6(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x77u,2u,p[0]);
    { uint8_t p0,p1,p2; uint16_t b=(uint16_t)(i->cpu.d+p[1]),v; uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read16(i,a,&v))return 0; if(!civ_adc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0C7(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x07u,2u,0x000007u);
    { uint8_t p0,p1,p2; uint16_t v,b=(uint16_t)(i->cpu.d+0x07u); if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; if(!civ_bus_read16(i,((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)),&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0C8(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x3Eu,3u,p[0]);
    { uint8_t v,cin=(uint8_t)((i->cpu.p&CIV_P_C)!=0u); if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),&v))return 0; if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)((v<<1)|cin); if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),v))return 0; civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0C9(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xEFu,4u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,p[0],&v))return 0; if(!civ_sbc8_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0CA(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x95u,2u,p[0]);
    if(!civ_bus_write8(i,(uint16_t)(i->cpu.d+p[1]+i->cpu.x),(uint8_t)i->cpu.a))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0CB(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,p[0]);
    if(!civ_push8(i,p[1]))return 0;
    if(!civ_push8(i,p[2]))return 0;
    if(i->cpu.x==p[3])i->cpu.pc=p[4];
    else if(i->cpu.x==p[5])i->cpu.pc=p[6];
    else if(i->cpu.x==p[7])i->cpu.pc=p[8];
    else if(i->cpu.x==p[9])i->cpu.pc=p[10];
    else if(i->cpu.x==p[11])i->cpu.pc=p[12];
    else if(i->cpu.x==p[13])i->cpu.pc=p[14];
    else if(i->cpu.x==p[15])i->cpu.pc=p[16];
    else if(i->cpu.x==p[17])i->cpu.pc=p[18];
    else if(i->cpu.x==p[19])i->cpu.pc=p[20];
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0CC(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,p[0]);
    if(!civ_push8(i,p[1]))return 0;
    if(!civ_push8(i,p[2]))return 0;
    if(i->cpu.x==p[3])i->cpu.pc=p[4];
    else if(i->cpu.x==p[5])i->cpu.pc=p[6];
    else if(i->cpu.x==p[7])i->cpu.pc=p[8];
    else if(i->cpu.x==p[9])i->cpu.pc=p[10];
    else if(i->cpu.x==p[11])i->cpu.pc=p[12];
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0CD(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x40u,1u,0x000000u);
    if(!civ_rti_native(i))return 0;
    i->instruction_count++;return 1;
}

static int civ_aot_template_0CE(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x92u,2u,p[0]);
    { uint16_t ptr; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&ptr))return 0; if(!civ_bus_write16(i,((uint32_t)i->cpu.dbr<<16)|ptr,i->cpu.a))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0CF(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x4Du,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a^v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0D0(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x46u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; if(v&0x0001u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v>>1); if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0D1(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xEFu,4u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,p[0],&v))return 0; if(!civ_sbc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0D2(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xBAu,1u,0x000000u);
    i->cpu.x=i->cpu.s;
    civ_set_nz16(i,i->cpu.x);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0D3(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,p[0]);
    if(!civ_push8(i,0x01u))return 0;
    if(!civ_push8(i,p[1]))return 0;
    if(i->cpu.x==p[2])i->cpu.pc=p[3];
    else if(i->cpu.x==p[4])i->cpu.pc=p[5];
    else if(i->cpu.x==p[6])i->cpu.pc=p[7];
    else if(i->cpu.x==p[8])i->cpu.pc=p[9];
    else if(i->cpu.x==p[10])i->cpu.pc=p[11];
    else if(i->cpu.x==p[12])i->cpu.pc=p[13];
    else if(i->cpu.x==p[14])i->cpu.pc=p[15];
    else if(i->cpu.x==p[16])i->cpu.pc=p[17];
    else if(i->cpu.x==p[18])i->cpu.pc=p[19];
    else if(i->cpu.x==p[20])i->cpu.pc=p[21];
    else if(i->cpu.x==p[22])i->cpu.pc=p[23];
    else if(i->cpu.x==p[24])i->cpu.pc=p[25];
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0D4(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x2Eu,3u,0x001932u);
    { uint16_t v,cin=(uint16_t)((i->cpu.p&CIV_P_C)!=0u); if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|0x1932u),&v))return 0; if(v&0x8000u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)((v<<1)|cin); if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|0x1932u),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0D5(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x1Fu,4u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(p[0]+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0D6(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xF6u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]+i->cpu.x),&v))return 0; v=(uint16_t)(v+1u); if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]+i->cpu.x),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0D7(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xD6u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]+i->cpu.x),&v))return 0; v=(uint16_t)(v-1u); if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]+i->cpu.x),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0D8(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x4Eu,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; if(v&0x01u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)(v>>1); if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),v))return 0; civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0D9(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x3Fu,4u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(p[0]+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0DA(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xB5u,2u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+p[1]+i->cpu.x),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0DB(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x2Fu,4u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,p[0],&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0DC(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x0Fu,4u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,p[0],&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[1];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0DD(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x2Au,1u,0x000000u);
    { uint8_t v=(uint8_t)i->cpu.a; uint8_t cin=(uint8_t)((i->cpu.p&CIV_P_C)!=0u); if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)((v<<1)|cin); i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0DE(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x98u,1u,0x000000u);
    i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|(uint8_t)i->cpu.y);
    civ_set_nz8(i,(uint8_t)i->cpu.a);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0DF(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x1Bu,1u,0x000000u);
    i->cpu.s=i->cpu.e?(uint16_t)(0x0100u|(i->cpu.a&0x00FFu)):i->cpu.a;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0E0(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xCAu,1u,0x000000u);
    i->cpu.x=(uint8_t)((uint8_t)i->cpu.x-1u);
    civ_set_nz8(i,(uint8_t)i->cpu.x);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0E1(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xAAu,1u,0x000000u);
    i->cpu.x=(uint8_t)i->cpu.a;
    civ_set_nz8(i,(uint8_t)i->cpu.x);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0E2(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x74u,2u,p[0]);
    if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]+i->cpu.x),0u))return 0;
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0E3(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,p[0]);
    if(!civ_push8(i,p[1]))return 0;
    if(!civ_push8(i,p[2]))return 0;
    if(i->cpu.x==p[3])i->cpu.pc=p[4];
    else if(i->cpu.x==p[5])i->cpu.pc=p[6];
    else if(i->cpu.x==p[7])i->cpu.pc=p[8];
    else if(i->cpu.x==p[9])i->cpu.pc=p[10];
    else if(i->cpu.x==p[11])i->cpu.pc=p[12];
    else if(i->cpu.x==p[13])i->cpu.pc=p[14];
    else if(i->cpu.x==p[15])i->cpu.pc=p[16];
    else if(i->cpu.x==p[17])i->cpu.pc=p[18];
    else if(i->cpu.x==p[19])i->cpu.pc=p[20];
    else if(i->cpu.x==p[21])i->cpu.pc=p[22];
    else if(i->cpu.x==p[23])i->cpu.pc=p[24];
    else if(i->cpu.x==p[25])i->cpu.pc=p[26];
    else if(i->cpu.x==p[27])i->cpu.pc=p[28];
    else if(i->cpu.x==p[29])i->cpu.pc=p[30];
    else if(i->cpu.x==p[31])i->cpu.pc=p[32];
    else if(i->cpu.x==p[33])i->cpu.pc=p[34];
    else if(i->cpu.x==p[35])i->cpu.pc=p[36];
    else if(i->cpu.x==p[37])i->cpu.pc=p[38];
    else if(i->cpu.x==p[39])i->cpu.pc=p[40];
    else if(i->cpu.x==p[41])i->cpu.pc=p[42];
    else if(i->cpu.x==p[43])i->cpu.pc=p[44];
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0E4(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xB2u,2u,0x0000F8u);
    { uint16_t ptr,v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+0xF8u),&ptr))return 0; if(!civ_bus_read16(i,((uint32_t)i->cpu.dbr<<16)|ptr,&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0E5(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x26u,2u,p[0]);
    { uint16_t v,cin=(uint16_t)((i->cpu.p&CIV_P_C)!=0u); if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; if(v&0x8000u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)((v<<1)|cin); if(!civ_bus_write16(i,(uint16_t)(i->cpu.d+p[1]),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0E6(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x45u,2u,0x000027u);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+0x27u),&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a^v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0E7(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x04u,2u,p[0]);
    { uint8_t v,a=(uint8_t)i->cpu.a; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; if((uint8_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint8_t)(v|a); if(!civ_bus_write8(i,(uint16_t)(i->cpu.d+p[1]),v))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0E8(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x2Eu,3u,p[0]);
    { uint8_t v,cin=(uint8_t)((i->cpu.p&CIV_P_C)!=0u); if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),&v))return 0; if(v&0x80u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint8_t)((v<<1)|cin); if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|p[1]),v))return 0; civ_set_nz8(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0E9(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xDDu,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),&v))return 0; civ_cmp8(i,(uint8_t)i->cpu.a,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0EA(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x3Bu,1u,0x000000u);
    i->cpu.a=i->cpu.s;
    civ_set_nz16(i,i->cpu.a);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0EB(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x5Bu,1u,0x000000u);
    i->cpu.d=i->cpu.a;
    civ_set_nz16(i,i->cpu.d);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0EC(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x58u,1u,0x000000u);
    i->cpu.p &= (uint8_t)~CIV_P_I;
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0ED(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x70u,2u,p[0]);
    i->cpu.pc=(i->cpu.p&CIV_P_V)?p[1]:p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0EE(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE8u,1u,0x000000u);
    i->cpu.x=(uint8_t)((uint8_t)i->cpu.x+1u);
    civ_set_nz8(i,(uint8_t)i->cpu.x);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0EF(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA0u,2u,p[0]);
    i->cpu.y=(uint16_t)p[1];
    civ_set_nz8(i,(uint8_t)i->cpu.y);
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0F0(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x88u,1u,0x000000u);
    i->cpu.y=(uint8_t)((uint8_t)i->cpu.y-1u);
    civ_set_nz8(i,(uint8_t)i->cpu.y);
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0F1(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,0x0054C5u);
    if(!civ_push8(i,p[0]))return 0;
    if(!civ_push8(i,p[1]))return 0;
    if(i->cpu.x==0x0000u)i->cpu.pc=0x550Bu;
    else if(i->cpu.x==0x0002u)i->cpu.pc=0x5525u;
    else if(i->cpu.x==0x0004u)i->cpu.pc=0x5603u;
    else if(i->cpu.x==0x0006u)i->cpu.pc=0x5525u;
    else if(i->cpu.x==0x0008u)i->cpu.pc=0x55B9u;
    else if(i->cpu.x==0x000Au)i->cpu.pc=0x5525u;
    else if(i->cpu.x==0x000Cu)i->cpu.pc=0x5632u;
    else if(i->cpu.x==0x000Eu)i->cpu.pc=0x55B9u;
    else if(i->cpu.x==0x0010u)i->cpu.pc=0x5723u;
    else if(i->cpu.x==0x0012u)i->cpu.pc=0x5723u;
    else if(i->cpu.x==0x0014u)i->cpu.pc=0x5632u;
    else if(i->cpu.x==0x0016u)i->cpu.pc=0x5603u;
    else if(i->cpu.x==0x0018u)i->cpu.pc=0x550Bu;
    else if(i->cpu.x==0x001Au)i->cpu.pc=0x553Fu;
    else if(i->cpu.x==0x001Cu)i->cpu.pc=0x5562u;
    else if(i->cpu.x==0x001Eu)i->cpu.pc=0x5589u;
    else if(i->cpu.x==0x0020u)i->cpu.pc=0x5723u;
    else if(i->cpu.x==0x0022u)i->cpu.pc=0x553Fu;
    else if(i->cpu.x==0x0024u)i->cpu.pc=0x5603u;
    else if(i->cpu.x==0x0026u)i->cpu.pc=0x5603u;
    else if(i->cpu.x==0x0028u)i->cpu.pc=0x5571u;
    else if(i->cpu.x==0x002Au)i->cpu.pc=0x5603u;
    else if(i->cpu.x==0x002Cu)i->cpu.pc=0x5603u;
    else if(i->cpu.x==0x002Eu)i->cpu.pc=0x5603u;
    else if(i->cpu.x==0x0030u)i->cpu.pc=0x5653u;
    else if(i->cpu.x==0x0032u)i->cpu.pc=0x5603u;
    else if(i->cpu.x==0x0034u)i->cpu.pc=0x55A1u;
    else if(i->cpu.x==0x0036u)i->cpu.pc=0x5603u;
    else if(i->cpu.x==0x0038u)i->cpu.pc=0x550Bu;
    else if(i->cpu.x==0x003Au)i->cpu.pc=0x566Au;
    else if(i->cpu.x==0x003Cu)i->cpu.pc=0x5681u;
    else if(i->cpu.x==0x003Eu)i->cpu.pc=0x5698u;
    else if(i->cpu.x==0x0040u)i->cpu.pc=0x56AFu;
    else if(i->cpu.x==0x0042u)i->cpu.pc=0x56CAu;
    else if(i->cpu.x==0x0044u)i->cpu.pc=0x56E1u;
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0F2(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,p[0]);
    if(!civ_push8(i,p[1]))return 0;
    if(!civ_push8(i,p[2]))return 0;
    if(i->cpu.x==0x0000u)i->cpu.pc=p[3];
    else if(i->cpu.x==0x0002u)i->cpu.pc=p[4];
    else if(i->cpu.x==0x0004u)i->cpu.pc=p[4];
    else if(i->cpu.x==0x0006u)i->cpu.pc=p[4];
    else if(i->cpu.x==0x0008u)i->cpu.pc=p[4];
    else if(i->cpu.x==0x000Au)i->cpu.pc=p[4];
    else if(i->cpu.x==0x000Cu)i->cpu.pc=p[4];
    else if(i->cpu.x==0x000Eu)i->cpu.pc=p[4];
    else if(i->cpu.x==0x0010u)i->cpu.pc=p[4];
    else if(i->cpu.x==0x0012u)i->cpu.pc=p[4];
    else if(i->cpu.x==0x0014u)i->cpu.pc=p[4];
    else if(i->cpu.x==0x0016u)i->cpu.pc=p[4];
    else if(i->cpu.x==0x0018u)i->cpu.pc=p[4];
    else if(i->cpu.x==0x001Au)i->cpu.pc=p[4];
    else if(i->cpu.x==0x001Cu)i->cpu.pc=p[5];
    else if(i->cpu.x==0x001Eu)i->cpu.pc=p[6];
    else if(i->cpu.x==0x0020u)i->cpu.pc=p[7];
    else if(i->cpu.x==0x0022u)i->cpu.pc=p[7];
    else if(i->cpu.x==0x0024u)i->cpu.pc=p[7];
    else if(i->cpu.x==0x0026u)i->cpu.pc=p[8];
    else if(i->cpu.x==0x0028u)i->cpu.pc=p[8];
    else if(i->cpu.x==0x002Au)i->cpu.pc=p[8];
    else if(i->cpu.x==0x002Cu)i->cpu.pc=p[8];
    else if(i->cpu.x==0x002Eu)i->cpu.pc=p[7];
    else if(i->cpu.x==0x0030u)i->cpu.pc=p[7];
    else if(i->cpu.x==0x0032u)i->cpu.pc=p[9];
    else if(i->cpu.x==0x0034u)i->cpu.pc=p[10];
    else if(i->cpu.x==0x0036u)i->cpu.pc=p[11];
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0F3(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,p[0]);
    if(!civ_push8(i,p[1]))return 0;
    if(!civ_push8(i,p[2]))return 0;
    if(i->cpu.x==0x0000u)i->cpu.pc=p[3];
    else if(i->cpu.x==0x0002u)i->cpu.pc=p[4];
    else if(i->cpu.x==0x0004u)i->cpu.pc=p[5];
    else if(i->cpu.x==0x0006u)i->cpu.pc=p[6];
    else if(i->cpu.x==0x0008u)i->cpu.pc=p[7];
    else if(i->cpu.x==0x000Au)i->cpu.pc=p[8];
    else if(i->cpu.x==0x000Cu)i->cpu.pc=p[9];
    else if(i->cpu.x==0x000Eu)i->cpu.pc=p[10];
    else if(i->cpu.x==0x0010u)i->cpu.pc=p[11];
    else if(i->cpu.x==0x0012u)i->cpu.pc=p[12];
    else if(i->cpu.x==0x0014u)i->cpu.pc=p[13];
    else if(i->cpu.x==0x0016u)i->cpu.pc=p[14];
    else if(i->cpu.x==0x0018u)i->cpu.pc=p[15];
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0F4(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,p[0]);
    if(!civ_push8(i,p[1]))return 0;
    if(!civ_push8(i,p[2]))return 0;
    if(i->cpu.x==0x0000u)i->cpu.pc=p[3];
    else if(i->cpu.x==0x0002u)i->cpu.pc=p[4];
    else if(i->cpu.x==0x0004u)i->cpu.pc=p[5];
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0F5(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,p[0]);
    if(!civ_push8(i,p[1]))return 0;
    if(!civ_push8(i,p[2]))return 0;
    if(i->cpu.x==0x0000u)i->cpu.pc=p[3];
    else if(i->cpu.x==0x0002u)i->cpu.pc=p[4];
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_0F6(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xE3u,2u,0x000001u);
    { uint16_t v,a=(uint16_t)(i->cpu.s+0x01u); if(!civ_bus_read16(i,a,&v))return 0; if(!civ_sbc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0F7(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFDu,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),&v))return 0; if(!civ_sbc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0F8(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x39u,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.y)),&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0F9(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x19u,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.y)),&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a|v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0FA(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x79u,3u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.y)),&v))return 0; if(!civ_adc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0FB(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x4Eu,3u,0x00043Du);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|0x043Du),&v))return 0; if(v&0x0001u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v>>1); if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|0x043Du),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0FC(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x24u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]),&v))return 0; if((i->cpu.a&v)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; i->cpu.p=(uint8_t)((i->cpu.p&~(CIV_P_N|CIV_P_V))|((v&0x8000u)?CIV_P_N:0u)|((v&0x4000u)?CIV_P_V:0u)); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0FD(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xD5u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]+i->cpu.x),&v))return 0; civ_cmp16(i,i->cpu.a,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0FE(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x15u,2u,p[0]);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+p[1]+i->cpu.x),&v))return 0; v=(uint16_t)(i->cpu.a|v); i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_0FF(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFBu,1u,0x000000u);
    { uint8_t old_e=i->cpu.e; uint8_t old_c=(uint8_t)((i->cpu.p&CIV_P_C)!=0u);
      i->cpu.e=old_c; if(old_e)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C;
      if(i->cpu.e){i->cpu.p|=(CIV_P_M|CIV_P_X);i->cpu.x&=0x00FFu;i->cpu.y&=0x00FFu;i->cpu.s=(uint16_t)(0x0100u|(i->cpu.s&0x00FFu));} }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_100(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x5Du,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a^v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_101(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x1Du,3u,p[0]);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(p[1]+i->cpu.x)),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[2];
    i->instruction_count++;return 1;
}

static int civ_aot_template_102(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x19u,3u,0x000000u);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(0x0000u+i->cpu.y)),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_103(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA6u,2u,0x00004Bu);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+0x4Bu),&v))return 0; i->cpu.x=v; civ_set_nz8(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_104(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA4u,2u,0x00004Du);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+0x4Du),&v))return 0; i->cpu.y=v; civ_set_nz8(i,v); }
    i->cpu.pc=p[0];
    i->instruction_count++;return 1;
}

static int civ_aot_template_105(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA2u,2u,0x000001u);
    i->cpu.x=(uint16_t)0x01u;
    civ_set_nz8(i,(uint8_t)i->cpu.x);
    i->cpu.pc=0x138Du;
    i->instruction_count++;return 1;
}

static int civ_aot_template_106(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xBBu,1u,0x000000u);
    i->cpu.x=(uint8_t)i->cpu.y;
    civ_set_nz8(i,(uint8_t)i->cpu.x);
    i->cpu.pc=0x38E2u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_107(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xA8u,1u,0x000000u);
    i->cpu.y=(uint8_t)i->cpu.a;
    civ_set_nz8(i,(uint8_t)i->cpu.y);
    i->cpu.pc=0x38D6u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_108(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Bu,1u,0x000000u);
    i->cpu.y=(uint8_t)i->cpu.x;
    civ_set_nz8(i,(uint8_t)i->cpu.y);
    i->cpu.pc=0x2D91u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_109(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Du,3u,0x000000u);
    if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(0x0000u+i->cpu.x)),i->cpu.a))return 0;
    i->v16_wram_clear_bytes+=2u;
    i->cpu.pc=0x812Du;
    i->instruction_count++;return 1;
}

static int civ_aot_template_10A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x9Du,3u,0x000000u);
    if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(0x0000u+i->cpu.x)),(uint8_t)i->cpu.a))return 0;
    i->v16_wram_clear_bytes+=1u;
    i->cpu.pc=0x8117u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_10B(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x8Cu,3u,0x004203u);
    if(!civ_bus_write8(i,(((uint32_t)i->cpu.dbr<<16)|0x4203u),(uint8_t)i->cpu.y))return 0;
    i->cpu.pc=0x1D65u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_10C(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x84u,2u,0x000049u);
    if(!civ_bus_write8(i,(uint16_t)(i->cpu.d+0x49u),(uint8_t)i->cpu.y))return 0;
    i->cpu.pc=0x2D90u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_10D(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,0x00F05Du);
    if(!civ_push8(i,0xF0u))return 0;
    if(!civ_push8(i,0x5Au))return 0;
    if(i->cpu.x==0x0002u)i->cpu.pc=0xF08Fu;
    else if(i->cpu.x==0x0004u)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x0006u)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x0008u)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x000Au)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x000Cu)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x000Eu)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x0010u)i->cpu.pc=0xF0FFu;
    else if(i->cpu.x==0x0012u)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x0014u)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x0016u)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x0018u)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x001Au)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x001Cu)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x001Eu)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x0020u)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x0022u)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x0024u)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x0026u)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x0028u)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x002Au)i->cpu.pc=0xF122u;
    else if(i->cpu.x==0x002Cu)i->cpu.pc=0xF126u;
    else if(i->cpu.x==0x002Eu)i->cpu.pc=0xF126u;
    else if(i->cpu.x==0x0030u)i->cpu.pc=0xF126u;
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_10E(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,0x000AC5u);
    if(!civ_push8(i,0x0Au))return 0;
    if(!civ_push8(i,0xC2u))return 0;
    if(i->cpu.x==0x0000u)i->cpu.pc=0x0AF1u;
    else if(i->cpu.x==0x0002u)i->cpu.pc=0x0AF5u;
    else if(i->cpu.x==0x0004u)i->cpu.pc=0x0AF9u;
    else if(i->cpu.x==0x0006u)i->cpu.pc=0x0B03u;
    else if(i->cpu.x==0x0008u)i->cpu.pc=0x0B0Du;
    else if(i->cpu.x==0x000Au)i->cpu.pc=0x0B17u;
    else if(i->cpu.x==0x000Cu)i->cpu.pc=0x0B21u;
    else if(i->cpu.x==0x000Eu)i->cpu.pc=0x0B2Bu;
    else if(i->cpu.x==0x0010u)i->cpu.pc=0x0B35u;
    else if(i->cpu.x==0x0012u)i->cpu.pc=0x0B3Fu;
    else if(i->cpu.x==0x0014u)i->cpu.pc=0x0B49u;
    else if(i->cpu.x==0x0016u)i->cpu.pc=0x0B4Du;
    else if(i->cpu.x==0x0018u)i->cpu.pc=0x0B51u;
    else if(i->cpu.x==0x001Au)i->cpu.pc=0x0B99u;
    else if(i->cpu.x==0x001Cu)i->cpu.pc=0x0BAEu;
    else if(i->cpu.x==0x001Eu)i->cpu.pc=0x0BC3u;
    else if(i->cpu.x==0x0020u)i->cpu.pc=0x0BD8u;
    else if(i->cpu.x==0x0022u)i->cpu.pc=0x0BEDu;
    else if(i->cpu.x==0x0024u)i->cpu.pc=0x0C20u;
    else if(i->cpu.x==0x0026u)i->cpu.pc=0x0C35u;
    else if(i->cpu.x==0x0028u)i->cpu.pc=0x0C39u;
    else if(i->cpu.x==0x002Au)i->cpu.pc=0x0C3Du;
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_10F(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,0x00AD7Fu);
    if(!civ_push8(i,0xADu))return 0;
    if(!civ_push8(i,0x7Cu))return 0;
    if(i->cpu.x==0x0000u)i->cpu.pc=0xADA3u;
    else if(i->cpu.x==0x0002u)i->cpu.pc=0xADD3u;
    else if(i->cpu.x==0x0004u)i->cpu.pc=0xAE30u;
    else if(i->cpu.x==0x0006u)i->cpu.pc=0xAE60u;
    else if(i->cpu.x==0x0008u)i->cpu.pc=0xAE90u;
    else if(i->cpu.x==0x000Au)i->cpu.pc=0xAEC0u;
    else if(i->cpu.x==0x000Cu)i->cpu.pc=0xAEF0u;
    else if(i->cpu.x==0x000Eu)i->cpu.pc=0xAF20u;
    else if(i->cpu.x==0x0010u)i->cpu.pc=0xAF50u;
    else if(i->cpu.x==0x0012u)i->cpu.pc=0xAF80u;
    else if(i->cpu.x==0x0014u)i->cpu.pc=0xAFEEu;
    else if(i->cpu.x==0x0016u)i->cpu.pc=0xAFF5u;
    else if(i->cpu.x==0x0018u)i->cpu.pc=0xB025u;
    else if(i->cpu.x==0x001Au)i->cpu.pc=0xB055u;
    else if(i->cpu.x==0x001Cu)i->cpu.pc=0xB085u;
    else if(i->cpu.x==0x001Eu)i->cpu.pc=0xB0B5u;
    else if(i->cpu.x==0x0020u)i->cpu.pc=0xB0E5u;
    else if(i->cpu.x==0x0022u)i->cpu.pc=0xB115u;
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_110(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,0x000F1Au);
    if(!civ_push8(i,0x0Eu))return 0;
    if(!civ_push8(i,0xECu))return 0;
    if(i->cpu.x==0x0000u)i->cpu.pc=0x18DCu;
    else if(i->cpu.x==0x0002u)i->cpu.pc=0x1914u;
    else if(i->cpu.x==0x0004u)i->cpu.pc=0x0F3Au;
    else if(i->cpu.x==0x0006u)i->cpu.pc=0x108Du;
    else if(i->cpu.x==0x0008u)i->cpu.pc=0x0F7Eu;
    else if(i->cpu.x==0x000Au)i->cpu.pc=0x0F3Au;
    else if(i->cpu.x==0x000Cu)i->cpu.pc=0x108Du;
    else if(i->cpu.x==0x000Eu)i->cpu.pc=0x12F6u;
    else if(i->cpu.x==0x0010u)i->cpu.pc=0x11DBu;
    else if(i->cpu.x==0x0012u)i->cpu.pc=0x0F3Au;
    else if(i->cpu.x==0x0014u)i->cpu.pc=0x108Du;
    else if(i->cpu.x==0x0016u)i->cpu.pc=0x0F78u;
    else if(i->cpu.x==0x0018u)i->cpu.pc=0x11DBu;
    else if(i->cpu.x==0x001Au)i->cpu.pc=0x0F3Au;
    else if(i->cpu.x==0x001Cu)i->cpu.pc=0x108Du;
    else if(i->cpu.x==0x001Eu)i->cpu.pc=0x0FA2u;
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_111(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xFCu,3u,0x005FFBu);
    if(!civ_push8(i,0x5Fu))return 0;
    if(!civ_push8(i,0xF9u))return 0;
    if(i->cpu.x==0x0000u)i->cpu.pc=0x600Fu;
    else if(i->cpu.x==0x0002u)i->cpu.pc=0x604Fu;
    else if(i->cpu.x==0x0004u)i->cpu.pc=0x6268u;
    else if(i->cpu.x==0x0006u)i->cpu.pc=0x6291u;
    else if(i->cpu.x==0x0008u)i->cpu.pc=0x62F4u;
    else if(i->cpu.x==0x000Au)i->cpu.pc=0x630Au;
    else if(i->cpu.x==0x000Cu)i->cpu.pc=0x60C6u;
    else if(i->cpu.x==0x000Eu)i->cpu.pc=0x60FFu;
    else if(i->cpu.x==0x0010u)i->cpu.pc=0x6199u;
    else if(i->cpu.x==0x0012u)i->cpu.pc=0x6231u;
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_112(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x4Bu,1u,0x000000u);
    if(!civ_push8(i,i->cpu.pbr))return 0;
    i->cpu.pc=0x84DAu;
    i->instruction_count++;return 1;
}

static int civ_aot_template_113(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xD0u,2u,0x0000FBu);
    if(!i->natural_timing_enabled)civ_timing_advance_master(i,6u);
    i->cpu.pc=(i->cpu.p&CIV_P_Z)?0x8285u:0x8280u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_114(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x7Cu,3u,0x00FAE7u);
    if(i->cpu.x==0x0000u)i->cpu.pc=0xF8A1u;
    else if(i->cpu.x==0x0002u)i->cpu.pc=0xF8DFu;
    else if(i->cpu.x==0x0004u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0006u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0008u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x000Au)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x000Cu)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x000Eu)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0010u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0012u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0014u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0016u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0018u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x001Au)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x001Cu)i->cpu.pc=0xF8F5u;
    else if(i->cpu.x==0x001Eu)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0020u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0022u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0024u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0026u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0028u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x002Au)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x002Cu)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x002Eu)i->cpu.pc=0xF93Cu;
    else if(i->cpu.x==0x0030u)i->cpu.pc=0xF8F2u;
    else if(i->cpu.x==0x0032u)i->cpu.pc=0xF93Fu;
    else if(i->cpu.x==0x0034u)i->cpu.pc=0xF968u;
    else if(i->cpu.x==0x0036u)i->cpu.pc=0xFA2Bu;
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_115(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x7Cu,3u,0x00FE7Au);
    if(i->cpu.x==0x0000u)i->cpu.pc=0xFC41u;
    else if(i->cpu.x==0x0002u)i->cpu.pc=0xFC44u;
    else if(i->cpu.x==0x0004u)i->cpu.pc=0xFC58u;
    else if(i->cpu.x==0x0006u)i->cpu.pc=0xFC7Bu;
    else if(i->cpu.x==0x0008u)i->cpu.pc=0xFC92u;
    else if(i->cpu.x==0x000Au)i->cpu.pc=0xFCAFu;
    else if(i->cpu.x==0x000Cu)i->cpu.pc=0xFCE0u;
    else if(i->cpu.x==0x000Eu)i->cpu.pc=0xFCFDu;
    else if(i->cpu.x==0x0010u)i->cpu.pc=0xFD38u;
    else if(i->cpu.x==0x0012u)i->cpu.pc=0xFCAFu;
    else if(i->cpu.x==0x0014u)i->cpu.pc=0xFD7Du;
    else if(i->cpu.x==0x0016u)i->cpu.pc=0xFD94u;
    else if(i->cpu.x==0x0018u)i->cpu.pc=0xFC41u;
    else if(i->cpu.x==0x001Au)i->cpu.pc=0xFDAEu;
    else if(i->cpu.x==0x001Cu)i->cpu.pc=0xFDC5u;
    else if(i->cpu.x==0x001Eu)i->cpu.pc=0xFDE0u;
    else if(i->cpu.x==0x0020u)i->cpu.pc=0xFC41u;
    else if(i->cpu.x==0x0022u)i->cpu.pc=0xFDE0u;
    else if(i->cpu.x==0x0024u)i->cpu.pc=0xFDFDu;
    else if(i->cpu.x==0x0026u)i->cpu.pc=0xFDC5u;
    else if(i->cpu.x==0x0028u)i->cpu.pc=0xFDE0u;
    else if(i->cpu.x==0x002Au)i->cpu.pc=0xFE2Eu;
    else if(i->cpu.x==0x002Cu)i->cpu.pc=0xFE77u;
    else if(i->cpu.x==0x002Eu)i->cpu.pc=0xFE77u;
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->instruction_count++;return 1;
}

static int civ_aot_template_116(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x7Cu,3u,0x008D75u);
    if(i->cpu.x==0x0000u)i->cpu.pc=0x8D8Bu;
    else if(i->cpu.x==0x0002u)i->cpu.pc=0xB42Du;
    else if(i->cpu.x==0x0004u)i->cpu.pc=0x8EB5u;
    else if(i->cpu.x==0x0006u)i->cpu.pc=0x8F5Fu;
    else if(i->cpu.x==0x0008u)i->cpu.pc=0xB42Du;
    else if(i->cpu.x==0x000Au)i->cpu.pc=0x8FB2u;
    else if(i->cpu.x==0x000Cu)i->cpu.pc=0xB42Du;
    else if(i->cpu.x==0x000Eu)i->cpu.pc=0xB40Fu;
    else if(i->cpu.x==0x0010u)i->cpu.pc=0x8D58u;
    else if(i->cpu.x==0x0012u)i->cpu.pc=0xB42Du;
    else if(i->cpu.x==0x0014u)i->cpu.pc=0x8D64u;
    else return civ_fail_frontier(i,"Indexed-indirect selector is outside the finite Civilization proof set.",NULL);
    i->stage_dispatch_count++;
    i->last_stage_index=(uint16_t)(i->cpu.x>>1);
    i->instruction_count++;return 1;
}

static int civ_aot_template_117(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x72u,2u,0x0000DAu);
    { uint16_t ptr,v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+0xDAu),&ptr))return 0; if(!civ_bus_read16(i,((uint32_t)i->cpu.dbr<<16)|ptr,&v))return 0; if(!civ_adc16_binary(i,v,NULL))return 0; }
    i->cpu.pc=0x4176u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_118(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xB1u,2u,0x0000F6u);
    { uint16_t ptr,v; uint32_t a; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+0xF6u),&ptr))return 0; a=(((uint32_t)i->cpu.dbr<<16)|((uint16_t)(ptr+i->cpu.y))); if(!civ_bus_read16(i,a,&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    i->cpu.pc=0xAC23u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_119(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x11u,2u,0x000007u);
    { uint16_t ptr; uint8_t v; uint32_t a; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+0x07u),&ptr))return 0; a=(((uint32_t)i->cpu.dbr<<16)|((uint16_t)(ptr+i->cpu.y))); if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=0x917Bu;
    i->instruction_count++;return 1;
}

static int civ_aot_template_11A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xB1u,2u,0x000007u);
    { uint16_t ptr; uint8_t v; uint32_t a; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+0x07u),&ptr))return 0; a=(((uint32_t)i->cpu.dbr<<16)|((uint16_t)(ptr+i->cpu.y))); if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=0x8BD9u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_11B(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xBCu,3u,0x000185u);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(0x0185u+i->cpu.x)),&v))return 0; i->cpu.y=v; civ_set_nz16(i,v); }
    i->cpu.pc=0x82DEu;
    i->instruction_count++;return 1;
}

static int civ_aot_template_11C(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x5Eu,3u,0x000115u);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(0x0115u+i->cpu.x)),&v))return 0; if(v&0x0001u)i->cpu.p|=CIV_P_C;else i->cpu.p&=(uint8_t)~CIV_P_C; v=(uint16_t)(v>>1); if(!civ_bus_write16(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(0x0115u+i->cpu.x)),v))return 0; civ_set_nz16(i,v); }
    i->cpu.pc=0x5948u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_11D(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xADu,3u,0x000181u);
    { uint16_t v; if(!civ_bus_read16(i,(((uint32_t)i->cpu.dbr<<16)|0x0181u),&v))return 0; i->cpu.a=v; civ_set_nz16(i,v); }
    if(!i->natural_timing_enabled)civ_timing_advance_master(i,8u);
    i->cpu.pc=0x8283u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_11E(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x5Fu,4u,0x7E6820u);
    { uint16_t v; if(!civ_bus_read16(i,(0x7E6820u+i->cpu.x)&0xFFFFFFu,&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a^v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=0x01A4u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_11F(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xC4u,2u,0x00001Fu);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+0x1Fu),&v))return 0; civ_cmp16(i,i->cpu.y,v); }
    i->cpu.pc=0x7D26u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_120(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x35u,2u,0x00000Du);
    { uint16_t v; if(!civ_bus_read16(i,(uint16_t)(i->cpu.d+0x0Du+i->cpu.x),&v))return 0; i->cpu.a=(uint16_t)(i->cpu.a&v); civ_set_nz16(i,i->cpu.a); }
    i->cpu.pc=0x3E16u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_121(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x60u,1u,0x000000u);
    { uint8_t lo,hi; uint16_t target; if(!civ_pull8(i,&lo)||!civ_pull8(i,&hi))return 0;
      target=(uint16_t)((uint16_t)(lo|((uint16_t)hi<<8))+1u);
      if(!civ_generated_return_allowed(2090u,((uint32_t)i->cpu.pbr<<16)|target))return civ_fail_frontier(i,"RTS returned outside the finite Civilization proof set.",NULL);
      i->cpu.pc=target; }
    if(i->cpu.pc==0x8746u)i->descriptor_handler_8746_count++;
    else if(i->cpu.pc==0x8780u)i->descriptor_handler_8780_count++;
    else if(i->cpu.pc==0x8598u)i->descriptor_handler_8598_count++;
    i->instruction_count++;return 1;
}

static int civ_aot_template_122(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x07u,2u,0x0000CBu);
    { uint8_t p0,p1,p2,v; uint16_t b=(uint16_t)(i->cpu.d+0xCBu); if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; if(!civ_bus_read8(i,((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=0x329Fu;
    i->instruction_count++;return 1;
}

static int civ_aot_template_123(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x37u,2u,0x00000Du);
    { uint8_t p0,p1,p2,v; uint16_t b=(uint16_t)(i->cpu.d+0x0Du); uint32_t a; if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; a=((((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)))+i->cpu.y)&0xFFFFFFu; if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=0x3CE8u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_124(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x87u,2u,0x000007u);
    { uint8_t p0,p1,p2; uint16_t b=(uint16_t)(i->cpu.d+0x07u); if(!civ_bus_read8(i,b,&p0)||!civ_bus_read8(i,(uint16_t)(b+1u),&p1)||!civ_bus_read8(i,(uint16_t)(b+2u),&p2))return 0; if(!civ_bus_write8(i,((uint32_t)p2<<16)|((uint16_t)p0|((uint16_t)p1<<8)),(uint8_t)i->cpu.a))return 0; }
    i->cpu.pc=0x32A1u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_125(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x14u,2u,0x000047u);
    { uint8_t v,a=(uint8_t)i->cpu.a; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+0x47u),&v))return 0; if((uint8_t)(v&a)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; v=(uint8_t)(v&~a); if(!civ_bus_write8(i,(uint16_t)(i->cpu.d+0x47u),v))return 0; }
    i->cpu.pc=0x098Eu;
    i->instruction_count++;return 1;
}

static int civ_aot_template_126(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x39u,3u,0x000300u);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|(uint16_t)(0x0300u+i->cpu.y)),&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a&v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=0x3CDFu;
    i->instruction_count++;return 1;
}

static int civ_aot_template_127(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0xAEu,3u,0x0019FEu);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|0x19FEu),&v))return 0; i->cpu.x=v; civ_set_nz8(i,v); }
    i->cpu.pc=0x38CCu;
    i->instruction_count++;return 1;
}

static int civ_aot_template_128(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x2Cu,3u,0x001C0Cu);
    { uint8_t v; if(!civ_bus_read8(i,(((uint32_t)i->cpu.dbr<<16)|0x1C0Cu),&v))return 0; if(((uint8_t)i->cpu.a&v)==0u)i->cpu.p|=CIV_P_Z;else i->cpu.p&=(uint8_t)~CIV_P_Z; i->cpu.p=(uint8_t)((i->cpu.p&~(CIV_P_N|CIV_P_V))|(v&(CIV_P_N|CIV_P_V))); }
    i->cpu.pc=0xFE51u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_129(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x15u,2u,0x00000Bu);
    { uint8_t v; if(!civ_bus_read8(i,(uint16_t)(i->cpu.d+0x0Bu+i->cpu.x),&v))return 0; v=(uint8_t)((uint8_t)i->cpu.a|v); i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|v); civ_set_nz8(i,v); }
    i->cpu.pc=0x2DF6u;
    i->instruction_count++;return 1;
}

static int civ_aot_template_12A(CivRecomp *i,const uint32_t *p){
  (void)p;
    civ_cpu_begin_static_instruction(i,0x03u,2u,0x000001u);
    { uint8_t v; uint16_t a=(uint16_t)(i->cpu.s+0x01u); if(!civ_bus_read8(i,a,&v))return 0; i->cpu.a=(uint16_t)((i->cpu.a&0xFF00u)|((uint8_t)i->cpu.a|v)); civ_set_nz8(i,(uint8_t)i->cpu.a); }
    i->cpu.pc=0x946Eu;
    i->instruction_count++;return 1;
}

int civ_compact_aot_execute(CivRecomp *i,uint16_t template_id,const uint32_t *p){
  switch(template_id){
  case 0x000u:return civ_aot_template_000(i,p);
  case 0x001u:return civ_aot_template_001(i,p);
  case 0x002u:return civ_aot_template_002(i,p);
  case 0x003u:return civ_aot_template_003(i,p);
  case 0x004u:return civ_aot_template_004(i,p);
  case 0x005u:return civ_aot_template_005(i,p);
  case 0x006u:return civ_aot_template_006(i,p);
  case 0x007u:return civ_aot_template_007(i,p);
  case 0x008u:return civ_aot_template_008(i,p);
  case 0x009u:return civ_aot_template_009(i,p);
  case 0x00Au:return civ_aot_template_00A(i,p);
  case 0x00Bu:return civ_aot_template_00B(i,p);
  case 0x00Cu:return civ_aot_template_00C(i,p);
  case 0x00Du:return civ_aot_template_00D(i,p);
  case 0x00Eu:return civ_aot_template_00E(i,p);
  case 0x00Fu:return civ_aot_template_00F(i,p);
  case 0x010u:return civ_aot_template_010(i,p);
  case 0x011u:return civ_aot_template_011(i,p);
  case 0x012u:return civ_aot_template_012(i,p);
  case 0x013u:return civ_aot_template_013(i,p);
  case 0x014u:return civ_aot_template_014(i,p);
  case 0x015u:return civ_aot_template_015(i,p);
  case 0x016u:return civ_aot_template_016(i,p);
  case 0x017u:return civ_aot_template_017(i,p);
  case 0x018u:return civ_aot_template_018(i,p);
  case 0x019u:return civ_aot_template_019(i,p);
  case 0x01Au:return civ_aot_template_01A(i,p);
  case 0x01Bu:return civ_aot_template_01B(i,p);
  case 0x01Cu:return civ_aot_template_01C(i,p);
  case 0x01Du:return civ_aot_template_01D(i,p);
  case 0x01Eu:return civ_aot_template_01E(i,p);
  case 0x01Fu:return civ_aot_template_01F(i,p);
  case 0x020u:return civ_aot_template_020(i,p);
  case 0x021u:return civ_aot_template_021(i,p);
  case 0x022u:return civ_aot_template_022(i,p);
  case 0x023u:return civ_aot_template_023(i,p);
  case 0x024u:return civ_aot_template_024(i,p);
  case 0x025u:return civ_aot_template_025(i,p);
  case 0x026u:return civ_aot_template_026(i,p);
  case 0x027u:return civ_aot_template_027(i,p);
  case 0x028u:return civ_aot_template_028(i,p);
  case 0x029u:return civ_aot_template_029(i,p);
  case 0x02Au:return civ_aot_template_02A(i,p);
  case 0x02Bu:return civ_aot_template_02B(i,p);
  case 0x02Cu:return civ_aot_template_02C(i,p);
  case 0x02Du:return civ_aot_template_02D(i,p);
  case 0x02Eu:return civ_aot_template_02E(i,p);
  case 0x02Fu:return civ_aot_template_02F(i,p);
  case 0x030u:return civ_aot_template_030(i,p);
  case 0x031u:return civ_aot_template_031(i,p);
  case 0x032u:return civ_aot_template_032(i,p);
  case 0x033u:return civ_aot_template_033(i,p);
  case 0x034u:return civ_aot_template_034(i,p);
  case 0x035u:return civ_aot_template_035(i,p);
  case 0x036u:return civ_aot_template_036(i,p);
  case 0x037u:return civ_aot_template_037(i,p);
  case 0x038u:return civ_aot_template_038(i,p);
  case 0x039u:return civ_aot_template_039(i,p);
  case 0x03Au:return civ_aot_template_03A(i,p);
  case 0x03Bu:return civ_aot_template_03B(i,p);
  case 0x03Cu:return civ_aot_template_03C(i,p);
  case 0x03Du:return civ_aot_template_03D(i,p);
  case 0x03Eu:return civ_aot_template_03E(i,p);
  case 0x03Fu:return civ_aot_template_03F(i,p);
  case 0x040u:return civ_aot_template_040(i,p);
  case 0x041u:return civ_aot_template_041(i,p);
  case 0x042u:return civ_aot_template_042(i,p);
  case 0x043u:return civ_aot_template_043(i,p);
  case 0x044u:return civ_aot_template_044(i,p);
  case 0x045u:return civ_aot_template_045(i,p);
  case 0x046u:return civ_aot_template_046(i,p);
  case 0x047u:return civ_aot_template_047(i,p);
  case 0x048u:return civ_aot_template_048(i,p);
  case 0x049u:return civ_aot_template_049(i,p);
  case 0x04Au:return civ_aot_template_04A(i,p);
  case 0x04Bu:return civ_aot_template_04B(i,p);
  case 0x04Cu:return civ_aot_template_04C(i,p);
  case 0x04Du:return civ_aot_template_04D(i,p);
  case 0x04Eu:return civ_aot_template_04E(i,p);
  case 0x04Fu:return civ_aot_template_04F(i,p);
  case 0x050u:return civ_aot_template_050(i,p);
  case 0x051u:return civ_aot_template_051(i,p);
  case 0x052u:return civ_aot_template_052(i,p);
  case 0x053u:return civ_aot_template_053(i,p);
  case 0x054u:return civ_aot_template_054(i,p);
  case 0x055u:return civ_aot_template_055(i,p);
  case 0x056u:return civ_aot_template_056(i,p);
  case 0x057u:return civ_aot_template_057(i,p);
  case 0x058u:return civ_aot_template_058(i,p);
  case 0x059u:return civ_aot_template_059(i,p);
  case 0x05Au:return civ_aot_template_05A(i,p);
  case 0x05Bu:return civ_aot_template_05B(i,p);
  case 0x05Cu:return civ_aot_template_05C(i,p);
  case 0x05Du:return civ_aot_template_05D(i,p);
  case 0x05Eu:return civ_aot_template_05E(i,p);
  case 0x05Fu:return civ_aot_template_05F(i,p);
  case 0x060u:return civ_aot_template_060(i,p);
  case 0x061u:return civ_aot_template_061(i,p);
  case 0x062u:return civ_aot_template_062(i,p);
  case 0x063u:return civ_aot_template_063(i,p);
  case 0x064u:return civ_aot_template_064(i,p);
  case 0x065u:return civ_aot_template_065(i,p);
  case 0x066u:return civ_aot_template_066(i,p);
  case 0x067u:return civ_aot_template_067(i,p);
  case 0x068u:return civ_aot_template_068(i,p);
  case 0x069u:return civ_aot_template_069(i,p);
  case 0x06Au:return civ_aot_template_06A(i,p);
  case 0x06Bu:return civ_aot_template_06B(i,p);
  case 0x06Cu:return civ_aot_template_06C(i,p);
  case 0x06Du:return civ_aot_template_06D(i,p);
  case 0x06Eu:return civ_aot_template_06E(i,p);
  case 0x06Fu:return civ_aot_template_06F(i,p);
  case 0x070u:return civ_aot_template_070(i,p);
  case 0x071u:return civ_aot_template_071(i,p);
  case 0x072u:return civ_aot_template_072(i,p);
  case 0x073u:return civ_aot_template_073(i,p);
  case 0x074u:return civ_aot_template_074(i,p);
  case 0x075u:return civ_aot_template_075(i,p);
  case 0x076u:return civ_aot_template_076(i,p);
  case 0x077u:return civ_aot_template_077(i,p);
  case 0x078u:return civ_aot_template_078(i,p);
  case 0x079u:return civ_aot_template_079(i,p);
  case 0x07Au:return civ_aot_template_07A(i,p);
  case 0x07Bu:return civ_aot_template_07B(i,p);
  case 0x07Cu:return civ_aot_template_07C(i,p);
  case 0x07Du:return civ_aot_template_07D(i,p);
  case 0x07Eu:return civ_aot_template_07E(i,p);
  case 0x07Fu:return civ_aot_template_07F(i,p);
  case 0x080u:return civ_aot_template_080(i,p);
  case 0x081u:return civ_aot_template_081(i,p);
  case 0x082u:return civ_aot_template_082(i,p);
  case 0x083u:return civ_aot_template_083(i,p);
  case 0x084u:return civ_aot_template_084(i,p);
  case 0x085u:return civ_aot_template_085(i,p);
  case 0x086u:return civ_aot_template_086(i,p);
  case 0x087u:return civ_aot_template_087(i,p);
  case 0x088u:return civ_aot_template_088(i,p);
  case 0x089u:return civ_aot_template_089(i,p);
  case 0x08Au:return civ_aot_template_08A(i,p);
  case 0x08Bu:return civ_aot_template_08B(i,p);
  case 0x08Cu:return civ_aot_template_08C(i,p);
  case 0x08Du:return civ_aot_template_08D(i,p);
  case 0x08Eu:return civ_aot_template_08E(i,p);
  case 0x08Fu:return civ_aot_template_08F(i,p);
  case 0x090u:return civ_aot_template_090(i,p);
  case 0x091u:return civ_aot_template_091(i,p);
  case 0x092u:return civ_aot_template_092(i,p);
  case 0x093u:return civ_aot_template_093(i,p);
  case 0x094u:return civ_aot_template_094(i,p);
  case 0x095u:return civ_aot_template_095(i,p);
  case 0x096u:return civ_aot_template_096(i,p);
  case 0x097u:return civ_aot_template_097(i,p);
  case 0x098u:return civ_aot_template_098(i,p);
  case 0x099u:return civ_aot_template_099(i,p);
  case 0x09Au:return civ_aot_template_09A(i,p);
  case 0x09Bu:return civ_aot_template_09B(i,p);
  case 0x09Cu:return civ_aot_template_09C(i,p);
  case 0x09Du:return civ_aot_template_09D(i,p);
  case 0x09Eu:return civ_aot_template_09E(i,p);
  case 0x09Fu:return civ_aot_template_09F(i,p);
  case 0x0A0u:return civ_aot_template_0A0(i,p);
  case 0x0A1u:return civ_aot_template_0A1(i,p);
  case 0x0A2u:return civ_aot_template_0A2(i,p);
  case 0x0A3u:return civ_aot_template_0A3(i,p);
  case 0x0A4u:return civ_aot_template_0A4(i,p);
  case 0x0A5u:return civ_aot_template_0A5(i,p);
  case 0x0A6u:return civ_aot_template_0A6(i,p);
  case 0x0A7u:return civ_aot_template_0A7(i,p);
  case 0x0A8u:return civ_aot_template_0A8(i,p);
  case 0x0A9u:return civ_aot_template_0A9(i,p);
  case 0x0AAu:return civ_aot_template_0AA(i,p);
  case 0x0ABu:return civ_aot_template_0AB(i,p);
  case 0x0ACu:return civ_aot_template_0AC(i,p);
  case 0x0ADu:return civ_aot_template_0AD(i,p);
  case 0x0AEu:return civ_aot_template_0AE(i,p);
  case 0x0AFu:return civ_aot_template_0AF(i,p);
  case 0x0B0u:return civ_aot_template_0B0(i,p);
  case 0x0B1u:return civ_aot_template_0B1(i,p);
  case 0x0B2u:return civ_aot_template_0B2(i,p);
  case 0x0B3u:return civ_aot_template_0B3(i,p);
  case 0x0B4u:return civ_aot_template_0B4(i,p);
  case 0x0B5u:return civ_aot_template_0B5(i,p);
  case 0x0B6u:return civ_aot_template_0B6(i,p);
  case 0x0B7u:return civ_aot_template_0B7(i,p);
  case 0x0B8u:return civ_aot_template_0B8(i,p);
  case 0x0B9u:return civ_aot_template_0B9(i,p);
  case 0x0BAu:return civ_aot_template_0BA(i,p);
  case 0x0BBu:return civ_aot_template_0BB(i,p);
  case 0x0BCu:return civ_aot_template_0BC(i,p);
  case 0x0BDu:return civ_aot_template_0BD(i,p);
  case 0x0BEu:return civ_aot_template_0BE(i,p);
  case 0x0BFu:return civ_aot_template_0BF(i,p);
  case 0x0C0u:return civ_aot_template_0C0(i,p);
  case 0x0C1u:return civ_aot_template_0C1(i,p);
  case 0x0C2u:return civ_aot_template_0C2(i,p);
  case 0x0C3u:return civ_aot_template_0C3(i,p);
  case 0x0C4u:return civ_aot_template_0C4(i,p);
  case 0x0C5u:return civ_aot_template_0C5(i,p);
  case 0x0C6u:return civ_aot_template_0C6(i,p);
  case 0x0C7u:return civ_aot_template_0C7(i,p);
  case 0x0C8u:return civ_aot_template_0C8(i,p);
  case 0x0C9u:return civ_aot_template_0C9(i,p);
  case 0x0CAu:return civ_aot_template_0CA(i,p);
  case 0x0CBu:return civ_aot_template_0CB(i,p);
  case 0x0CCu:return civ_aot_template_0CC(i,p);
  case 0x0CDu:return civ_aot_template_0CD(i,p);
  case 0x0CEu:return civ_aot_template_0CE(i,p);
  case 0x0CFu:return civ_aot_template_0CF(i,p);
  case 0x0D0u:return civ_aot_template_0D0(i,p);
  case 0x0D1u:return civ_aot_template_0D1(i,p);
  case 0x0D2u:return civ_aot_template_0D2(i,p);
  case 0x0D3u:return civ_aot_template_0D3(i,p);
  case 0x0D4u:return civ_aot_template_0D4(i,p);
  case 0x0D5u:return civ_aot_template_0D5(i,p);
  case 0x0D6u:return civ_aot_template_0D6(i,p);
  case 0x0D7u:return civ_aot_template_0D7(i,p);
  case 0x0D8u:return civ_aot_template_0D8(i,p);
  case 0x0D9u:return civ_aot_template_0D9(i,p);
  case 0x0DAu:return civ_aot_template_0DA(i,p);
  case 0x0DBu:return civ_aot_template_0DB(i,p);
  case 0x0DCu:return civ_aot_template_0DC(i,p);
  case 0x0DDu:return civ_aot_template_0DD(i,p);
  case 0x0DEu:return civ_aot_template_0DE(i,p);
  case 0x0DFu:return civ_aot_template_0DF(i,p);
  case 0x0E0u:return civ_aot_template_0E0(i,p);
  case 0x0E1u:return civ_aot_template_0E1(i,p);
  case 0x0E2u:return civ_aot_template_0E2(i,p);
  case 0x0E3u:return civ_aot_template_0E3(i,p);
  case 0x0E4u:return civ_aot_template_0E4(i,p);
  case 0x0E5u:return civ_aot_template_0E5(i,p);
  case 0x0E6u:return civ_aot_template_0E6(i,p);
  case 0x0E7u:return civ_aot_template_0E7(i,p);
  case 0x0E8u:return civ_aot_template_0E8(i,p);
  case 0x0E9u:return civ_aot_template_0E9(i,p);
  case 0x0EAu:return civ_aot_template_0EA(i,p);
  case 0x0EBu:return civ_aot_template_0EB(i,p);
  case 0x0ECu:return civ_aot_template_0EC(i,p);
  case 0x0EDu:return civ_aot_template_0ED(i,p);
  case 0x0EEu:return civ_aot_template_0EE(i,p);
  case 0x0EFu:return civ_aot_template_0EF(i,p);
  case 0x0F0u:return civ_aot_template_0F0(i,p);
  case 0x0F1u:return civ_aot_template_0F1(i,p);
  case 0x0F2u:return civ_aot_template_0F2(i,p);
  case 0x0F3u:return civ_aot_template_0F3(i,p);
  case 0x0F4u:return civ_aot_template_0F4(i,p);
  case 0x0F5u:return civ_aot_template_0F5(i,p);
  case 0x0F6u:return civ_aot_template_0F6(i,p);
  case 0x0F7u:return civ_aot_template_0F7(i,p);
  case 0x0F8u:return civ_aot_template_0F8(i,p);
  case 0x0F9u:return civ_aot_template_0F9(i,p);
  case 0x0FAu:return civ_aot_template_0FA(i,p);
  case 0x0FBu:return civ_aot_template_0FB(i,p);
  case 0x0FCu:return civ_aot_template_0FC(i,p);
  case 0x0FDu:return civ_aot_template_0FD(i,p);
  case 0x0FEu:return civ_aot_template_0FE(i,p);
  case 0x0FFu:return civ_aot_template_0FF(i,p);
  case 0x100u:return civ_aot_template_100(i,p);
  case 0x101u:return civ_aot_template_101(i,p);
  case 0x102u:return civ_aot_template_102(i,p);
  case 0x103u:return civ_aot_template_103(i,p);
  case 0x104u:return civ_aot_template_104(i,p);
  case 0x105u:return civ_aot_template_105(i,p);
  case 0x106u:return civ_aot_template_106(i,p);
  case 0x107u:return civ_aot_template_107(i,p);
  case 0x108u:return civ_aot_template_108(i,p);
  case 0x109u:return civ_aot_template_109(i,p);
  case 0x10Au:return civ_aot_template_10A(i,p);
  case 0x10Bu:return civ_aot_template_10B(i,p);
  case 0x10Cu:return civ_aot_template_10C(i,p);
  case 0x10Du:return civ_aot_template_10D(i,p);
  case 0x10Eu:return civ_aot_template_10E(i,p);
  case 0x10Fu:return civ_aot_template_10F(i,p);
  case 0x110u:return civ_aot_template_110(i,p);
  case 0x111u:return civ_aot_template_111(i,p);
  case 0x112u:return civ_aot_template_112(i,p);
  case 0x113u:return civ_aot_template_113(i,p);
  case 0x114u:return civ_aot_template_114(i,p);
  case 0x115u:return civ_aot_template_115(i,p);
  case 0x116u:return civ_aot_template_116(i,p);
  case 0x117u:return civ_aot_template_117(i,p);
  case 0x118u:return civ_aot_template_118(i,p);
  case 0x119u:return civ_aot_template_119(i,p);
  case 0x11Au:return civ_aot_template_11A(i,p);
  case 0x11Bu:return civ_aot_template_11B(i,p);
  case 0x11Cu:return civ_aot_template_11C(i,p);
  case 0x11Du:return civ_aot_template_11D(i,p);
  case 0x11Eu:return civ_aot_template_11E(i,p);
  case 0x11Fu:return civ_aot_template_11F(i,p);
  case 0x120u:return civ_aot_template_120(i,p);
  case 0x121u:return civ_aot_template_121(i,p);
  case 0x122u:return civ_aot_template_122(i,p);
  case 0x123u:return civ_aot_template_123(i,p);
  case 0x124u:return civ_aot_template_124(i,p);
  case 0x125u:return civ_aot_template_125(i,p);
  case 0x126u:return civ_aot_template_126(i,p);
  case 0x127u:return civ_aot_template_127(i,p);
  case 0x128u:return civ_aot_template_128(i,p);
  case 0x129u:return civ_aot_template_129(i,p);
  case 0x12Au:return civ_aot_template_12A(i,p);
  default:return civ_fail_frontier(i,"Unknown compact AOT semantic template.",NULL);
  }
}
