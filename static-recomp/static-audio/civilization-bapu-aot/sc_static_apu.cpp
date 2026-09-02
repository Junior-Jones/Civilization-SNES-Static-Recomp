#include "sc_static_apu.h"
#include "sc_audio_clock_profile.h"
#include "static_snes.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace SC_STATIC_SNES { CPU cpu; }
namespace {
constexpr uint64_t kFnvOffset=UINT64_C(1469598103934665603);
constexpr uint64_t kFnvPrime=UINT64_C(1099511628211);
SCStaticAudioSink g_sink=nullptr;
void *g_sink_context=nullptr;
SCStaticTraceCallbacks g_trace={nullptr,nullptr,nullptr,nullptr,nullptr};
uint64_t g_master=0,g_cycles=0,g_pcm_frames=0;
uint64_t g_sync_calls=0,g_rendezvous_hash=kFnvOffset;
uint64_t g_cpu_port_event_count=0,g_cpu_port_event_hash=kFnvOffset;
uint32_t g_phase_remainder=0,g_max_sync_master_delta=0,g_first_sync_master_delta=0,g_max_followup_sync_master_delta=0;
uint16_t g_current_instruction_pc=0xffc0u;
uint32_t g_code_write_barriers=0;
uint64_t g_sdsp_primitive_steps=0,g_sdsp_primitive_hash=kFnvOffset;
uint64_t g_sdsp_brr_steps=0,g_sdsp_brr_hash=kFnvOffset;
uint8_t g_sdsp_static_failed=0,g_sdsp_fail_reason=0,g_sdsp_fail_phase=0;
bool g_program_started=false;
int g_acquired=0;
FILE *g_diag_trace=nullptr,*g_cpu_apuio_trace=nullptr;
uint64_t g_diag_start_cycle=0u,g_diag_end_cycle=0u,g_active_chunk_target=0u;

uint64_t diagnostic_cycle(){
  const int64_t cycle=(int64_t)g_active_chunk_target+(int64_t)SC_STATIC_SNES::smp.clock;
  return cycle<0?0u:(uint64_t)cycle;
}
void close_diag_trace(){if(g_diag_trace){std::fclose(g_diag_trace);g_diag_trace=nullptr;}if(g_cpu_apuio_trace){std::fclose(g_cpu_apuio_trace);g_cpu_apuio_trace=nullptr;}}
void configure_diag_trace(){
  close_diag_trace();g_diag_start_cycle=g_diag_end_cycle=0u;
  const char *cpu_path=std::getenv("CIV_V21_CPU_APUIO_TRACE");
  if(cpu_path&&*cpu_path){g_cpu_apuio_trace=std::fopen(cpu_path,"w");if(g_cpu_apuio_trace)std::fputs("event,direction,master,port,value,smp_cycles,smp_clock,smp_pc\n",g_cpu_apuio_trace);}
  const char *path=std::getenv("CIV_V21_SMP_TRACE");
  const char *start=std::getenv("CIV_V21_SMP_TRACE_START");
  const char *end=std::getenv("CIV_V21_SMP_TRACE_END");
  if(!path||!*path||!start||!*start||!end||!*end)return;
  char *tail=nullptr;g_diag_start_cycle=std::strtoull(start,&tail,0);if(!tail||*tail)return;
  tail=nullptr;g_diag_end_cycle=std::strtoull(end,&tail,0);if(!tail||*tail||g_diag_end_cycle<g_diag_start_cycle)return;
  g_diag_trace=std::fopen(path,"w");
  if(g_diag_trace)std::fputs("kind,cycle,pc,opcode,direction,port,value\n",g_diag_trace);
}
void log_diag_instruction(uint16_t pc,uint8_t opcode){
  if(!g_diag_trace)return;const uint64_t cycle=diagnostic_cycle();
  if(cycle<g_diag_start_cycle||cycle>g_diag_end_cycle)return;
  std::fprintf(g_diag_trace,"I,%llu,%04X,%02X,,,\n",(unsigned long long)cycle,(unsigned)pc,(unsigned)opcode);
}
void log_diag_port(uint16_t pc,uint8_t direction,uint8_t port,uint8_t value){
  if(!g_diag_trace)return;const uint64_t cycle=diagnostic_cycle();
  if(cycle<g_diag_start_cycle||cycle>g_diag_end_cycle)return;
  std::fprintf(g_diag_trace,"P,%llu,%04X,,%u,%u,%02X\n",(unsigned long long)cycle,(unsigned)pc,(unsigned)direction,(unsigned)port,(unsigned)value);
}
struct ApuSnapshotHeader {
  char magic[8]; uint32_t version,smp_size,aram_size,dsp_capacity;
  uint8_t cpu_registers[4]; uint32_t dsp_frequency; int32_t dsp_clock;
  uint64_t master,cycles,pcm_frames,sync_calls,rendezvous_hash;
  uint64_t cpu_port_event_count,cpu_port_event_hash;
  uint32_t phase_remainder,max_sync_master_delta,first_sync_master_delta;
  uint32_t max_followup_sync_master_delta,current_instruction_pc,code_write_barriers;
  uint64_t sdsp_primitive_steps,sdsp_primitive_hash,sdsp_brr_steps,sdsp_brr_hash;
  uint8_t sdsp_static_failed,sdsp_fail_reason,sdsp_fail_phase,program_started;
};
#include "smp/sc_smp_aot_code_bitmap.inc"
void copy_error(char *out,size_t cap,const char *text){if(out&&cap)std::snprintf(out,cap,"%s",text?text:"");}
uint64_t mix8(uint64_t h,uint8_t v){h^=v;return h*kFnvPrime;}
uint64_t mix32(uint64_t h,uint32_t v){for(unsigned i=0;i<4u;++i)h=mix8(h,(uint8_t)(v>>(i*8u)));return h;}
uint64_t mix64(uint64_t h,uint64_t v){for(unsigned i=0;i<8u;++i)h=mix8(h,(uint8_t)(v>>(i*8u)));return h;}
bool is_code_byte(uint16_t address){return(sc_smp_aot_code_bitmap[address>>3]&(uint8)(1u<<(address&7u)))!=0u;}
uint8_t timer_enable_mask(){return (SC_STATIC_SNES::smp.timer0.enable?1u:0u)|(SC_STATIC_SNES::smp.timer1.enable?2u:0u)|(SC_STATIC_SNES::smp.timer2.enable?4u:0u);}
template <typename TimerT> void mix_timer(uint64_t &h,const TimerT &timer){h=mix8(h,timer.target);h=mix8(h,timer.stage1_ticks);h=mix8(h,timer.stage2_ticks);h=mix8(h,timer.stage3_ticks);}
template <typename TimerT> void store_timer(uint8_t *target,uint8_t *stage1,uint8_t *stage2,uint8_t *stage3,unsigned index,const TimerT &timer){target[index]=timer.target;stage1[index]=timer.stage1_ticks;stage2[index]=timer.stage2_ticks;stage3[index]=timer.stage3_ticks;}
void record_rendezvous(uint64_t master,uint64_t target_cycles,uint64_t master_delta){
  const uint32_t bounded_delta=(uint32_t)(master_delta>UINT32_MAX?UINT32_MAX:master_delta);
  if(g_sync_calls==0u)g_first_sync_master_delta=bounded_delta;
  else if(master_delta>g_max_followup_sync_master_delta)g_max_followup_sync_master_delta=bounded_delta;
  ++g_sync_calls;
  if(master_delta>g_max_sync_master_delta)g_max_sync_master_delta=bounded_delta;
  uint64_t h=g_rendezvous_hash;
  h=mix64(h,master);h=mix64(h,target_cycles);h=mix32(h,g_phase_remainder);
  h=mix32(h,(uint32_t)SC_STATIC_SNES::smp.clock);
  h=mix8(h,(uint8_t)SC_STATIC_SNES::dsp.core.phase);
  h=mix8(h,timer_enable_mask());
  mix_timer(h,SC_STATIC_SNES::smp.timer0);
  mix_timer(h,SC_STATIC_SNES::smp.timer1);
  mix_timer(h,SC_STATIC_SNES::smp.timer2);
  g_rendezvous_hash=h;
}
void record_cpu_port(uint8_t direction,uint64_t master,unsigned port,uint8_t value){
  ++g_cpu_port_event_count;
  if(g_cpu_apuio_trace)std::fprintf(g_cpu_apuio_trace,"%llu,%u,%llu,%u,%02X,%llu,%d,%04X\n",
    (unsigned long long)g_cpu_port_event_count,(unsigned)direction,(unsigned long long)master,(unsigned)port,(unsigned)value,
    (unsigned long long)g_cycles,(int)SC_STATIC_SNES::smp.clock,(unsigned)SC_STATIC_SNES::smp.regs.pc);
  uint64_t h=g_cpu_port_event_hash;
  h=mix8(h,direction);h=mix8(h,(uint8_t)port);h=mix8(h,value);
  h=mix64(h,master);h=mix64(h,g_cycles);h=mix32(h,g_phase_remainder);
  h=mix32(h,(uint32_t)SC_STATIC_SNES::smp.clock);
  g_cpu_port_event_hash=h;
}
int fail_message(char *error,size_t cap){
  const auto &s=SC_STATIC_SNES::smp;
  char text[224];
  const char *reason=s.sc_aot_fail_reason_value==1u?"unknown exact S-SMP PC":
    s.sc_aot_fail_reason_value==2u?"S-SMP opcode mismatch":
    s.sc_aot_fail_reason_value==3u?"unemitted S-SMP opcode":
    s.sc_aot_fail_reason_value==4u?"write to statically compiled S-SMP code":
    s.sc_aot_fail_reason_value==5u?"unknown ARAM read":"static S-SMP AOT failure";
  if(s.sc_aot_fail_reason_value==5u)
    std::snprintf(text,sizeof(text),"%s: PC=%04X address=%02X%02X",reason,
      (unsigned)s.sc_aot_fail_pc_value,(unsigned)s.sc_aot_expected_opcode,(unsigned)s.sc_aot_actual_opcode);
  else
    std::snprintf(text,sizeof(text),"%s: PC=%04X expected=%02X actual=%02X",reason,
      (unsigned)s.sc_aot_fail_pc_value,(unsigned)s.sc_aot_expected_opcode,(unsigned)s.sc_aot_actual_opcode);
  copy_error(error,cap,text);return 0;
}
int sync_impl(uint64_t master,char *error,size_t cap){
  if(!g_acquired){copy_error(error,cap,"Static S-SMP AOT is not acquired.");return 0;}
  if(master<g_master){copy_error(error,cap,"Static S-SMP AOT master clock moved backwards.");return 0;}
  if(SC_STATIC_SNES::smp.sc_aot_failed_value)return fail_message(error,cap);
  if(g_sdsp_static_failed){copy_error(error,cap,"Static S-DSP program failure.");return 0;}
  const uint64_t master_delta=master-g_master;
  uint64_t delta=sc_audio_clock_advance(master_delta,&g_phase_remainder);
  const uint64_t target=g_cycles+delta;
  const uint64_t closed_form=sc_audio_clock_closed_form(master,0u);
  const uint32_t closed_remainder=sc_audio_clock_remainder(master);
  if(target!=closed_form||g_phase_remainder!=closed_remainder){copy_error(error,cap,"Static S-SMP AOT clock accumulator invariant failed.");return 0;}
  while(delta){
    unsigned chunk=(delta>131072u)?131072u:(unsigned)delta;
    g_active_chunk_target=g_cycles+(uint64_t)chunk;
    SC_STATIC_SNES::smp.clock-=(int32)chunk;
    SC_STATIC_SNES::smp.enter();
    if(SC_STATIC_SNES::smp.sc_aot_failed_value)return fail_message(error,cap);
    SC_STATIC_SNES::dsp.synchronize();g_cycles+=chunk;delta-=chunk;
  }
  g_master=master;record_rendezvous(master,target,master_delta);copy_error(error,cap,"");return 1;
}
}
extern "C" int sc_static_apu_acquire(char *error,size_t cap){
  if(g_acquired){copy_error(error,cap,"Only one Full Static S-SMP AOT instance is supported.");return 0;}
  g_acquired=1;sc_static_apu_reset();copy_error(error,cap,"");return 1;
}
extern "C" void sc_static_apu_release(void){close_diag_trace();g_acquired=0;g_sink=nullptr;g_sink_context=nullptr;std::memset(&g_trace,0,sizeof(g_trace));}
extern "C" void sc_static_apu_reset(void){
  SC_STATIC_SNES::cpu.reset();SC_STATIC_SNES::smp.power();SC_STATIC_SNES::dsp.power();
  g_master=g_cycles=g_pcm_frames=0;g_phase_remainder=0;
  g_max_sync_master_delta=0;g_first_sync_master_delta=0;
  g_max_followup_sync_master_delta=0;
  g_sync_calls=g_cpu_port_event_count=0;g_rendezvous_hash=g_cpu_port_event_hash=kFnvOffset;
  g_current_instruction_pc=0xffc0u;g_code_write_barriers=0;g_program_started=false;
  g_sdsp_primitive_steps=g_sdsp_brr_steps=0;g_sdsp_primitive_hash=g_sdsp_brr_hash=kFnvOffset;
  g_sdsp_static_failed=g_sdsp_fail_reason=g_sdsp_fail_phase=0;g_active_chunk_target=0u;
  configure_diag_trace();
}
extern "C" void sc_static_apu_set_sink(SCStaticAudioSink sink,void *context){g_sink=sink;g_sink_context=context;}
extern "C" void sc_static_apu_set_trace_callbacks(const SCStaticTraceCallbacks *callbacks){if(callbacks)g_trace=*callbacks;else std::memset(&g_trace,0,sizeof(g_trace));}
extern "C" void sc_static_apu_trace_instruction_event(uint16_t pc,uint8_t opcode){
  g_current_instruction_pc=pc;if(pc==0x0800u)g_program_started=true;log_diag_instruction(pc,opcode);
  if(g_trace.instruction)g_trace.instruction(g_trace.context,pc,opcode);
}
extern "C" int sc_static_apu_trace_aram_write_event(uint16_t address,uint8_t value){
  if(g_program_started&&is_code_byte(address)){++g_code_write_barriers;return 0;}
  if(g_trace.aram_write&&!g_trace.aram_write(g_trace.context,g_current_instruction_pc,address,value))return 0;
  return 1;
}
extern "C" void sc_static_apu_trace_dsp_write_event(uint8_t address,uint8_t value){if(g_trace.dsp_write)g_trace.dsp_write(g_trace.context,g_current_instruction_pc,address,value);}
extern "C" void sc_static_apu_trace_port_event(uint8_t direction,uint8_t port,uint8_t value){log_diag_port(g_current_instruction_pc,direction,port,value);if(g_trace.port)g_trace.port(g_trace.context,g_current_instruction_pc,direction,port,value);}
extern "C" void sc_static_sdsp_primitive_step(uint8_t phase){
  if(phase>=32u){g_sdsp_static_failed=1u;g_sdsp_fail_reason=1u;g_sdsp_fail_phase=phase;return;}
  ++g_sdsp_primitive_steps;g_sdsp_primitive_hash^=phase;g_sdsp_primitive_hash*=kFnvPrime;
}
extern "C" int sc_static_sdsp_brr_step(uint16_t address,const uint8_t *aram){
  if(!aram){g_sdsp_static_failed=1u;g_sdsp_fail_reason=2u;return 0;}
  ++g_sdsp_brr_steps;g_sdsp_brr_hash^=address;g_sdsp_brr_hash*=kFnvPrime;
  for(unsigned i=0;i<9u;++i){g_sdsp_brr_hash^=aram[(uint16_t)(address+i)];g_sdsp_brr_hash*=kFnvPrime;}
  return 1;
}
extern "C" void sc_static_sdsp_report_failure(uint32_t reason,uint8_t phase){
  if(!g_sdsp_static_failed){g_sdsp_static_failed=1u;g_sdsp_fail_reason=(uint8_t)(reason&0xffu);g_sdsp_fail_phase=phase;}
}
extern "C" int sc_static_apu_sync_to_master(uint64_t m,char *e,size_t c){return sync_impl(m,e,c);}
extern "C" int sc_static_apu_cpu_write_port(uint64_t m,unsigned p,uint8_t v,char *e,size_t c){
  if(p>=4u){copy_error(e,c,"Invalid static APUIO port.");return 0;}if(!sync_impl(m,e,c))return 0;
  SC_STATIC_SNES::cpu.port_write((uint8)p,v);record_cpu_port(1u,m,p,v);return 1;
}
extern "C" uint8_t sc_static_apu_cpu_read_port(uint64_t m,unsigned p,int *ok,char *e,size_t c){
  if(ok) *ok=0;if(p>=4u){copy_error(e,c,"Invalid static APUIO port.");return 0;}
  if(!sync_impl(m,e,c)) return 0;const uint8_t value=(uint8_t)SC_STATIC_SNES::smp.port_read(p);
  record_cpu_port(0u,m,p,value);if(ok) *ok=1;return value;
}
extern "C" int sc_static_apu_status(SCStaticApuStatus *s){
  if(!s||!g_acquired) return 0;std::memset(s,0,sizeof(*s));
  s->synchronized_master_clock=g_master;s->smp_cycles=g_cycles;
  s->smp_cycle_overshoot=SC_STATIC_SNES::smp.clock;s->smp_executed_cycles=g_cycles+(uint64_t)(SC_STATIC_SNES::smp.clock<0?0:SC_STATIC_SNES::smp.clock);
  s->smp_instructions=SC_STATIC_SNES::smp.instruction_count;s->aot_validated_instructions=SC_STATIC_SNES::smp.sc_aot_instructions;
  g_pcm_frames=SC_STATIC_SNES::dsp.core.pcm_frames_produced;
  s->pcm_frames=g_pcm_frames;s->smp_pc=SC_STATIC_SNES::smp.regs.pc;s->aot_fail_pc=SC_STATIC_SNES::smp.sc_aot_fail_pc_value;
  s->pcm_known_frames=SC_STATIC_SNES::dsp.core.pcm_known_frames_produced;
  s->pcm_unknown_frames=SC_STATIC_SNES::dsp.core.pcm_unknown_frames_produced;
  s->pcm_hash=SC_STATIC_SNES::dsp.core.pcm_fnv1a64;
  s->pcm_overflows=sc_static_apu_pcm_overflow_count();
  s->pcm_available=SC_STATIC_SNES::dsp.core.pcm_count;
  s->expected_opcode=SC_STATIC_SNES::smp.sc_aot_expected_opcode;s->actual_opcode=SC_STATIC_SNES::smp.sc_aot_actual_opcode;
  s->aot_failed=(uint8_t)(SC_STATIC_SNES::smp.sc_aot_failed_value!=0u);s->aot_fail_reason=(uint8_t)SC_STATIC_SNES::smp.sc_aot_fail_reason_value;
  s->current_epoch=(uint8_t)(SC_STATIC_SNES::smp.status.iplrom_enable?0u:1u);s->code_write_barriers=g_code_write_barriers;
  s->sdsp_primitive_steps=SC_STATIC_SNES::dsp.core.phase_steps;
  s->sdsp_primitive_hash=SC_STATIC_SNES::dsp.core.pcm_fnv1a64;
  for(unsigned voice=0;voice<CIV_DSP_VOICES;++voice)s->sdsp_brr_steps+=SC_STATIC_SNES::dsp.core.voices[voice].brr_groups_decoded;
  s->sdsp_brr_hash=SC_STATIC_SNES::dsp.core.pcm_fnv1a64;
  s->sdsp_static_failed=g_sdsp_static_failed;s->sdsp_fail_reason=g_sdsp_fail_reason;s->sdsp_fail_phase=g_sdsp_fail_phase;
  s->dsp_phase=(uint8_t)SC_STATIC_SNES::dsp.core.phase;s->timer_enable_mask=timer_enable_mask();
  store_timer(s->timer_target,s->timer_stage1,s->timer_stage2,s->timer_stage3,0u,SC_STATIC_SNES::smp.timer0);
  store_timer(s->timer_target,s->timer_stage1,s->timer_stage2,s->timer_stage3,1u,SC_STATIC_SNES::smp.timer1);
  store_timer(s->timer_target,s->timer_stage1,s->timer_stage2,s->timer_stage3,2u,SC_STATIC_SNES::smp.timer2);
  s->clock_ratio_numerator=SC_AUDIO_CLOCK_RATIO_NUMERATOR;s->clock_ratio_denominator=SC_AUDIO_CLOCK_RATIO_DENOMINATOR;s->clock_remainder=g_phase_remainder;
  s->maximum_sync_master_delta=g_max_sync_master_delta;s->first_sync_master_delta=g_first_sync_master_delta;s->maximum_followup_sync_master_delta=g_max_followup_sync_master_delta;s->sync_calls=g_sync_calls;s->rendezvous_hash=g_rendezvous_hash;
  s->cpu_port_event_count=g_cpu_port_event_count;s->cpu_port_event_hash=g_cpu_port_event_hash;s->initialized=1u;return 1;
}
extern "C" uint8_t sc_static_apu_peek_aram(uint16_t address){
  return g_acquired?SC_STATIC_SNES::smp.apuram[address]:0u;
}
extern "C" uint8_t sc_static_apu_peek_dsp(uint8_t address){
  if(!g_acquired)return 0u;
  SC_STATIC_SNES::dsp.synchronize();
  return SC_STATIC_SNES::dsp.core.regs[address&0x7fu];
}
extern "C" size_t sc_static_apu_pcm_available(void){return g_acquired?civ_dsp_pcm_available(&SC_STATIC_SNES::dsp.core):0u;}
extern "C" size_t sc_static_apu_pcm_read(int16_t *out,uint8_t *known,size_t capacity){
  if(!g_acquired||!out)return 0u;
  const size_t count=civ_dsp_pcm_read_with_knownness(&SC_STATIC_SNES::dsp.core,out,known,capacity);
  if(g_sink)for(size_t i=0;i<count;i++)g_sink(g_sink_context,out[i*2u],out[i*2u+1u]);
  return count;
}
extern "C" uint64_t sc_static_apu_pcm_overflow_count(void){return g_acquired?SC_STATIC_SNES::dsp.core.pcm_overflows:0u;}
extern "C" size_t sc_static_apu_snapshot_size(void){return sizeof(ApuSnapshotHeader)+sizeof(SC_STATIC_SNES::SMP)+65536u+8192u+sizeof(civ_dsp);}
extern "C" int sc_static_apu_snapshot_save(void *data,size_t capacity){
  ApuSnapshotHeader h{};unsigned char *out=(unsigned char*)data;civ_dsp dsp_state;
  if(!g_acquired||!data||capacity<sc_static_apu_snapshot_size())return 0;
  std::memcpy(h.magic,"SCAPU003",8u);h.version=3u;h.smp_size=(uint32_t)sizeof(SC_STATIC_SNES::SMP);h.aram_size=65536u;h.dsp_capacity=(uint32_t)sizeof(civ_dsp);
  std::memcpy(h.cpu_registers,SC_STATIC_SNES::cpu.registers,4u);h.dsp_frequency=SC_STATIC_SNES::dsp.frequency;h.dsp_clock=SC_STATIC_SNES::dsp.clock;
  h.master=g_master;h.cycles=g_cycles;h.pcm_frames=g_pcm_frames;h.sync_calls=g_sync_calls;h.rendezvous_hash=g_rendezvous_hash;
  h.cpu_port_event_count=g_cpu_port_event_count;h.cpu_port_event_hash=g_cpu_port_event_hash;h.phase_remainder=g_phase_remainder;
  h.max_sync_master_delta=g_max_sync_master_delta;h.first_sync_master_delta=g_first_sync_master_delta;h.max_followup_sync_master_delta=g_max_followup_sync_master_delta;
  h.current_instruction_pc=g_current_instruction_pc;h.code_write_barriers=g_code_write_barriers;
  h.sdsp_primitive_steps=g_sdsp_primitive_steps;h.sdsp_primitive_hash=g_sdsp_primitive_hash;h.sdsp_brr_steps=g_sdsp_brr_steps;h.sdsp_brr_hash=g_sdsp_brr_hash;
  h.sdsp_static_failed=g_sdsp_static_failed;h.sdsp_fail_reason=g_sdsp_fail_reason;h.sdsp_fail_phase=g_sdsp_fail_phase;h.program_started=g_program_started?1u:0u;
  std::memcpy(out,&h,sizeof(h));out+=sizeof(h);std::memcpy(out,&SC_STATIC_SNES::smp,sizeof(SC_STATIC_SNES::smp));out+=sizeof(SC_STATIC_SNES::smp);
  std::memcpy(out,SC_STATIC_SNES::smp.apuram,65536u);out+=65536u;
  std::memcpy(out,SC_STATIC_SNES::smp.aram_known,8192u);out+=8192u;
  dsp_state=SC_STATIC_SNES::dsp.core;dsp_state.aram=nullptr;dsp_state.aram_known=nullptr;
  std::memcpy(out,&dsp_state,sizeof(dsp_state));return 1;
}
extern "C" int sc_static_apu_snapshot_load(const void *data,size_t size,char *error,size_t error_capacity){
  ApuSnapshotHeader h;const unsigned char *in=(const unsigned char*)data;uint8 *apuram,*aram_known;
  if(!g_acquired||!data||size!=sc_static_apu_snapshot_size()){copy_error(error,error_capacity,"Static audio snapshot size is invalid.");return 0;}
  std::memcpy(&h,in,sizeof(h));in+=sizeof(h);
  if(std::memcmp(h.magic,"SCAPU003",8u)!=0||h.version!=3u||h.smp_size!=sizeof(SC_STATIC_SNES::SMP)||h.aram_size!=65536u||h.dsp_capacity!=sizeof(civ_dsp)){copy_error(error,error_capacity,"Static audio snapshot is from another build.");return 0;}
  apuram=SC_STATIC_SNES::smp.apuram;aram_known=SC_STATIC_SNES::smp.aram_known;
  std::memcpy(&SC_STATIC_SNES::smp,in,sizeof(SC_STATIC_SNES::smp));SC_STATIC_SNES::smp.apuram=apuram;SC_STATIC_SNES::smp.aram_known=aram_known;in+=sizeof(SC_STATIC_SNES::smp);
  std::memcpy(apuram,in,65536u);in+=65536u;std::memcpy(aram_known,in,8192u);in+=8192u;
  std::memcpy(SC_STATIC_SNES::cpu.registers,h.cpu_registers,4u);SC_STATIC_SNES::dsp.frequency=h.dsp_frequency;SC_STATIC_SNES::dsp.clock=h.dsp_clock;
  std::memcpy(&SC_STATIC_SNES::dsp.core,in,sizeof(SC_STATIC_SNES::dsp.core));SC_STATIC_SNES::dsp.core.aram=apuram;SC_STATIC_SNES::dsp.core.aram_known=aram_known;
  g_master=h.master;g_cycles=h.cycles;g_pcm_frames=h.pcm_frames;g_sync_calls=h.sync_calls;g_rendezvous_hash=h.rendezvous_hash;
  g_cpu_port_event_count=h.cpu_port_event_count;g_cpu_port_event_hash=h.cpu_port_event_hash;g_phase_remainder=h.phase_remainder;
  g_max_sync_master_delta=h.max_sync_master_delta;g_first_sync_master_delta=h.first_sync_master_delta;g_max_followup_sync_master_delta=h.max_followup_sync_master_delta;
  g_current_instruction_pc=(uint16_t)h.current_instruction_pc;g_code_write_barriers=h.code_write_barriers;
  g_sdsp_primitive_steps=h.sdsp_primitive_steps;g_sdsp_primitive_hash=h.sdsp_primitive_hash;g_sdsp_brr_steps=h.sdsp_brr_steps;g_sdsp_brr_hash=h.sdsp_brr_hash;
  g_sdsp_static_failed=h.sdsp_static_failed;g_sdsp_fail_reason=h.sdsp_fail_reason;g_sdsp_fail_phase=h.sdsp_fail_phase;g_program_started=h.program_started!=0u;
  copy_error(error,error_capacity,"");return 1;
}
