#include "civilization_audio.h"
#include "civilization_diagnostics.h"
#include "../static-audio/civilization-bapu-aot/sc_static_apu.h"

#include <cstdio>
#include <cstring>
#include <limits>

namespace {
constexpr uint64_t kFnvOffset=UINT64_C(14695981039346656037);
uint64_t v20_audio_target_master_clock(const CivRecomp *i) {
  if(!i)return 0ull;
  /* The production core has one clock authority. S-CPU bus/internal cycles, DRAM
     refresh, PPU events, S-SMP timers, and S-DSP output all rendezvous on this
     native NTSC master-clock timeline. */
  return i->master_clock;
}

constexpr uint64_t kFnvPrime=UINT64_C(1099511628211);
constexpr uint32_t kCaptureFrames=8192u;
constexpr uint32_t kWaveSampleRate=32040u;
/* The static S-SMP implementation is a process singleton.  g_owner is only
   its checked lease token; capture, metrics and callbacks remain per-core. */
CivRecomp *g_owner=nullptr;

uint64_t mix8(uint64_t h,uint8_t v){h^=v;return h*kFnvPrime;}
uint64_t mix16(uint64_t h,int16_t v){uint16_t u=(uint16_t)v;h=mix8(h,(uint8_t)u);h=mix8(h,(uint8_t)(u>>8));return h;}
void reset_metrics(CivRecomp *i){
  i->v20_pcm_frame_count=0u;i->v20_pcm_fnv1a64=kFnvOffset;i->v20_nonzero_pcm_frame_count=0u;
  i->v20_first_nonzero_pcm_frame=UINT64_MAX;i->v20_pcm_capture_frame_count=0u;i->v20_pcm_capture_fnv1a64=kFnvOffset;
  i->v20_dsp_write_count=0u;i->v20_dsp_write_fnv1a64=kFnvOffset;
  i->v20_aram_write_count=0u;i->v20_aram_write_fnv1a64=kFnvOffset;i->v20_smp_port_event_count=0u;i->v20_smp_port_event_fnv1a64=kFnvOffset;
  i->v20_pcm_capture_frame_count=0u;i->v20_pcm_capture_started=0u;
  std::memset(i->v20_pcm_capture,0,sizeof(i->v20_pcm_capture));
}
void pcm_sink(void *context,int16_t left,int16_t right){
  CivRecomp *i=static_cast<CivRecomp*>(context);if(!i)return;
  const uint64_t frame=i->v20_pcm_frame_count;
  i->v20_pcm_fnv1a64=mix16(mix16(i->v20_pcm_fnv1a64,left),right);
  if(left!=0 || right!=0){
    if(i->v20_nonzero_pcm_frame_count==0u){i->v20_first_nonzero_pcm_frame=frame;i->v20_pcm_capture_started=1u;}
    ++i->v20_nonzero_pcm_frame_count;
  }
  if(i->v20_pcm_capture_started && i->v20_pcm_capture_frame_count<kCaptureFrames){
    uint32_t capture_frame=i->v20_pcm_capture_frame_count;
    i->v20_pcm_capture[capture_frame*2u]=left;i->v20_pcm_capture[capture_frame*2u+1u]=right;
    i->v20_pcm_capture_fnv1a64=mix16(mix16(i->v20_pcm_capture_fnv1a64,left),right);
    ++i->v20_pcm_capture_frame_count;
  }
  ++i->v20_pcm_frame_count;
  if(i->host_hooks.pcm)i->host_hooks.pcm(i->host_hooks.context,left,right);
}
void instruction_trace(void *,uint16_t,uint8_t){}
int aram_trace(void *context,uint16_t pc,uint16_t address,uint8_t value){
  CivRecomp *i=static_cast<CivRecomp*>(context);if(!i)return 0;
  uint64_t h=i->v20_aram_write_fnv1a64;h=mix16(h,(int16_t)pc);h=mix16(h,(int16_t)address);h=mix8(h,value);
  i->v20_aram_write_fnv1a64=h;++i->v20_aram_write_count;return 1;
}
void dsp_trace(void *context,uint16_t pc,uint8_t address,uint8_t value){
  CivRecomp *i=static_cast<CivRecomp*>(context);if(!i)return;
  uint64_t h=i->v20_dsp_write_fnv1a64;h=mix16(h,(int16_t)pc);h=mix8(h,address);h=mix8(h,value);
  i->v20_dsp_write_fnv1a64=h;++i->v20_dsp_write_count;
}
void port_trace(void *context,uint16_t pc,uint8_t direction,uint8_t port,uint8_t value){
  CivRecomp *i=static_cast<CivRecomp*>(context);if(!i)return;
  uint64_t h=i->v20_smp_port_event_fnv1a64;h=mix16(h,(int16_t)pc);h=mix8(h,direction);h=mix8(h,port);h=mix8(h,value);
  i->v20_smp_port_event_fnv1a64=h;++i->v20_smp_port_event_count;
}
int copy_status(CivRecomp *i){
  SCStaticApuStatus s{};if(!i||!sc_static_apu_status(&s))return 0;
  i->v20_static_smp_instructions=s.smp_instructions;i->v20_static_smp_aot_instructions=s.aot_validated_instructions;
  i->v20_static_smp_pc=s.smp_pc;i->v20_static_aot_failed=s.aot_failed;i->v20_static_aot_fail_pc=s.aot_fail_pc;
  i->v20_static_aot_expected_opcode=s.expected_opcode;i->v20_static_aot_actual_opcode=s.actual_opcode;i->v20_static_code_write_barriers=s.code_write_barriers;
  i->v20_sdsp_primitive_steps=s.sdsp_primitive_steps;i->v20_sdsp_brr_steps=s.sdsp_brr_steps;
  return 1;
}
int fail_audio(CivRecomp *i,const char *prefix,const char *detail){
  char text[192];
  std::snprintf(text,sizeof(text),"%.80s%s%.100s",
                prefix?prefix:"Full Static audio failed",
                detail&&*detail?": ":"",detail&&*detail?detail:"");
  if(!i)return 0;
  i->v20_full_static_audio_failed=1u;
  i->failed=1;
  std::snprintf(i->frontier_address,sizeof(i->frontier_address),"%02X:%04X",(unsigned)i->cpu.pbr,(unsigned)i->cpu.pc);
  std::snprintf(i->frontier_reason,sizeof(i->frontier_reason),"%.191s",text);
  return 0;
}
#pragma pack(push,1)
struct V20AudioStateHeader { char magic[8];uint32_t version;uint32_t apu_bytes;uint32_t capture_frames;uint8_t capture_started;uint8_t reserved[3];
  uint64_t pcm_frames,pcm_hash,nonzero_frames,first_nonzero,capture_hash,dsp_hash,aram_hash,port_hash;uint32_t dsp_writes,aram_writes,port_events; };
#pragma pack(pop)
}

