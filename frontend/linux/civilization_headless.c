#include "civilization_diagnostics.h"
#include "civilization_frontend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *load_file(const char *path,size_t *size)
{
    FILE *file=fopen(path,"rb");long length;unsigned char *buffer;
    if(!file)return NULL;
    if(fseek(file,0,SEEK_END)||(length=ftell(file))<0||fseek(file,0,SEEK_SET)){fclose(file);return NULL;}
    buffer=(unsigned char *)malloc((size_t)length);
    if(!buffer){fclose(file);return NULL;}
    if(fread(buffer,1,(size_t)length,file)!=(size_t)length){free(buffer);fclose(file);return NULL;}
    fclose(file);*size=(size_t)length;return buffer;
}

static char *arguments_after_command(char *line,const char *command)
{
    char *p=line+strlen(command);
    while(*p==' '||*p=='\t')++p;
    p[strcspn(p,"\r\n")]='\0';
    return p;
}

static uint16_t snes_mask_to_serial(uint16_t snes_mask)
{
    uint16_t serial=0u;unsigned bit;
    for(bit=0u;bit<16u;++bit)serial|=(uint16_t)(((snes_mask>>bit)&1u)<<(15u-bit));
    return serial;
}

static uint16_t wram_word(const CivRecomp *core,unsigned offset)
{
    uint8_t bytes[2];
    if(!civ_diagnostics_read_wram(core,offset,bytes,sizeof(bytes)))return 0u;
    return (uint16_t)(bytes[0]|((uint16_t)bytes[1]<<8));
}

static uint64_t frame_instruction_budget(uint64_t frames)
{
    const uint64_t minimum=UINT64_C(60000000);
    const uint64_t instructions_per_frame=UINT64_C(20000);
    uint64_t proportional;
    if(frames>UINT64_MAX/instructions_per_frame)return UINT64_MAX;
    proportional=frames*instructions_per_frame;
    return proportional>minimum?proportional:minimum;
}

static void print_status(const CivFrontend *f)
{
    CivDiagnosticState d;
    if(!civ_diagnostics_capture(f->core,&d)){puts("status unavailable");return;}
    printf("status paused=%d instr=%llu pc=%02X:%04X e=%u p=%02X a=%04X x=%04X y=%04X s=%04X d=%04X dbr=%02X failed=%d frontier=%s reason=%s input1=%04X frame=%llu master=%llu scanline=%u hclock=%u field=%u natural=%u code=%llu data=%llu internal=%llu refresh=%llu/%llu blank=%u brightness=%u mode=%u main=%02X sub=%02X\n",
           f->paused,(unsigned long long)d.instruction_count,d.cpu.pbr,d.cpu.pc,
           d.cpu.e,d.cpu.p,d.cpu.a,d.cpu.x,d.cpu.y,d.cpu.s,d.cpu.d,d.cpu.dbr,
           d.failed,d.frontier_address,d.frontier_reason,f->controller1,
           (unsigned long long)d.frame_count,(unsigned long long)d.master_clock,
           d.vcounter,d.hcounter,d.field,d.natural_timing_enabled,
           (unsigned long long)d.cpu_code_access_count,
           (unsigned long long)d.cpu_data_access_count,
           (unsigned long long)d.cpu_internal_cycle_count,
           (unsigned long long)d.dram_refresh_count,
           (unsigned long long)d.dram_refresh_master_clocks,
           (unsigned)d.ppu.forced_blank,(unsigned)d.ppu.brightness,
           d.ppu.bg_mode,d.ppu.main_screen_layers,d.ppu.sub_screen_layers);
}

static void print_input_status(const CivFrontend *f)
{
    CivDiagnosticState d;uint8_t flags[2]={0u,0u};
    if(!civ_diagnostics_capture(f->core,&d)){puts("input-state unavailable");return;}
    (void)civ_diagnostics_read_wram(f->core,0x0663u,flags,sizeof(flags));
    printf("input-state host-serial=%04X device=%u live=%04X latched=%04X shift=%u auto4218=%04X guest-current=%04X guest-old=%04X guest-edge=%04X manual4016=%u manual4017=%u auto-serial1=%u auto-serial2=%u auto-polls=%u mouse-probes=%u w0663=%02X w0664=%02X\n",
           f->controller1,d.controller_device[0],d.controller_live[0],
           d.controller_latched[0],d.controller_shift_index[0],d.auto_joypad_data[0],
           wram_word(f->core,0x0037u),wram_word(f->core,0x003bu),wram_word(f->core,0x003du),
           d.controller_serial_read_count[0],d.controller_serial_read_count[1],
           d.auto_joypad_serial_read_count[0],d.auto_joypad_serial_read_count[1],
           d.auto_joypad_poll_count,d.mouse_bios_probe_count,flags[0],flags[1]);
}

