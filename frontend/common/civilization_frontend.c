#include "civilization_frontend.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define CIV_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define CIV_MKDIR(path) mkdir((path), 0777)
#endif

#define CIV_SRAM_FLUSH_INTERVAL_FRAMES 120u

static int drain_core_pcm(CivRecomp *core)
{
    int16_t pcm[512u*2u];uint8_t known[512u];size_t available;
    if(!core)return 0;
    while((available=civ_audio_available(core))!=0u){
        size_t request=available<512u?available:512u;
        if(civ_audio_read(core,pcm,known,request)!=request)return 0;
    }
    return civ_audio_overflow_count(core)==0u;
}

static void frontend_error(CivFrontend *f,char *error,size_t error_cap,const char *text)
{
    if(!text)text="";
    if(f&&text!=f->last_error)(void)snprintf(f->last_error,sizeof(f->last_error),"%s",text);
    if(error&&error_cap)(void)snprintf(error,error_cap,"%s",text);
}

static int path_join(const char *left,const char *right,char *out,size_t capacity)
{
    size_t n;int written;
    if(!left||!right||!out||capacity==0u)return 0;
    n=strlen(left);
    if(n==0u)written=snprintf(out,capacity,"%s",right);
    else if(left[n-1u]=='/'||left[n-1u]=='\\')written=snprintf(out,capacity,"%s%s",left,right);
    else written=snprintf(out,capacity,"%s/%s",left,right);
    return written>=0&&(size_t)written<capacity;
}

static int ensure_directory(const char *path)
{
    char copy[CIV_FRONTEND_PATH_CAPACITY];size_t i,n;
    if(!path||!*path)return 0;
    if(strlen(path)>=sizeof(copy))return 0;
    (void)snprintf(copy,sizeof(copy),"%s",path);
    n=strlen(copy);
    for(i=1u;i<n;++i){
        if(copy[i]=='/'||copy[i]=='\\'){
#ifdef _WIN32
            /* Do not pass a drive designator such as "C:" to _mkdir(). */
            if(i==2u&&copy[1]==':')continue;
            /* The leading pair in a UNC path is a root marker, not a directory. */
            if(i==1u&&(copy[0]=='/'||copy[0]=='\\'))continue;
#endif
            char saved=copy[i];copy[i]='\0';
            if(copy[0]&&CIV_MKDIR(copy)!=0&&errno!=EEXIST){copy[i]=saved;return 0;}
            copy[i]=saved;
        }
    }
    return CIV_MKDIR(copy)==0||errno==EEXIST;
}

void civ_frontend_init_empty(CivFrontend *f)
{
    if(f) {
        memset(f, 0, sizeof(*f));
        (void)snprintf(f->state_directory,sizeof(f->state_directory),".");
        f->core=civ_create(f->last_error,sizeof(f->last_error));
    }
}

void civ_frontend_shutdown(CivFrontend *f)
{
    if(!f)return;
    civ_destroy(f->core);
    f->core=NULL;
    f->loaded=0;
}

int civ_frontend_load_rom(CivFrontend *f, const uint8_t *rom, size_t size, char *error, size_t error_cap)
{
    if(!f || !f->core || !rom) return 0;
    civ_reset(f->core);
    if(!civ_attach_verified_rom(f->core, rom, size, error, error_cap)) return 0;
    f->rom = rom;
    f->rom_size = size;
    f->loaded = 1;
    f->paused = 1;
    frontend_error(f,error,error_cap,"");
    return 1;
}

int civ_frontend_set_state_directory(CivFrontend *f,const char *directory,char *error,size_t error_cap)
{
    if(!f||!directory||!*directory||strlen(directory)>=sizeof(f->state_directory)){
        frontend_error(f,error,error_cap,"A valid persistence directory is required.");return 0;
    }
    if(!ensure_directory(directory)){
        frontend_error(f,error,error_cap,"Unable to create the persistence directory.");return 0;
    }
    (void)snprintf(f->state_directory,sizeof(f->state_directory),"%s",directory);
    frontend_error(f,error,error_cap,"");
    return 1;
}

const char *civ_frontend_state_directory(const CivFrontend *f){return f?f->state_directory:"";}
const char *civ_frontend_last_error(const CivFrontend *f){return f?f->last_error:"";}