extern "C" int civ_v20_audio_begin(CivRecomp *i){
  char error[192];if(!i)return 0;
  if(g_owner && g_owner!=i){g_owner->v20_full_static_audio_acquired=0u;sc_static_apu_release();g_owner=nullptr;}
  if(g_owner==i && i->v20_full_static_audio_acquired)return 1;
  sc_static_apu_release();
  if(!sc_static_apu_acquire(error,sizeof(error)))return fail_audio(i,"Could not acquire Full Static audio",error);
  g_owner=i;i->v20_full_static_audio_enabled=1u;i->v20_full_static_audio_acquired=1u;i->v20_full_static_audio_failed=0u;
  reset_metrics(i);
  SCStaticTraceCallbacks traces{instruction_trace,aram_trace,dsp_trace,port_trace,i};
  sc_static_apu_set_sink(pcm_sink,i);sc_static_apu_set_trace_callbacks(&traces);
  return copy_status(i);
}
extern "C" int civ_v20_audio_sync_internal(CivRecomp *i){
  char error[192];if(!i||!i->v20_full_static_audio_enabled)return 1;
  if(!sc_static_apu_sync_to_master(v20_audio_target_master_clock(i),error,sizeof(error)))return fail_audio(i,"Full Static audio synchronization failed",error);
  return copy_status(i);
}
extern "C" int civ_v20_audio_cpu_write(CivRecomp *i,unsigned port,uint8_t value){
  char error[192];if(!i)return 0;if(!i->v20_full_static_audio_enabled)return fail_audio(i,"Production Full Static APUIO write before audio acquisition","");
  if(!sc_static_apu_cpu_write_port(v20_audio_target_master_clock(i),port,value,error,sizeof(error)))return fail_audio(i,"Full Static APUIO write failed",error);
  return copy_status(i);
}
extern "C" int civ_v20_audio_cpu_read(CivRecomp *i,unsigned port,uint8_t *value){
  char error[192];int ok=0;if(!i||!value)return 0;if(!i->v20_full_static_audio_enabled)return fail_audio(i,"Production Full Static APUIO read before audio acquisition","");
  *value=sc_static_apu_cpu_read_port(v20_audio_target_master_clock(i),port,&ok,error,sizeof(error));
  if(!ok)return fail_audio(i,"Full Static APUIO read failed",error);
  return copy_status(i);
}
extern "C" void civ_v20_audio_release_internal(CivRecomp *i){if(g_owner==i){sc_static_apu_release();g_owner=nullptr;}if(i)i->v20_full_static_audio_acquired=0u;}
extern "C" uint8_t civ_v20_audio_peek_aram_internal(uint16_t a){return sc_static_apu_peek_aram(a);}
extern "C" uint8_t civ_v20_audio_peek_dsp_internal(uint8_t a){return sc_static_apu_peek_dsp(a);}
extern "C" size_t civ_v20_audio_state_size_internal(void){return sizeof(V20AudioStateHeader)+sc_static_apu_snapshot_size()+kCaptureFrames*2u*sizeof(int16_t);}
extern "C" int civ_v20_audio_state_save_internal(const CivRecomp *i,void *data,size_t capacity){
  if(!i||g_owner!=i||!data||capacity<civ_v20_audio_state_size_internal())return 0;
  V20AudioStateHeader h{};std::memcpy(h.magic,"CVAUD20",7u);h.version=1u;h.apu_bytes=(uint32_t)sc_static_apu_snapshot_size();h.capture_frames=i->v20_pcm_capture_frame_count;h.capture_started=i->v20_pcm_capture_started;
  h.pcm_frames=i->v20_pcm_frame_count;h.pcm_hash=i->v20_pcm_fnv1a64;h.nonzero_frames=i->v20_nonzero_pcm_frame_count;h.first_nonzero=i->v20_first_nonzero_pcm_frame;h.capture_hash=i->v20_pcm_capture_fnv1a64;
  h.dsp_hash=i->v20_dsp_write_fnv1a64;h.aram_hash=i->v20_aram_write_fnv1a64;h.port_hash=i->v20_smp_port_event_fnv1a64;h.dsp_writes=i->v20_dsp_write_count;h.aram_writes=i->v20_aram_write_count;h.port_events=i->v20_smp_port_event_count;
  auto *out=static_cast<uint8_t*>(data);std::memcpy(out,&h,sizeof(h));out+=sizeof(h);if(!sc_static_apu_snapshot_save(out,h.apu_bytes))return 0;out+=h.apu_bytes;std::memcpy(out,i->v20_pcm_capture,sizeof(i->v20_pcm_capture));return 1;
}
extern "C" int civ_v20_audio_state_load_internal(CivRecomp *i,const void *data,size_t size){
  char error[192];if(!i||g_owner!=i||!data||size!=civ_v20_audio_state_size_internal())return 0;V20AudioStateHeader h{};const auto *in=static_cast<const uint8_t*>(data);std::memcpy(&h,in,sizeof(h));in+=sizeof(h);
  if(std::memcmp(h.magic,"CVAUD20",7u)!=0||h.version!=1u||h.apu_bytes!=sc_static_apu_snapshot_size()||h.capture_frames>kCaptureFrames)return 0;
  if(!sc_static_apu_snapshot_load(in,h.apu_bytes,error,sizeof(error)))
    return fail_audio(i,"Full Static audio snapshot load failed",error);
  in+=h.apu_bytes;
  std::memcpy(i->v20_pcm_capture,in,sizeof(i->v20_pcm_capture));
  i->v20_pcm_capture_started=h.capture_started!=0u;
  i->v20_pcm_frame_count=h.pcm_frames;i->v20_pcm_fnv1a64=h.pcm_hash;i->v20_nonzero_pcm_frame_count=h.nonzero_frames;i->v20_first_nonzero_pcm_frame=h.first_nonzero;i->v20_pcm_capture_frame_count=h.capture_frames;i->v20_pcm_capture_fnv1a64=h.capture_hash;
  i->v20_dsp_write_count=h.dsp_writes;i->v20_dsp_write_fnv1a64=h.dsp_hash;i->v20_aram_write_count=h.aram_writes;i->v20_aram_write_fnv1a64=h.aram_hash;i->v20_smp_port_event_count=h.port_events;i->v20_smp_port_event_fnv1a64=h.port_hash;return copy_status(i);
}