static void print_audio_status(const CivFrontend *f)
{
    CivV20AudioStatus a;
    if(!civ_v20_get_audio_status(f->core,&a)){puts("audio-status ready=0");return;}
    printf("audio-status ready=1 sync-master=%llu core-master=%llu delta=%lld smp-pc=%04X smp-cycles=%llu smp-instr=%llu aot=%llu pcm=%llu nonzero=%llu first-nonzero=%llu hash=%016llX capture=%u capture-hash=%016llX aot-failed=%u barriers=%u\n",
           (unsigned long long)a.synchronized_master_clock,
           (unsigned long long)civ_master_clock(f->core),
           (long long)a.synchronized_master_clock-(long long)civ_master_clock(f->core),
           a.smp_pc,(unsigned long long)a.smp_cycles,(unsigned long long)a.smp_instructions,
           (unsigned long long)a.aot_validated_instructions,(unsigned long long)a.pcm_frames,
           (unsigned long long)a.nonzero_pcm_frames,(unsigned long long)a.first_nonzero_pcm_frame,
           (unsigned long long)a.pcm_fnv1a64,a.capture_frames,
           (unsigned long long)a.capture_fnv1a64,a.aot_failed,a.code_write_barriers);
}

static int save_ppm(const char *path,const uint32_t *framebuffer)
{
    FILE *out;unsigned n;
    if(!path||!*path||!framebuffer)return 0;
    out=fopen(path,"wb");if(!out)return 0;
    if(fprintf(out,"P6\n%u %u\n255\n",CIV_FRAME_WIDTH,CIV_FRAME_HEIGHT)<0){fclose(out);return 0;}
    for(n=0;n<CIV_FRAME_PIXELS;++n){
        unsigned char rgb[3]={(unsigned char)(framebuffer[n]>>16),(unsigned char)(framebuffer[n]>>8),(unsigned char)framebuffer[n]};
        if(fwrite(rgb,1,3,out)!=3u){fclose(out);return 0;}
    }
    return fclose(out)==0;
}

static int trace_to_frame(CivFrontend *frontend,uint64_t target,const char *path)
{
    FILE *out;
    uint64_t rows=0u;
    if(!frontend||!path||!*path||target<=civ_frame_count(frontend->core))return 0;
    out=fopen(path,"w");
    if(!out)return 0;
    fputs("row,instruction,frame,master,scanline,hclock,pbr,pc,e,p,a,x,y,s,d,dbr,reg2100,w0037,w003b,w003d\n",out);
    while(civ_frame_count(frontend->core)<target&&!civ_has_failed(frontend->core)&&rows<UINT64_C(5000000)) {
        CivDiagnosticState d;if(!civ_diagnostics_capture(frontend->core,&d))break;
        if(fprintf(out,"%llu,%llu,%llu,%llu,%u,%u,%02X,%04X,%u,%02X,%04X,%04X,%04X,%04X,%04X,%02X,%02X,%04X,%04X,%04X\n",
                   (unsigned long long)rows,(unsigned long long)d.instruction_count,
                   (unsigned long long)d.frame_count,(unsigned long long)d.master_clock,
                   d.vcounter,d.hcounter,d.cpu.pbr,d.cpu.pc,d.cpu.e,d.cpu.p,
                   d.cpu.a,d.cpu.x,d.cpu.y,d.cpu.s,d.cpu.d,d.cpu.dbr,d.ppu_regs[0],
                   wram_word(frontend->core,0x0037u),wram_word(frontend->core,0x003bu),wram_word(frontend->core,0x003du))<0) {
            fclose(out);return 0;
        }
        if(!civ_frontend_run(frontend,1u))break;
        ++rows;
    }
    if(fclose(out)!=0)return 0;
    if(civ_audio_active(frontend->core)&&!civ_v20_audio_sync(frontend->core))return 0;
    return !civ_has_failed(frontend->core)&&civ_frame_count(frontend->core)>=target;
}

