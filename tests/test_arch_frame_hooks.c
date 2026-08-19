#include "civilization_static_recomp.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct HookCounts {unsigned frames,failures,diagnostics;} HookCounts;
static void frame_hook(void *p,uint64_t frame,const uint32_t *pixels)
{HookCounts *c=(HookCounts *)p;(void)frame;(void)pixels;c->frames++;}
static void failure_hook(void *p,const char *address,const char *reason)
{HookCounts *c=(HookCounts *)p;(void)address;(void)reason;c->failures++;}
static void diagnostic_hook(void *p,const char *event)
{HookCounts *c=(HookCounts *)p;(void)event;c->diagnostics++;}

int main(int argc,char **argv)
{
    FILE *f;uint8_t *rom;long size;CivRecomp *core;CivFrameResult result;
    HookCounts counts={0u,0u,0u};CivHostHooks hooks={0};char error[256];
    if(argc!=2)return 2;
    f=fopen(argv[1],"rb");if(!f)return 3;
    if(fseek(f,0,SEEK_END)!=0||(size=ftell(f))<=0||fseek(f,0,SEEK_SET)!=0){fclose(f);return 4;}
    rom=(uint8_t *)malloc((size_t)size);if(!rom){fclose(f);return 5;}
    if(fread(rom,1u,(size_t)size,f)!=(size_t)size){fclose(f);free(rom);return 6;}fclose(f);
    core=civ_create(error,sizeof(error));if(!core){free(rom);return 7;}
    if(!civ_attach_verified_rom(core,rom,(size_t)size,error,sizeof(error))){civ_destroy(core);free(rom);return 8;}
    hooks.context=&counts;hooks.frame_complete=frame_hook;hooks.failure=failure_hook;hooks.diagnostic=diagnostic_hook;
    civ_set_host_hooks(core,&hooks);
    if(!civ_run_frame(core,0u,UINT64_C(20000000),0,&result)){civ_destroy(core);free(rom);return 9;}
    if(!result.frame_completed||result.end_frame!=result.start_frame+1u||counts.frames!=1u||counts.diagnostics!=1u||counts.failures!=0u){civ_destroy(core);free(rom);return 10;}
    civ_destroy(core);free(rom);return 0;
}