int civ_frontend_sram_path(const CivFrontend *f,char *path,size_t capacity)
{
    return f&&path_join(f->state_directory,"Civilization.srm",path,capacity);
}

int civ_frontend_snapshot_path(const CivFrontend *f,unsigned slot,char *path,size_t capacity)
{
    char dir[CIV_FRONTEND_PATH_CAPACITY],name[64];
    if(!f||slot<1u||slot>CIV_FRONTEND_SNAPSHOT_SLOTS)return 0;
    if(!path_join(f->state_directory,"Snapshots",dir,sizeof(dir)))return 0;
    (void)snprintf(name,sizeof(name),"Snapshot%u.civsnap",slot);
    return path_join(dir,name,path,capacity);
}

int civ_frontend_load_persistent_sram(CivFrontend *f)
{
    char path[CIV_FRONTEND_PATH_CAPACITY];FILE *file;uint8_t image[CIV_SRAM_SIZE];int trailing;
    if(!f||!f->loaded||!civ_frontend_sram_path(f,path,sizeof(path)))return 0;
    file=fopen(path,"rb");
    if(!file){
        if(errno==ENOENT){frontend_error(f,NULL,0u,"");return 1;}
        frontend_error(f,NULL,0u,"Unable to open the battery SRAM file.");return 0;
    }
    if(fread(image,1u,sizeof(image),file)!=sizeof(image)||(trailing=fgetc(file))!=EOF||ferror(file)){
        fclose(file);frontend_error(f,NULL,0u,"Battery SRAM must be an exact 32 KiB image.");return 0;
    }
    if(fclose(file)!=0||!civ_sram_load(f->core,image,sizeof(image),f->last_error,sizeof(f->last_error)))return 0;
    f->last_sram_flush_frame=civ_frame_count(f->core);
    return 1;
}

int civ_frontend_flush_persistent_sram(CivFrontend *f,int force)
{
    char path[CIV_FRONTEND_PATH_CAPACITY],temp[CIV_FRONTEND_PATH_CAPACITY];
    FILE *file;uint8_t image[CIV_SRAM_SIZE];
    if(!f||!f->loaded)return 0;
    if(!force&&!civ_sram_dirty(f->core))return 1;
    if(!civ_frontend_sram_path(f,path,sizeof(path))||
       snprintf(temp,sizeof(temp),"%s.tmp",path)<=0||strlen(temp)>=sizeof(temp)||
       !civ_sram_copy(f->core,image,sizeof(image))){frontend_error(f,NULL,0u,"Unable to prepare battery SRAM image.");return 0;}
    if(!ensure_directory(f->state_directory)){frontend_error(f,NULL,0u,"Unable to create the persistence directory.");return 0;}
    file=fopen(temp,"wb");
    if(!file){frontend_error(f,NULL,0u,"Unable to create battery SRAM image.");return 0;}
    if(fwrite(image,1u,sizeof(image),file)!=sizeof(image)){
        (void)fclose(file);(void)remove(temp);frontend_error(f,NULL,0u,"Unable to write battery SRAM image.");return 0;
    }
    if(fclose(file)!=0){(void)remove(temp);frontend_error(f,NULL,0u,"Unable to finalize temporary battery SRAM image.");return 0;}
    (void)remove(path);
    if(rename(temp,path)!=0){(void)remove(temp);frontend_error(f,NULL,0u,"Unable to finalize battery SRAM image.");return 0;}
    civ_sram_mark_clean(f->core);f->last_sram_flush_frame=civ_frame_count(f->core);frontend_error(f,NULL,0u,"");return 1;
}

static int maybe_flush_sram(CivFrontend *f)
{
    if(!f||!f->loaded||!civ_sram_dirty(f->core))return 1;
    if(civ_frame_count(f->core)-f->last_sram_flush_frame<CIV_SRAM_FLUSH_INTERVAL_FRAMES)return 1;
    return civ_frontend_flush_persistent_sram(f,0);
}

int civ_frontend_reset(CivFrontend *f, char *error, size_t error_cap)
{
    if(!f || !f->loaded) return 0;
    if(!civ_frontend_flush_persistent_sram(f,0)){
        frontend_error(f,error,error_cap,f->last_error);return 0;
    }
    civ_reset(f->core);
    if(!civ_attach_verified_rom(f->core, f->rom, f->rom_size, error, error_cap)) return 0;
    f->paused = 1;
    f->controller1 = 0;
    frontend_error(f,error,error_cap,"");
    return 1;
}