static int trace_pc_to_frame(CivFrontend *frontend,uint64_t target,uint8_t pbr,uint16_t pc,const char *path)
{
    FILE *out;
    uint64_t matches=0u,steps=0u;
    if(!frontend||!path||!*path||target<=civ_frame_count(frontend->core))return 0;
    out=fopen(path,"w");
    if(!out)return 0;
    fputs("match,instruction,frame,master,scanline,hclock,pbr,pc,e,p,a,x,y,s,d,dbr,w002f,w0031\n",out);
    while(civ_frame_count(frontend->core)<target&&!civ_has_failed(frontend->core)&&steps<UINT64_C(60000000)) {
        CivDiagnosticState d;if(!civ_diagnostics_capture(frontend->core,&d))break;
        if(d.cpu.pbr==pbr&&d.cpu.pc==pc) {
            if(fprintf(out,"%llu,%llu,%llu,%llu,%u,%u,%02X,%04X,%u,%02X,%04X,%04X,%04X,%04X,%04X,%02X,%04X,%04X\n",
                       (unsigned long long)matches,(unsigned long long)d.instruction_count,
                       (unsigned long long)d.frame_count,(unsigned long long)d.master_clock,
                       d.vcounter,d.hcounter,d.cpu.pbr,d.cpu.pc,d.cpu.e,d.cpu.p,
                       d.cpu.a,d.cpu.x,d.cpu.y,d.cpu.s,d.cpu.d,d.cpu.dbr,
                       wram_word(frontend->core,0x002fu),wram_word(frontend->core,0x0031u))<0) {
                fclose(out);return 0;
            }
            matches++;
        }
        if(!civ_frontend_run(frontend,1u))break;
        steps++;
    }
    if(fclose(out)!=0)return 0;
    if(civ_audio_active(frontend->core)&&!civ_v20_audio_sync(frontend->core))return 0;
    return !civ_has_failed(frontend->core)&&civ_frame_count(frontend->core)>=target;
}

static void print_ppu_state(const CivFrontend *frontend)
{
    CivDiagnosticState d;const CivDiagnosticPpuState *p;
    if(!civ_diagnostics_capture(frontend->core,&d)){puts("ppu-state unavailable");return;}
    p=&d.ppu;
    printf("ppu-state mode=%u bg3hi=%u large=%u%u%u%u map=%04X,%04X,%04X,%04X size=%u%u,%u%u,%u%u,%u%u chr=%04X,%04X,%04X,%04X scroll=%04X,%04X;%04X,%04X;%04X,%04X;%04X,%04X main=%02X sub=%02X win=%02X/%02X math=%02X cgwsel=%02X fixed=%04X obj=%u/%04X rotate=%u\n",
        p->bg_mode,p->mode1_bg3_priority,p->bg_large_tiles[0],p->bg_large_tiles[1],p->bg_large_tiles[2],p->bg_large_tiles[3],
        p->bg_tilemap_address[0],p->bg_tilemap_address[1],p->bg_tilemap_address[2],p->bg_tilemap_address[3],
        p->bg_double_width[0],p->bg_double_height[0],p->bg_double_width[1],p->bg_double_height[1],
        p->bg_double_width[2],p->bg_double_height[2],p->bg_double_width[3],p->bg_double_height[3],
        p->bg_chr_address[0],p->bg_chr_address[1],p->bg_chr_address[2],p->bg_chr_address[3],
        p->bg_hscroll[0],p->bg_vscroll[0],p->bg_hscroll[1],p->bg_vscroll[1],p->bg_hscroll[2],p->bg_vscroll[2],p->bg_hscroll[3],p->bg_vscroll[3],
        p->main_screen_layers,p->sub_screen_layers,p->window_mask_main,p->window_mask_sub,
        p->color_math_enabled,d.ppu_regs[0x30u],p->fixed_color,p->oam_mode,p->oam_base_address,p->oam_priority_rotation);
}