extern "C" int civ_v20_audio_sync(CivRecomp *i){return civ_v20_audio_sync_internal(i);}
extern "C" int civ_v20_get_audio_status(const CivRecomp *i,CivV20AudioStatus *out){
  SCStaticApuStatus s{};if(!i||!out||g_owner!=i||!sc_static_apu_status(&s))return 0;std::memset(out,0,sizeof(*out));
  out->synchronized_master_clock=s.synchronized_master_clock;out->smp_cycles=s.smp_cycles;out->smp_instructions=s.smp_instructions;out->aot_validated_instructions=s.aot_validated_instructions;out->pcm_frames=i->v20_pcm_frame_count;out->pcm_fnv1a64=i->v20_pcm_fnv1a64;out->nonzero_pcm_frames=i->v20_nonzero_pcm_frame_count;out->first_nonzero_pcm_frame=i->v20_first_nonzero_pcm_frame;out->capture_frames=i->v20_pcm_capture_frame_count;out->capture_fnv1a64=i->v20_pcm_capture_fnv1a64;out->smp_pc=s.smp_pc;out->aot_fail_pc=s.aot_fail_pc;out->aot_failed=s.aot_failed;out->expected_opcode=s.expected_opcode;out->actual_opcode=s.actual_opcode;out->code_write_barriers=s.code_write_barriers;out->sdsp_primitive_steps=s.sdsp_primitive_steps;out->sdsp_brr_steps=s.sdsp_brr_steps;out->dsp_phase=s.dsp_phase;out->timer_enable_mask=s.timer_enable_mask;out->dsp_write_count=i->v20_dsp_write_count;out->dsp_write_fnv1a64=i->v20_dsp_write_fnv1a64;out->aram_write_count=i->v20_aram_write_count;out->aram_write_fnv1a64=i->v20_aram_write_fnv1a64;out->smp_port_event_count=i->v20_smp_port_event_count;out->smp_port_event_fnv1a64=i->v20_smp_port_event_fnv1a64;return 1;
}
extern "C" size_t civ_v20_audio_state_size(void){return civ_v20_audio_state_size_internal();}
extern "C" int civ_v20_audio_state_save(const CivRecomp *i,void *d,size_t c){return civ_v20_audio_state_save_internal(i,d,c);}
extern "C" int civ_v20_audio_state_load(CivRecomp *i,const void *d,size_t s){return civ_v20_audio_state_load_internal(i,d,s);}
extern "C" int civ_v20_copy_pcm_capture(const CivRecomp *i,int16_t *out,size_t cap,size_t *frames_written){
  if(frames_written)*frames_written=0u;
  if(!i||g_owner!=i||!out||cap<i->v20_pcm_capture_frame_count)return 0;
  std::memcpy(out,i->v20_pcm_capture,i->v20_pcm_capture_frame_count*2u*sizeof(int16_t));
  if(frames_written)*frames_written=i->v20_pcm_capture_frame_count;
  return 1;
}
extern "C" void civ_v20_set_host_pcm_sink(CivRecomp *i,CivHostPcmSink sink,void *context){
  if(!i)return;
  i->host_hooks.pcm=sink;
  i->host_hooks.context=context;
}
extern "C" int civ_v20_audio_peek_dsp(const CivRecomp *i,uint8_t address,uint8_t *value){
  if(!i||g_owner!=i||!value)return 0;
  *value=sc_static_apu_peek_dsp((uint8_t)(address&0x7fu));
  return 1;
}
extern "C" int civ_v20_audio_peek_aram(const CivRecomp *i,uint16_t address,uint8_t *value){
  if(!i||g_owner!=i||!value)return 0;
  *value=sc_static_apu_peek_aram(address);
  return 1;
}
extern "C" int civ_v20_write_wav(const CivRecomp *i,const char *path){
  if(!i||g_owner!=i||!path||i->v20_pcm_capture_frame_count==0u)return 0;
  FILE *f=std::fopen(path,"wb");
  if(!f)return 0;
  const uint32_t data_bytes=i->v20_pcm_capture_frame_count*4u;
  const uint32_t riff_size=36u+data_bytes;
  const uint16_t format=1u,channels=2u,bits=16u,block=4u;
  const uint32_t byte_rate=kWaveSampleRate*block;
  auto w16=[&](uint16_t v){std::fputc((int)(v&0xffu),f);std::fputc((int)(v>>8),f);};auto w32=[&](uint32_t v){w16((uint16_t)v);w16((uint16_t)(v>>16));};
  std::fwrite("RIFF",1,4,f);w32(riff_size);std::fwrite("WAVEfmt ",1,8,f);w32(16u);w16(format);w16(channels);w32(kWaveSampleRate);w32(byte_rate);w16(block);w16(bits);std::fwrite("data",1,4,f);w32(data_bytes);
  for(uint32_t n=0;n<i->v20_pcm_capture_frame_count*2u;n++)w16((uint16_t)i->v20_pcm_capture[n]);
  const int ok=std::ferror(f)==0?1:0;
  std::fclose(f);
  return ok;
}