void civ_frontend_pause(CivFrontend *f){if(f)f->paused=1;}
void civ_frontend_play(CivFrontend *f){if(f&&f->loaded)f->paused=0;}

void civ_frontend_set_controller1(CivFrontend *f, uint16_t mask)
{
    if(f){f->controller1=mask;if(f->loaded)civ_set_controller_input(f->core,0u,mask);}
}

int civ_frontend_run(CivFrontend *f, uint64_t instructions)
{
    int ok;
    if(!f||!f->loaded||f->paused)return 0;
    civ_set_controller_input(f->core,0u,f->controller1);
    ok=civ_run_static(f->core,instructions);
    if(ok&&!drain_core_pcm(f->core))return 0;
    if(!maybe_flush_sram(f))return 0;
    return ok;
}

int civ_frontend_run_to_frame(CivFrontend *f, uint64_t target_frame, uint64_t instruction_budget)
{
    CivFrameResult frame;
    if(!f||!f->loaded||f->paused)return 0;
    while(civ_frame_count(f->core)<target_frame&&!civ_has_failed(f->core))
        if(!civ_run_frame(f->core,f->controller1,instruction_budget,0,&frame)||!drain_core_pcm(f->core))return 0;
    if(!maybe_flush_sram(f))return 0;
    return !civ_has_failed(f->core)&&civ_frame_count(f->core)>=target_frame;
}

int civ_frontend_snapshot_save(CivFrontend *f,unsigned slot)
{
    char path[CIV_FRONTEND_PATH_CAPACITY],dir[CIV_FRONTEND_PATH_CAPACITY];
    if(!f||!f->loaded||slot<1u||slot>CIV_FRONTEND_SNAPSHOT_SLOTS)return 0;
    if(civ_audio_active(f->core)&&!civ_v20_audio_sync(f->core)){
        frontend_error(f,NULL,0u,"Unable to synchronize Full Static audio before snapshot.");return 0;
    }
    if(!path_join(f->state_directory,"Snapshots",dir,sizeof(dir))||!ensure_directory(dir)||
       !civ_frontend_snapshot_path(f,slot,path,sizeof(path))||
       !civ_snapshot_save(f->core,path,f->last_error,sizeof(f->last_error)))return 0;
    return 1;
}

int civ_frontend_snapshot_load(CivFrontend *f,unsigned slot)
{
    char path[CIV_FRONTEND_PATH_CAPACITY];
    int widescreen;
    if(!f||!f->loaded||slot<1u||slot>CIV_FRONTEND_SNAPSHOT_SLOTS)return 0;
    widescreen=civ_widescreen_enabled(f->core);
    if(
       !civ_frontend_snapshot_path(f,slot,path,sizeof(path))||
       !civ_snapshot_load(f->core,f->rom,f->rom_size,path,f->last_error,sizeof(f->last_error)))return 0;
    civ_set_widescreen_enabled(f->core,widescreen);
    f->paused=1;f->controller1=0u;civ_set_controller_input(f->core,0u,0u);
    if(!civ_render_present_frame(f->core)){frontend_error(f,NULL,0u,"Snapshot restored, but current-frame rendering failed.");return 0;}
    return 1;
}

int civ_frontend_snapshot_exists(const CivFrontend *f,unsigned slot)
{
    char path[CIV_FRONTEND_PATH_CAPACITY];FILE *file;
    if(!civ_frontend_snapshot_path(f,slot,path,sizeof(path)))return 0;
    file=fopen(path,"rb");if(!file)return 0;fclose(file);return 1;
}

int civ_frontend_frame_counter_ready(const CivFrontend *f){return f&&f->loaded;}
int civ_frontend_frame_presenter_ready(const CivFrontend *f){return f&&f->loaded&&civ_get_framebuffer_rgba(f->core)!=NULL;}
int civ_frontend_audio_pcm_ready(const CivFrontend *f)
{
    return f&&f->loaded&&civ_audio_pcm_frames(f->core)!=0u;
}