static void print_frame_hash(CivFrontend *frontend)
{
    CivDiagnosticState d;int ok=civ_render_current_frame(frontend->core);
    if(!civ_diagnostics_capture(frontend->core,&d)){puts("frame-hash unavailable");return;}
    if(!ok)printf("frame-hash ok=0 mode=%u blank=%u frame=%llu\n",d.ppu.bg_mode,d.ppu.forced_blank,(unsigned long long)d.frame_count);
    else printf("frame-hash ok=1 hash=%016llX frame=%llu\n",(unsigned long long)d.framebuffer_fnv1a64,(unsigned long long)d.frame_count);
}

static void take_screenshot(CivFrontend *frontend,const char *path)
{
    CivDiagnosticState d;const uint32_t *fb;
    if(!civ_render_current_frame(frontend->core)||!civ_diagnostics_capture(frontend->core,&d)){
        if(civ_diagnostics_capture(frontend->core,&d))printf("screenshot-current ok=0 mode=%u blank=%u\n",d.ppu.bg_mode,d.ppu.forced_blank);
        else puts("screenshot-current ok=0 diagnostics-unavailable");
        return;
    }
    fb=civ_get_framebuffer_rgba(frontend->core);
    printf("screenshot-current ok=%d path=%s hash=%016llX frame=%llu\n",save_ppm(path,fb),path,
           (unsigned long long)d.framebuffer_fnv1a64,(unsigned long long)d.frame_count);
}

static void print_wram(const CivFrontend *frontend,unsigned offset,unsigned count)
{
    uint8_t bytes[256];unsigned done=0u,index;
    printf("wram %05X",offset);
    while(done<count){
        unsigned chunk=count-done;if(chunk>sizeof(bytes))chunk=(unsigned)sizeof(bytes);
        if(!civ_diagnostics_read_wram(frontend->core,offset+done,bytes,chunk)){puts(" unavailable");return;}
        for(index=0u;index<chunk;++index)printf(" %02X",bytes[index]);
        done+=chunk;
    }
    putchar('\n');
}

static void print_help(void)
{
    puts("Civilization headless commands: help, status, checkpoint, play, pause, reset, run INSTRUCTIONS, frame-run FRAME, frame-step FRAMES, tap-snes SNES_MASK FRAMES, trace-frame FRAME PATH.csv, trace-pc FRAME PBR PC PATH.csv, input SERIAL_MASK, input-snes SNES_MASK, input-status, mouse PORT DX DY BUTTONS, video-state, ppu-state, frame-hash, screenshot PATH.ppm, screenshot-current PATH.ppm, audio-status, record-audio PATH.wav, peek-wram OFFSET COUNT, state-dir PATH, sram-status, sram-save, sram-load, snapshot-save SLOT, snapshot-load SLOT, snapshot-path SLOT, quit");
    puts("snapshot slots are persistent and 1-based (1-5); state root defaults to the current directory or CIVILIZATION_STATE_DIR.");
    puts("serial mask: B=0x0001 Y=0x0002 Select=0x0004 Start=0x0008 Up=0x0010 Down=0x0020 Left=0x0040 Right=0x0080 A=0x0100 X=0x0200 L=0x0400 R=0x0800");
    puts("SNES mask: B=0x8000 Y=0x4000 Select=0x2000 Start=0x1000 Up=0x0800 Down=0x0400 Left=0x0200 Right=0x0100 A=0x0080 X=0x0040 L=0x0020 R=0x0010");
}

int main(int argc,char **argv)
{
    unsigned char *rom;size_t rom_size;static CivFrontend frontend;
    char error[192],line[4096];FILE *input=stdin;int exit_code=0;
    if(argc<2||argc>3){fprintf(stderr,"usage: %s ROM [script]\n",argv[0]);return 2;}
    rom=load_file(argv[1],&rom_size);if(!rom)return 3;
    civ_frontend_init_empty(&frontend);
    if(!frontend.core){fprintf(stderr,"Core allocation failed: %s\n",civ_frontend_last_error(&frontend));civ_frontend_shutdown(&frontend);free(rom);return 4;}
    if(!civ_frontend_load_rom(&frontend,rom,rom_size,error,sizeof(error))){fprintf(stderr,"ROM rejected: %s\n",error);civ_frontend_shutdown(&frontend);free(rom);return 4;}
    {
        const char *state_dir=getenv("CIVILIZATION_STATE_DIR");
        if(!state_dir||!*state_dir)state_dir=".";
        if(!civ_frontend_set_state_directory(&frontend,state_dir,error,sizeof(error))||
           !civ_frontend_load_persistent_sram(&frontend)){
            fprintf(stderr,"Persistence setup failed: %s\n",civ_frontend_last_error(&frontend));civ_frontend_shutdown(&frontend);free(rom);return 6;
        }
    }
    if(argc==3){input=fopen(argv[2],"r");if(!input){civ_frontend_shutdown(&frontend);free(rom);return 5;}}
    while(fgets(line,sizeof(line),input)){
        char command[64],*arguments;unsigned long long value=0;
        if(sscanf(line,"%63s",command)!=1||command[0]=='#')continue;
        arguments=arguments_after_command(line,command);
        if(!strcmp(command,"help"))print_help();
        else if(!strcmp(command,"status"))print_status(&frontend);
        else if(!strcmp(command,"checkpoint")){print_status(&frontend);print_input_status(&frontend);{CivVideoCheckpoint v;civ_capture_video_checkpoint(frontend.core,&v);printf("video-state forced-blank=%u brightness=%u mode=%u main=%02X sub=%02X frame=%llu master=%llu vram=%016llX cgram=%016llX oam=%016llX dma=%llu/%llu writes=%llu/%llu/%llu\n",v.forced_blank,v.brightness,v.bg_mode,v.main_screen_layers,v.sub_screen_layers,(unsigned long long)v.frame_count,(unsigned long long)v.master_clock,(unsigned long long)v.vram_fnv1a64,(unsigned long long)v.cgram_fnv1a64,(unsigned long long)v.oam_fnv1a64,(unsigned long long)v.dma_run_count,(unsigned long long)v.dma_transfer_byte_count,(unsigned long long)v.vram_data_write_count,(unsigned long long)v.cgram_data_write_count,(unsigned long long)v.oam_data_write_count);}print_audio_status(&frontend);}
        else if(!strcmp(command,"play")){civ_frontend_play(&frontend);puts("ok play");}
        else if(!strcmp(command,"pause")){civ_frontend_pause(&frontend);puts("ok pause");}
        else if(!strcmp(command,"reset")){if(!civ_frontend_reset(&frontend,error,sizeof(error))){printf("error reset %s\n",error);exit_code=1;}else puts("ok reset");}
        else if(!strcmp(command,"run")&&sscanf(arguments,"%llu",&value)==1){int ok=civ_frontend_run(&frontend,(uint64_t)value);printf("run ok=%d instr=%llu frame=%llu frontier=%s reason=%s\n",ok,(unsigned long long)civ_instruction_count(frontend.core),(unsigned long long)civ_frame_count(frontend.core),civ_frontier_address(frontend.core),civ_frontier_reason(frontend.core));if(!ok)exit_code=1;}
        else if(!strcmp(command,"frame-run")&&sscanf(arguments,"%llu",&value)==1){uint64_t target=(uint64_t)value;uint64_t delta=target>civ_frame_count(frontend.core)?target-civ_frame_count(frontend.core):0u;int ok=civ_frontend_run_to_frame(&frontend,target,frame_instruction_budget(delta));printf("frame-run ok=%d requested=%llu reached=%llu frontier=%s reason=%s\n",ok,value,(unsigned long long)civ_frame_count(frontend.core),civ_frontier_address(frontend.core),civ_frontier_reason(frontend.core));if(!ok)exit_code=1;}
        else if(!strcmp(command,"frame-step")&&sscanf(arguments,"%llu",&value)==1){uint64_t target=civ_frame_count(frontend.core)+(uint64_t)value;int ok=target>=civ_frame_count(frontend.core)&&civ_frontend_run_to_frame(&frontend,target,frame_instruction_budget((uint64_t)value));printf("frame-step ok=%d delta=%llu reached=%llu frontier=%s reason=%s\n",ok,value,(unsigned long long)civ_frame_count(frontend.core),civ_frontier_address(frontend.core),civ_frontier_reason(frontend.core));if(!ok)exit_code=1;}
        else if(!strcmp(command,"tap-snes")){unsigned mask;unsigned long long frames;if(sscanf(arguments,"%i %llu",(int *)&mask,&frames)!=2)puts("error usage tap-snes SNES_MASK FRAMES");else{uint64_t target=civ_frame_count(frontend.core)+(uint64_t)frames;int ok;frontend.controller1=snes_mask_to_serial((uint16_t)mask);civ_set_controller_input(frontend.core,0u,frontend.controller1);ok=target>=civ_frame_count(frontend.core)&&civ_frontend_run_to_frame(&frontend,target,frame_instruction_budget((uint64_t)frames));frontend.controller1=0u;civ_set_controller_input(frontend.core,0u,0u);printf("tap-snes ok=%d mask=%04X frames=%llu reached=%llu frontier=%s reason=%s\n",ok,(unsigned)((uint16_t)mask),frames,(unsigned long long)civ_frame_count(frontend.core),civ_frontier_address(frontend.core),civ_frontier_reason(frontend.core));if(!ok)exit_code=1;}}
        else if(!strcmp(command,"trace-frame")){char path[4096];int ok;if(sscanf(arguments,"%llu %4095s",&value,path)!=2)puts("error usage trace-frame FRAME PATH.csv");else{ok=trace_to_frame(&frontend,(uint64_t)value,path);printf("trace-frame ok=%d requested=%llu reached=%llu path=%s frontier=%s reason=%s\n",ok,value,(unsigned long long)civ_frame_count(frontend.core),path,civ_frontier_address(frontend.core),civ_frontier_reason(frontend.core));if(!ok)exit_code=1;}}
        else if(!strcmp(command,"trace-pc")){char path[4096];unsigned pbr,pc;int ok;if(sscanf(arguments,"%llu %x %x %4095s",&value,&pbr,&pc,path)!=4||pbr>0xFFu||pc>0xFFFFu)puts("error usage trace-pc FRAME PBR PC PATH.csv");else{ok=trace_pc_to_frame(&frontend,(uint64_t)value,(uint8_t)pbr,(uint16_t)pc,path);printf("trace-pc ok=%d requested=%llu reached=%llu pc=%02X:%04X path=%s frontier=%s reason=%s\n",ok,value,(unsigned long long)civ_frame_count(frontend.core),pbr,pc,path,civ_frontier_address(frontend.core),civ_frontier_reason(frontend.core));if(!ok)exit_code=1;}}
        else if(!strcmp(command,"input")){unsigned mask;if(sscanf(arguments,"%i",(int *)&mask)!=1)puts("error usage input SERIAL_MASK");else{civ_frontend_set_controller1(&frontend,(uint16_t)mask);printf("ok input serial=%04X\n",frontend.controller1);}}
        else if(!strcmp(command,"input-snes")){unsigned mask;if(sscanf(arguments,"%i",(int *)&mask)!=1)puts("error usage input-snes SNES_MASK");else{civ_frontend_set_controller1(&frontend,snes_mask_to_serial((uint16_t)mask));printf("ok input-snes serial=%04X\n",frontend.controller1);}}
        else if(!strcmp(command,"input-status"))print_input_status(&frontend);
        else if(!strcmp(command,"mouse")){unsigned port,buttons;int dx,dy;if(sscanf(arguments,"%u %d %d %i",&port,&dx,&dy,(int *)&buttons)!=4)puts("error usage mouse PORT DX DY BUTTONS");else{civ_set_mouse_input(frontend.core,port,(int16_t)dx,(int16_t)dy,(uint8_t)buttons);puts("ok mouse");}}
        else if(!strcmp(command,"state-dir")){if(!civ_frontend_set_state_directory(&frontend,arguments,error,sizeof(error)))printf("state-dir ok=0 error=%s\n",civ_frontend_last_error(&frontend));else if(!civ_frontend_load_persistent_sram(&frontend))printf("state-dir ok=0 error=%s\n",civ_frontend_last_error(&frontend));else printf("state-dir ok=1 path=%s\n",civ_frontend_state_directory(&frontend));}
        else if(!strcmp(command,"sram-status")){char path[4096];(void)civ_frontend_sram_path(&frontend,path,sizeof(path));printf("sram-status dirty=%d path=%s\n",civ_sram_dirty(frontend.core),path);}
        else if(!strcmp(command,"sram-save")){int ok=civ_frontend_flush_persistent_sram(&frontend,1);char path[4096];(void)civ_frontend_sram_path(&frontend,path,sizeof(path));printf("sram-save ok=%d path=%s error=%s\n",ok,path,civ_frontend_last_error(&frontend));if(!ok)exit_code=1;}
        else if(!strcmp(command,"sram-load")){int ok=civ_frontend_load_persistent_sram(&frontend);char path[4096];(void)civ_frontend_sram_path(&frontend,path,sizeof(path));printf("sram-load ok=%d path=%s error=%s\n",ok,path,civ_frontend_last_error(&frontend));if(!ok)exit_code=1;}
        else if(!strcmp(command,"snapshot-save")&&sscanf(arguments,"%llu",&value)==1){int ok=civ_frontend_snapshot_save(&frontend,(unsigned)value);char path[4096];(void)civ_frontend_snapshot_path(&frontend,(unsigned)value,path,sizeof(path));printf("snapshot-save %llu ok=%d path=%s error=%s\n",value,ok,path,civ_frontend_last_error(&frontend));if(!ok)exit_code=1;}
        else if(!strcmp(command,"snapshot-load")&&sscanf(arguments,"%llu",&value)==1){int ok=civ_frontend_snapshot_load(&frontend,(unsigned)value);char path[4096];(void)civ_frontend_snapshot_path(&frontend,(unsigned)value,path,sizeof(path));printf("snapshot-load %llu ok=%d path=%s error=%s\n",value,ok,path,civ_frontend_last_error(&frontend));if(!ok)exit_code=1;}
        else if(!strcmp(command,"snapshot-path")&&sscanf(arguments,"%llu",&value)==1){char path[4096];int ok=civ_frontend_snapshot_path(&frontend,(unsigned)value,path,sizeof(path));printf("snapshot-path %llu ok=%d exists=%d path=%s\n",value,ok,civ_frontend_snapshot_exists(&frontend,(unsigned)value),ok?path:"");}
        else if(!strcmp(command,"video-state")){CivVideoCheckpoint v;civ_capture_video_checkpoint(frontend.core,&v);printf("video-state forced-blank=%u brightness=%u mode=%u main=%02X sub=%02X frame=%llu master=%llu vram=%016llX cgram=%016llX oam=%016llX dma=%llu/%llu writes=%llu/%llu/%llu\n",v.forced_blank,v.brightness,v.bg_mode,v.main_screen_layers,v.sub_screen_layers,(unsigned long long)v.frame_count,(unsigned long long)v.master_clock,(unsigned long long)v.vram_fnv1a64,(unsigned long long)v.cgram_fnv1a64,(unsigned long long)v.oam_fnv1a64,(unsigned long long)v.dma_run_count,(unsigned long long)v.dma_transfer_byte_count,(unsigned long long)v.vram_data_write_count,(unsigned long long)v.cgram_data_write_count,(unsigned long long)v.oam_data_write_count);}
        else if(!strcmp(command,"ppu-state"))print_ppu_state(&frontend);
        else if(!strcmp(command,"frame-hash"))print_frame_hash(&frontend);
        else if(!strcmp(command,"screenshot")||!strcmp(command,"screenshot-current"))take_screenshot(&frontend,arguments);
        else if(!strcmp(command,"audio-status"))print_audio_status(&frontend);
        else if(!strcmp(command,"record-audio"))printf("record-audio ok=%d path=%s\n",civ_v20_write_wav(frontend.core,arguments),arguments);
        else if(!strcmp(command,"peek-wram")){unsigned offset,count;if(sscanf(arguments,"%i %u",(int *)&offset,&count)!=2||offset>=CIV_WRAM_SIZE||count>CIV_WRAM_SIZE-offset)puts("error usage peek-wram OFFSET COUNT");else print_wram(&frontend,offset,count);}
        else if(!strcmp(command,"quit"))break;
        else printf("error unknown-or-invalid-command %s\n",command);
        fflush(stdout);
    }
    if(!civ_frontend_flush_persistent_sram(&frontend,0)){fprintf(stderr,"SRAM flush failed: %s\n",civ_frontend_last_error(&frontend));exit_code=1;}
    if(input!=stdin)fclose(input);
    civ_frontend_shutdown(&frontend);
    free(rom);
    return exit_code;
}
